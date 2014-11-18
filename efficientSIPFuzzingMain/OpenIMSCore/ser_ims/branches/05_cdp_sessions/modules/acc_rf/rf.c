/**
 * $Id$
 *   
 * Copyright (C) 2004-2007 FhG Fokus
 *
 * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
 * implementation
 *
 * Open IMS Core is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the Open IMS Core software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact Fraunhofer FOKUS by e-mail at the following
 * addresses:
 *     info@open-ims.org
 *
 * Open IMS Core is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * It has to be noted that this Open Source IMS Core System is not 
 * intended to become or act as a product in a commercial context! Its 
 * sole purpose is to provide an IMS core reference implementation for 
 * IMS technology testing and IMS application prototyping for research 
 * purposes, typically performed in IMS test-beds.
 * 
 * Users of the Open Source IMS Core System have to be aware that IMS
 * technology may be subject of patents and licence terms, as being 
 * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
 * standards. Thus all Open IMS Core users have to take notice of this 
 * fact and have to agree to check out carefully before installing, 
 * using and extending the Open Source IMS Core System, if related 
 * patents and licences may become applicable to the intended usage 
 * context.  
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
 
#include "mod.h"
#include "rf.h"

#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"
#include "../cdp/acct.h"
#include "../cdp/acctstatemachine.h"
#include "rf_avp.h"
#include "sip.h"

#include "offline_charging.h"

/**< Structure with pointers to tm funcs */
extern struct tm_binds tmb;

/**< Structure with pointers to cdp funcs */
extern struct cdp_binds cdpb;

/**< Offline Charging flag */
extern struct offline_charging_flag cflag;          

/**< FQDN of CDF for offline charging, defined in acc_rf.c */
extern str cdf_peer_str;	

/* default configuration, used to generate AVPs. */
str service_context_id = {"32260@3gpp.org", 14};
unsigned int role_of_node = AVP_IMS_ORIGINATING_ROLE;
unsigned int node_functionality = AVP_IMS_P_CSCF;

/**
 * Create and send ACR for an Event record from SIP message.
 */
//AAAMessage* Rf_ACR_event(struct sip_msg *msg) 
int Rf_ACR_event(struct sip_msg *msg)
{
	AAAMessage *acr = 0;
	AAAMessage *aca = 0;
	
	//sessId = cdpb.AAACreateSession();
	
	//acr = cdpb.AAACreateRequest(IMS_Rf,Code_AC,Flag_Proxyable,&sessId);
	acr = cdpb.AAACreateRequest(IMS_Rf,Code_AC,Flag_Proxyable,0);
	
	if (!acr) goto error;
	
	/*---------- 1. Add mandatory AVPs ----------*/
	
	/* Session-Id, Origin-Host, Origin-Realm AVP are added by the stack. */
	
	/* Add Destination-Realm AVP */
	str realm = cscf_get_realm_from_ruri(msg);
	if (!Rf_add_destination_realm(acr, realm)) goto error;
	
	/* Add Accounting-Record-Type AVP */
	if (!Rf_add_accounting_record_type(acr, AAA_ACCT_EVENT)) goto error;
	
	/* Add Accounting-Record-Number AVP */
	if (!Rf_add_accounting_record_number(acr, 0)) goto error;
	
	/* Add Acct-Application-Id AVP */
	if (!Rf_add_acct_application_id(acr, IMS_Rf)) goto error;
	
	/* add Service-Context-Id, see TS32.299 V740 7.1.7 */
	if (!Rf_add_service_context_id(acr, service_context_id)) goto error; 

	
	
	/*---------- 2. Create and add Service-Information AVP ----------*/
	
	/*
	 *  See 3GPP TS32.299 V7.4.0:
	 * 
	 *  <Service-Information> = [IMS-Information]
	 * 								 [Event-Type]
	 * 								 [Role-Of-Node]
	 * 								 [Node-Functionality]
	 * 								 [User-Session-Id]
	 * 								*[Calling-Party-Address]
	 * 								 [Called-Party-Address]
	 * 								*[Associated-URI]	
	 * 								 [Time-Stamps]
	 * 								*[Application-Server-Information]	
	 * 								*[Inter-Operator-Identifier]	
	 * 								 [IMS-Charging-Identifier]	
	 * 								*[SDP-Session-Description]
	 * 								*[SDP-Media-Component]
	 * 								 [Served-Party-IP-Address]
	 * 								 [Server-Capabilities]
	 * 								 [Trunk-Group-Id]
	 * 								 [Bearer-Service]
	 * 								 [Service-Id]
	 * 								*[Service-Specific-Info]
	 * 								*[Message-Body]
	 * 								 [Cause-Code]
	 * 								 [Access-Network-Information]
	 *
	 */
	 			 	
	AAA_AVP_LIST ls_ims; /* AVP list for member AVPs in IMS-Information AVP */
	ls_ims.head=0; ls_ims.tail = 0;

	AAA_AVP_LIST ls_ser; /* AVP list for member AVPs in Service-Information AVP */
	ls_ser.head=0; ls_ser.tail = 0;
	
	
	AAA_AVP_LIST ls_tmp;
	ls_tmp.head=0; ls_tmp.tail = 0;
	
	/* Following AVPs are created by CSCF directly. */
	if(!Rf_add_role_of_node(&ls_ims, role_of_node)) goto error;
	if(!Rf_add_node_functionality(&ls_ims, node_functionality)) goto error;
	
	
	/* To decide which AVP should be created based on the charging flag. */
	
	/* SIP Request Method --> Event Type:
	 * 
	 * <Event-Type> = [SIP-Method]
	 *                [Event]
	 *                [Expires]
	 * 
	 */
	if ((cflag.cf_3gpp & CF_3GPP_EVENT_TYPE) &&
		(msg->first_line.type == SIP_REQUEST)) {
		
		str method = cscf_get_sip_method(msg);
		if (!Rf_add_sip_method(&ls_tmp, method)) goto error;
			
		str event = cscf_get_event(msg);
		if (event.len) 
			if (!Rf_add_event(&ls_tmp, event)) goto error;
		
		
		int expires = cscf_get_expires_hdr(msg);	
		if (expires >= 0) 
			if (!Rf_add_expires(&ls_tmp, expires)) goto error;
		
			
		if (!Rf_add_event_type(&ls_ims, &ls_tmp)) goto error;
	}
	
	/*  Call-ID --> User-Session-ID AVP */
	if (cflag.cf_3gpp & CF_3GPP_USER_SESSION_ID) {
		str user_session_id = cscf_get_call_id(msg, 0);
		if (user_session_id.len)
			if (!Rf_add_user_session_id(&ls_ims, user_session_id)) goto error;
	}
		
	/* P-Asserted-Identity --> Calling-Party-Address AVP */
	/*
	 * Obtained from P-Asserted-Identity of non-REGISTER SIP request.
	 * 
	 * TODO may appear serveral times when the P-Asserted-Identity header
	 * contains both a SIP URI and a TEL URI.
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLING_PARTY_ADDRESS) && 
		(msg->first_line.type == SIP_REQUEST) && 
		(msg->REQ_METHOD != METHOD_REGISTER)) {
		str calling_addr = cscf_get_asserted_identity(msg);
		if (calling_addr.len)
			if (!Rf_add_calling_party_address(&ls_ims, calling_addr)) goto error; 
			
	}
	
	/* Request URI / To URI --> Called-Party-Address AVP */
	/* if REG from To URI
	 * otherwise from Request URI
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLED_PARTY_ADDRESS) &&
		(msg->first_line.type == SIP_REQUEST)) {
		
		str called_addr;
		
		if (msg->REQ_METHOD & METHOD_REGISTER) {
			called_addr = cscf_get_public_identity(msg);
		} else {
			called_addr = cscf_get_public_identity_from_requri(msg);
		}
		
		if (called_addr.len)
			if (!Rf_add_called_party_address(&ls_ims, called_addr)) goto error;
	}
	
	/* P-Charging-Vector: icid-value -> IMS-Charging-Identifier AVP */
	if (cflag.cf_3gpp & CF_3GPP_IMS_CHARGING_IDENTIFIER) {
		str icid = cscf_get_p_charging_vector_icid(msg);
		// TODO: should we send ACR without icid?
		if (icid.len==0 || icid.s==0 || !Rf_add_ims_charging_id(&ls_ims, icid)) goto error;
	}
		
	
	
	if (!Rf_add_ims_information(&ls_ser, &ls_ims)) goto error;
	if (!Rf_add_service_information(acr, &ls_ser)) goto error;
	
	/*---------- 3. Send ACR to CDF ----------*/
	//aca = cdpb.AAASendRecvMessage(acr, &cdf_peer_str);
	
	str dlgid = cscf_get_call_id(msg, 0);
	//str dlgid_test = {"testevtid", 10};
	aca = cdpb.AAAAcctCliEvent(acr, &dlgid, &cdf_peer_str);
	
	
	/*---------- 4. Destroy the session ----------*/
	//cdpb.AAADropSession(&sessId);
	// THIS IS DONE BY CDP acc
	
	
	//if (aca) cdpb.AAAFreeMessage(&acr);  // acr freed by cdp worker
	if (aca) cdpb.AAAFreeMessage(&aca);
	
	return 0;
	//return aca;
	
error:
	//free stuff
	//if (session) cdpb.AAADropAcctSession(session);
	//if (sessId.s) cdpb.AAADropSession(&sessId);
	if (acr) cdpb.AAAFreeMessage(&acr);
	return 1;
}

//AAAMessage* Rf_ACR_start(struct sip_msg *msg) {
//	return 0;
//}

int Rf_ACR_start(struct sip_msg *msg) {
	AAAAcctSession* session = 0;
	AAAMessage *acr = 0;
	AAAMessage *aca = 0;
	
	acr = cdpb.AAACreateRequest(IMS_Rf,Code_AC,Flag_Proxyable,0);
	
	if (!acr) goto error;
	
	/*---------- 1. Add mandatory AVPs ----------*/
	
	/* Session-Id, Origin-Host, Origin-Realm AVP are added by the stack. */
	
	/* Add Destination-Realm AVP */
	str realm = cscf_get_realm_from_ruri(msg);
	if (!Rf_add_destination_realm(acr, realm)) goto error;
	
	/* Add Accounting-Record-Type AVP */
	if (!Rf_add_accounting_record_type(acr, AAA_ACCT_START)) goto error;
	
	/* Add Accounting-Record-Number AVP */
	if (!Rf_add_accounting_record_number(acr, 0)) goto error;
	
	/* Add Acct-Application-Id AVP */
	if (!Rf_add_acct_application_id(acr, IMS_Rf)) goto error;
	
	/* add Service-Context-Id, see TS32.299 V740 7.1.7 */
	if (!Rf_add_service_context_id(acr, service_context_id)) goto error; 

	
	
	/*---------- 2. Create and add Service-Information AVP ----------*/
	
	/*
	 *  See 3GPP TS32.299 V7.4.0:
	 * 
	 *  <Service-Information> = [IMS-Information]
	 * 								 [Event-Type]
	 * 								 [Role-Of-Node]
	 * 								 [Node-Functionality]
	 * 								 [User-Session-Id]
	 * 								*[Calling-Party-Address]
	 * 								 [Called-Party-Address]
	 * 								*[Associated-URI]	
	 * 								 [Time-Stamps]
	 * 								*[Application-Server-Information]	
	 * 								*[Inter-Operator-Identifier]	
	 * 								 [IMS-Charging-Identifier]	
	 * 								*[SDP-Session-Description]
	 * 								*[SDP-Media-Component]
	 * 								 [Served-Party-IP-Address]
	 * 								 [Server-Capabilities]
	 * 								 [Trunk-Group-Id]
	 * 								 [Bearer-Service]
	 * 								 [Service-Id]
	 * 								*[Service-Specific-Info]
	 * 								*[Message-Body]
	 * 								 [Cause-Code]
	 * 								 [Access-Network-Information]
	 *
	 */
	 			 	
	AAA_AVP_LIST ls_ims; /* AVP list for member AVPs in IMS-Information AVP */
	ls_ims.head=0; ls_ims.tail = 0;

	AAA_AVP_LIST ls_ser; /* AVP list for member AVPs in Service-Information AVP */
	ls_ser.head=0; ls_ser.tail = 0;
	
	
	AAA_AVP_LIST ls_tmp;
	ls_tmp.head=0; ls_tmp.tail = 0;
	
	/* Following AVPs are created by CSCF directly. */
	if(!Rf_add_role_of_node(&ls_ims, role_of_node)) goto error;
	if(!Rf_add_node_functionality(&ls_ims, node_functionality)) goto error;
	
	
	/* To decide which AVP should be created based on the charging flag. */
	
	/*  Call-ID --> User-Session-ID AVP */
	if (cflag.cf_3gpp & CF_3GPP_USER_SESSION_ID) {
		str user_session_id = cscf_get_call_id(msg, 0);
		if (user_session_id.len)
			if (!Rf_add_user_session_id(&ls_ims, user_session_id)) goto error;
	}
	
	/* P-Asserted-Identity --> Calling-Party-Address AVP */
	/*
	 * Obtained from P-Asserted-Identity of non-REGISTER SIP request.
	 * 
	 * TODO may appear serveral times when the P-Asserted-Identity header
	 * contains both a SIP URI and a TEL URI.
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLING_PARTY_ADDRESS) && 
		(msg->first_line.type == SIP_REQUEST) && 
		(msg->REQ_METHOD != METHOD_REGISTER)) {
		str calling_addr = cscf_get_asserted_identity(msg);
		if (calling_addr.len)
			if (!Rf_add_calling_party_address(&ls_ims, calling_addr)) goto error; 
			
	}
	
	/* Request URI / To URI --> Called-Party-Address AVP */
	/* if REG from To URI
	 * otherwise from Request URI
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLED_PARTY_ADDRESS) &&
		(msg->first_line.type == SIP_REQUEST)) {
		
		str called_addr;
		
		if (msg->REQ_METHOD & METHOD_REGISTER) {
			called_addr = cscf_get_public_identity(msg);
		} else {
			called_addr = cscf_get_public_identity_from_requri(msg);
		}
		
		if (called_addr.len)
			if (!Rf_add_called_party_address(&ls_ims, called_addr)) goto error;
	}
	
	/* P-Charging-Vector: icid-value -> IMS-Charging-Identifier AVP */
	if (cflag.cf_3gpp & CF_3GPP_IMS_CHARGING_IDENTIFIER) {
		str icid = cscf_get_p_charging_vector_icid(msg);
		// TODO: should we send ACR without icid?
		if (icid.len==0 || icid.s==0 || !Rf_add_ims_charging_id(&ls_ims, icid)) goto error;
	}
	
	if (!Rf_add_ims_information(&ls_ser, &ls_ims)) goto error;
	if (!Rf_add_service_information(acr, &ls_ser)) goto error;
	
	
	str dlgid = cscf_get_call_id(msg, 0);
	aca = cdpb.AAAAcctCliStart(acr, &dlgid, &cdf_peer_str, session); // we can get the created session here, in case we need it // TODO: do we?
	
	return 0;
error:
	//free stuff
	//if (session) cdpb.AAADropAcctSession(session);
	if (acr) cdpb.AAAFreeMessage(&acr);
	return 1;
}

 
//AAAMessage* Rf_ACR_interim(struct sip_msg *msg) {
	//s = cdpb.AAAGetAcctSession(&dlgid);
	//return 0;
//}

int Rf_ACR_interim(struct sip_msg *msg, AAAAcctSession *s) {
	AAAMessage *acr = 0;
	AAAMessage *aca = 0;
	
	acr = cdpb.AAACreateRequest(IMS_Rf,Code_AC,Flag_Proxyable,0);
	
	if (!acr) goto error;
	
	/*---------- 1. Add mandatory AVPs ----------*/
	
	/* Session-Id, Origin-Host, Origin-Realm AVP are added by the stack. */
	
	/* Add Destination-Realm AVP */
	str realm = cscf_get_realm_from_ruri(msg);
	if (!Rf_add_destination_realm(acr, realm)) goto error;
	
	/* Add Accounting-Record-Type AVP */
	if (!Rf_add_accounting_record_type(acr, AAA_ACCT_START)) goto error;
	
	/* Add Accounting-Record-Number AVP */
	if (!Rf_add_accounting_record_number(acr, 0)) goto error;
	
	/* Add Acct-Application-Id AVP */
	if (!Rf_add_acct_application_id(acr, IMS_Rf)) goto error;
	
	/* add Service-Context-Id, see TS32.299 V740 7.1.7 */
	if (!Rf_add_service_context_id(acr, service_context_id)) goto error; 

	
	
	/*---------- 2. Create and add Service-Information AVP ----------*/
	
	/*
	 *  See 3GPP TS32.299 V7.4.0:
	 * 
	 *  <Service-Information> = [IMS-Information]
	 * 								 [Event-Type]
	 * 								 [Role-Of-Node]
	 * 								 [Node-Functionality]
	 * 								 [User-Session-Id]
	 * 								*[Calling-Party-Address]
	 * 								 [Called-Party-Address]
	 * 								*[Associated-URI]	
	 * 								 [Time-Stamps]
	 * 								*[Application-Server-Information]	
	 * 								*[Inter-Operator-Identifier]	
	 * 								 [IMS-Charging-Identifier]	
	 * 								*[SDP-Session-Description]
	 * 								*[SDP-Media-Component]
	 * 								 [Served-Party-IP-Address]
	 * 								 [Server-Capabilities]
	 * 								 [Trunk-Group-Id]
	 * 								 [Bearer-Service]
	 * 								 [Service-Id]
	 * 								*[Service-Specific-Info]
	 * 								*[Message-Body]
	 * 								 [Cause-Code]
	 * 								 [Access-Network-Information]
	 *
	 */
	 			 	
	AAA_AVP_LIST ls_ims; /* AVP list for member AVPs in IMS-Information AVP */
	ls_ims.head=0; ls_ims.tail = 0;

	AAA_AVP_LIST ls_ser; /* AVP list for member AVPs in Service-Information AVP */
	ls_ser.head=0; ls_ser.tail = 0;
	
	
	AAA_AVP_LIST ls_tmp;
	ls_tmp.head=0; ls_tmp.tail = 0;
	
	/* Following AVPs are created by CSCF directly. */
	if(!Rf_add_role_of_node(&ls_ims, role_of_node)) goto error;
	if(!Rf_add_node_functionality(&ls_ims, node_functionality)) goto error;
	
	
	/* To decide which AVP should be created based on the charging flag. */
	
	/*  Call-ID --> User-Session-ID AVP */
	if (cflag.cf_3gpp & CF_3GPP_USER_SESSION_ID) {
		str user_session_id = cscf_get_call_id(msg, 0);
		if (user_session_id.len)
			if (!Rf_add_user_session_id(&ls_ims, user_session_id)) goto error;
	}
	
	/* P-Asserted-Identity --> Calling-Party-Address AVP */
	/*
	 * Obtained from P-Asserted-Identity of non-REGISTER SIP request.
	 * 
	 * TODO may appear serveral times when the P-Asserted-Identity header
	 * contains both a SIP URI and a TEL URI.
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLING_PARTY_ADDRESS) && 
		(msg->first_line.type == SIP_REQUEST) && 
		(msg->REQ_METHOD != METHOD_REGISTER)) {
		str calling_addr = cscf_get_asserted_identity(msg);
		if (calling_addr.len)
			if (!Rf_add_calling_party_address(&ls_ims, calling_addr)) goto error; 
			
	}
	
	/* Request URI / To URI --> Called-Party-Address AVP */
	/* if REG from To URI
	 * otherwise from Request URI
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLED_PARTY_ADDRESS) &&
		(msg->first_line.type == SIP_REQUEST)) {
		
		str called_addr;
		
		if (msg->REQ_METHOD & METHOD_REGISTER) {
			called_addr = cscf_get_public_identity(msg);
		} else {
			called_addr = cscf_get_public_identity_from_requri(msg);
		}
		
		if (called_addr.len)
			if (!Rf_add_called_party_address(&ls_ims, called_addr)) goto error;
	}
	
	/* P-Charging-Vector: icid-value -> IMS-Charging-Identifier AVP */
	if (cflag.cf_3gpp & CF_3GPP_IMS_CHARGING_IDENTIFIER) {
		str icid = cscf_get_p_charging_vector_icid(msg);
		// TODO: should we send ACR without icid?
		if (icid.len==0 || icid.s==0 || !Rf_add_ims_charging_id(&ls_ims, icid)) goto error;
	}
	
	if (!Rf_add_ims_information(&ls_ser, &ls_ims)) goto error;
	if (!Rf_add_service_information(acr, &ls_ser)) goto error;
	
	aca = cdpb.AAAAcctCliInterim(acr, &cdf_peer_str, s);
	
	return 0;
error:
	//free stuff
	//if (session) cdpb.AAADropAcctSession(session);
	if (acr) cdpb.AAAFreeMessage(&acr);
	return 1;
}

//AAAMessage* Rf_ACR_stop(struct sip_msg *msg) {
	//return 0;
//}

int Rf_ACR_stop(struct sip_msg *msg, AAAAcctSession *s) {
	AAAMessage *acr = 0;
	AAAMessage *aca = 0;
	
	acr = cdpb.AAACreateRequest(IMS_Rf,Code_AC,Flag_Proxyable,0);
	
	if (!acr) goto error;
	
	/*---------- 1. Add mandatory AVPs ----------*/
	
	/* Session-Id, Origin-Host, Origin-Realm AVP are added by the stack. */
	
	/* Add Destination-Realm AVP */
	str realm = cscf_get_realm_from_ruri(msg);
	if (!Rf_add_destination_realm(acr, realm)) goto error;
	
	/* Add Accounting-Record-Type AVP */
	if (!Rf_add_accounting_record_type(acr, AAA_ACCT_STOP)) goto error;
	
	/* Add Accounting-Record-Number AVP */
	if (!Rf_add_accounting_record_number(acr, 0)) goto error;
	
	/* Add Acct-Application-Id AVP */
	if (!Rf_add_acct_application_id(acr, IMS_Rf)) goto error;
	
	/* add Service-Context-Id, see TS32.299 V740 7.1.7 */
	if (!Rf_add_service_context_id(acr, service_context_id)) goto error; 

	
	
	/*---------- 2. Create and add Service-Information AVP ----------*/
	
	/*
	 *  See 3GPP TS32.299 V7.4.0:
	 * 
	 *  <Service-Information> = [IMS-Information]
	 * 								 [Event-Type]
	 * 								 [Role-Of-Node]
	 * 								 [Node-Functionality]
	 * 								 [User-Session-Id]
	 * 								*[Calling-Party-Address]
	 * 								 [Called-Party-Address]
	 * 								*[Associated-URI]	
	 * 								 [Time-Stamps]
	 * 								*[Application-Server-Information]	
	 * 								*[Inter-Operator-Identifier]	
	 * 								 [IMS-Charging-Identifier]	
	 * 								*[SDP-Session-Description]
	 * 								*[SDP-Media-Component]
	 * 								 [Served-Party-IP-Address]
	 * 								 [Server-Capabilities]
	 * 								 [Trunk-Group-Id]
	 * 								 [Bearer-Service]
	 * 								 [Service-Id]
	 * 								*[Service-Specific-Info]
	 * 								*[Message-Body]
	 * 								 [Cause-Code]
	 * 								 [Access-Network-Information]
	 *
	 */
	 			 	
	AAA_AVP_LIST ls_ims; /* AVP list for member AVPs in IMS-Information AVP */
	ls_ims.head=0; ls_ims.tail = 0;

	AAA_AVP_LIST ls_ser; /* AVP list for member AVPs in Service-Information AVP */
	ls_ser.head=0; ls_ser.tail = 0;
	
	
	AAA_AVP_LIST ls_tmp;
	ls_tmp.head=0; ls_tmp.tail = 0;
	
	/* Following AVPs are created by CSCF directly. */
	if(!Rf_add_role_of_node(&ls_ims, role_of_node)) goto error;
	if(!Rf_add_node_functionality(&ls_ims, node_functionality)) goto error;
	
	
	/* To decide which AVP should be created based on the charging flag. */
	
	/*  Call-ID --> User-Session-ID AVP */
	if (cflag.cf_3gpp & CF_3GPP_USER_SESSION_ID) {
		str user_session_id = cscf_get_call_id(msg, 0);
		if (user_session_id.len)
			if (!Rf_add_user_session_id(&ls_ims, user_session_id)) goto error;
	}
	
	/* P-Asserted-Identity --> Calling-Party-Address AVP */
	/*
	 * Obtained from P-Asserted-Identity of non-REGISTER SIP request.
	 * 
	 * TODO may appear serveral times when the P-Asserted-Identity header
	 * contains both a SIP URI and a TEL URI.
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLING_PARTY_ADDRESS) && 
		(msg->first_line.type == SIP_REQUEST) && 
		(msg->REQ_METHOD != METHOD_REGISTER)) {
		str calling_addr = cscf_get_asserted_identity(msg);
		if (calling_addr.len)
			if (!Rf_add_calling_party_address(&ls_ims, calling_addr)) goto error; 
			
	}
	
	/* Request URI / To URI --> Called-Party-Address AVP */
	/* if REG from To URI
	 * otherwise from Request URI
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLED_PARTY_ADDRESS) &&
		(msg->first_line.type == SIP_REQUEST)) {
		
		str called_addr;
		
		if (msg->REQ_METHOD & METHOD_REGISTER) {
			called_addr = cscf_get_public_identity(msg);
		} else {
			called_addr = cscf_get_public_identity_from_requri(msg);
		}
		
		if (called_addr.len)
			if (!Rf_add_called_party_address(&ls_ims, called_addr)) goto error;
	}
	
	/* P-Charging-Vector: icid-value -> IMS-Charging-Identifier AVP */
	if (cflag.cf_3gpp & CF_3GPP_IMS_CHARGING_IDENTIFIER) {
		str icid = cscf_get_p_charging_vector_icid(msg);
		// TODO: should we send ACR without icid?
		if (icid.len==0 || icid.s==0 || !Rf_add_ims_charging_id(&ls_ims, icid)) goto error;
	}
	
	if (!Rf_add_ims_information(&ls_ser, &ls_ims)) goto error;
	if (!Rf_add_service_information(acr, &ls_ser)) goto error;
	
	
	aca = cdpb.AAAAcctCliStop(acr, &cdf_peer_str, s);
	
	return 0;
error:
	//free stuff
	//if (session) cdpb.AAADropAcctSession(session);
	if (acr) cdpb.AAAFreeMessage(&acr);
	return 1;
}


/**
 * Handler for incoming Diameter answers.
 * NOT USED?
 * @param response - received response
 * @param t - transaction
 */
void RfAnswerHandler(AAAMessage *response, void *param)
{
/*	int rc=-1;
	int rec_type = -1;
	str sID = {0,0};
		
	switch(response->commandCode){
		case Code_AC:
			LOG(L_INFO,"INFO:"M_NAME":RfAnswerHandler: ACA received (Command Code %d)\n",
				response->commandCode);
			
			if (!Rf_get_result_code(response,&rc)) {
				LOG(L_ERR,"ERROR:"M_NAME":RfAnswerHandler: Could not get result code\n");
				return;
			}
	
			switch(rc){
				case AAA_SUCCESS:
					sID = Rf_get_session_id(response);
					// TODO: get the session from the session id
					
					
					if (!Rf_get_accounting_record_type(response, &rec_type)) {
						LOG(L_ERR,"ERROR:"M_NAME":RfAnswerHandler: Could not get Accounting-Record-Type AVP\n");
						return;
					}
					switch (rec_type) {
						case AAA_ACCT_EVENT:
							acct_cli_sm_process(s,ACC_EV_RCV_SUC_ACA_EVENT,0 //response
													,0);
							break;
						case AAA_ACCT_START:
							acct_cli_sm_process(s,ACC_EV_RCV_SUC_ACA_START,0 //response
													,0);
							break;
						case AAA_ACCT_STOP:
							acct_cli_sm_process(s,ACC_EV_RCV_SUC_ACA_STOP,0 //response
													,0);
							break;
						case AAA_ACCT_INTERIM:
							acct_cli_sm_process(s,ACC_EV_RCV_SUC_ACA_INTERIM,0 //response
													,0);
							break;
						default:
							LOG(L_ERR,"ERR:"M_NAME":RfAnswerHandler: Received ACA with unknown Accounting-Record-Type %d\n",rec_type);
					}
					break;
								
				default:
					// TODO: acct_sm_process(s, AccEv_Rcv_Failed_...
					LOG(L_ERR,"ERR:"M_NAME":RfAnswerHandler: Received ACA with Result Code %d\n",rc);	
			}
			break;
		default:
			LOG(L_ERR,"ERR:"M_NAME":RfAnswerHandler: Unkown Command Code %d\n",
				response->commandCode);
	}
	*/
}



/**
 * Handler for incoming Diameter failures
 * This is not (??) used as all diameter failures are handled transactionaly (??)
 * @param t - transaction
 * @param reason - failure reason
 * @returns 1
 */
int RfFailureHandler(AAATransaction *t, int reason)
{
	LOG(L_INFO,"INF:"M_NAME":RfFailureHandler:  SIP transaction %u %u Reason %d\n",
		t->hash,t->label,reason);
	switch(t->command_code){
		default:
		LOG(L_ERR,"ERR:"M_NAME":RfFailureHandler: Unkown Command Code %d\n",
			t->command_code);
	}
	return 0;
}

/**
 * Handler for incoming Diameter requests.
 * @param request - the received request
 * @param param - generic pointer
 * @returns the answer to this request
 */
AAAMessage* RfRequestHandler(AAAMessage *request,void *param)
{
	if (is_req(request)){		
		LOG(L_INFO,"INFO:"M_NAME":RfRequestHandler(): We have received a request\n");
		switch(request->applicationId){
        	default:
				LOG(L_ERR,"ERR:"M_NAME":RfRequestHandler(): - Received unknown request for app %d command %d\n",
					request->applicationId,
					request->commandCode);
				break;				
		}					
	}
	return 0;		
}


