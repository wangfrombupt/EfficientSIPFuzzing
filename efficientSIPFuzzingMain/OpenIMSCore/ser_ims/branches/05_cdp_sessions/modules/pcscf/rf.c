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
#include "rf.h"

#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"
#include "../cdp/acct.h"
#include "rf_avp.h"
#include "sip.h"

#include "offline_charging.h"

/**< Structure with pointers to tm funcs */
extern struct tm_binds tmb;

/**< Structure with pointers to cdp funcs */
extern struct cdp_binds cdpb;

/**< Offline Charging flag */
extern struct offline_charging_flag cflag;          

/**< FQDN of CDF for offline charging, defined in mod.c */
extern str cdf_peer;	

/* default configuration of pcscf, used to generate AVPs. */
static str service_context_id = {"32260@3gpp.org", 14};
static unsigned int role_of_node = AVP_IMS_ORIGINATING_ROLE;
static unsigned int node_functionality = AVP_IMS_P_CSCF;
static unsigned int offset = 2208988800; /* seconds between 01.01.1900, 01.01.1970 */



/*
 *******************************************************************************
 * Auxiliary functions to create grouped AVPs
 *******************************************************************************
 */
/* 
 * SIP Request Method --> Event Type:
 * 
 * <Event-Type> = [SIP-Method]
 *                [Event]
 *                [Expires]
 * 
 */
int add_event_type(struct sip_msg* msg, AAA_AVP_LIST* outl, AAA_AVP_LIST* inl)
{
	str method = cscf_get_sip_method(msg);
	if (!Rf_add_sip_method(inl, method)) goto error;
			
	str event = cscf_get_event(msg);
	if (event.len) 
		if (!Rf_add_event(inl, event)) goto error;
		
		
	int expires = cscf_get_expires_hdr(msg);	
	if (expires >= 0) 
		if (!Rf_add_expires(inl, expires)) goto error;
		
			
	if (!Rf_add_event_type(outl, inl)) goto error;

	return 1;

error:
	return 0;	

}



/* 
 * P-Charging-Vector: orig-ioi -> Originating-IOI
 * P-Charging-Vector: term-ioi -> Terminating-IOI
 * 
 * <Inter-Operator-Identifier> = [Originating-IOI]
 * 								 [Terminating-IOI]
 *
 */
int add_inter_operator_identifier(struct sip_msg* msg, AAA_AVP_LIST* outl, 
		AAA_AVP_LIST* inl)
{
	str orig_ioi = cscf_get_p_charging_vector_orig_ioi(msg);
	if (orig_ioi.len)
		if (!Rf_add_originating_ioi(inl, orig_ioi)) goto error;
		
	str term_ioi = cscf_get_p_charging_vector_term_ioi(msg);
	if (term_ioi.len)
		if (!Rf_add_terminating_ioi(inl, term_ioi)) goto error;

	if (!Rf_add_inter_operator_identifier(outl, inl)) goto error;

	return 1;

error:
	return 0;
	
}



/* 
 * Timestamp from REQ -> SIP-Request-Timestamp
 * Timestamp from RES -> SIP-Response-Timestamp
 * 
 * <Time-Stamps> = [SIP-Request-Timestamp]
 * 				   [SIP-Response-Timestamp]
 *
 */
int add_time_stamps(struct sip_msg* req, struct sip_msg* res,
								  AAA_AVP_LIST* outl, AAA_AVP_LIST* inl)
{
	
	unsigned int req_t, res_t;
	
	/*
	 * TODO SIP request/respons should have Timestamp header
	 * in order to create this AVP 
	 */
	//req_t = cscf_get_timestamp(req);
	//res_t = cscf_get_timestamp(res);
	req_t = 0;
	res_t = 0;
	
	if (!Rf_add_sip_request_timestamp(inl, req_t + offset)) goto error;
	if (!Rf_add_sip_response_timestamp(inl, res_t + offset)) goto error;
	if (!Rf_add_time_stamps(outl, inl)) goto error;

	return 1;

error:
	return 0;
	
}

/*
 *******************************************************************************
 * Rf_ACR_event/start/interim/stop
 *******************************************************************************
 */
AAAMessage* Rf_ACR_event(struct sip_msg* req, struct sip_msg* res) 
{
	AAAMessage *acr = 0;
	AAASessionId sessId = {0,0};
	AAAMessage *aca = 0;

	sessId = cdpb.AAACreateSession();
	acr = cdpb.AAACreateRequest(IMS_Rf,Code_AC,Flag_Proxyable,&sessId);
	
	if (!acr) goto error;
	
	/*---------- 1. Add mandatory AVPs, tags are not checked ----------*/
	
	/* Session-Id, Origin-Host, Origin-Realm AVP are added by the stack. */
	
	/* Add Destination-Realm AVP */
	str realm = cscf_get_realm_from_ruri(req);
	if (!Rf_add_destination_realm(acr, realm)) goto error;
	
	/* Add Accounting-Record-Type AVP */
	if (!Rf_add_accounting_record_type(acr, AAA_ACCT_EVENT)) goto error;
	
	/* Add Accounting-Record-Number AVP */
	if (!Rf_add_accounting_record_number(acr, 0)) goto error;
	
	/* Add Acct-Application-Id AVP */
	if (!Rf_add_acct_application_id(acr, IMS_Rf)) goto error;
	
	/* Add Event-Timestamp AVP */
	if (cflag.cf_ietf & CF_IETF_EVENT_TIME_STAMP) {
		time_t tm;
		time(&tm);
		if (!Rf_add_event_timestamp(acr, tm + offset)) goto error;
	}
	
	/* Add Service-Context-Id, see TS32.299 V740 7.1.7 */
	if (!Rf_add_service_context_id(acr, service_context_id)) goto error; 

	
	
	/*---------- 2. Create and add Service-Information AVP ----------*/
	
	/*
	 *  See 3GPP TS32.299 V7.5.0:
	 * 
	 *  <Service-Information> = [IMS-Information]
	 * 								 [Event-Type]
	 * 								 [Role-Of-Node]
	 * 								 [Node-Functionality]
	 * 								 [User-Session-Id]
	 * 								*[Calling-Party-Address]
	 * 								 [Called-Party-Address]
	 * 								 [Called-Asserted-Identity]
	 * 								 [Requested-Party-Address]
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
	 *								*[Early-Media-Description]
	 */
	 			 	
	AAA_AVP_LIST ls_ims; /* AVP list for member AVPs in IMS-Information AVP */
	ls_ims.head=0; ls_ims.tail = 0;

	AAA_AVP_LIST ls_ser; /* AVP list for member AVPs in Service-Information AVP */
	ls_ser.head=0; ls_ser.tail = 0;
	
	
	AAA_AVP_LIST ls_tmp;
	ls_tmp.head=0; ls_tmp.tail = 0;
	
	/* To decide which AVP should be created based on the charging flag. */
	
	/* SIP Request Method --> Event Type */
	if (cflag.cf_3gpp & CF_3GPP_EVENT_TYPE) 
		if (!add_event_type(req, &ls_ims, &ls_tmp)) goto error; 
	
	/* Role of Node created by P-CSCF directly. */
	if(!Rf_add_role_of_node(&ls_ims, role_of_node)) goto error;
	
	/* Node functionality created by P-CSCF directly */
	if(!Rf_add_node_functionality(&ls_ims, node_functionality)) goto error;
	
	/*  Call-ID --> User-Session-ID AVP */
	if (cflag.cf_3gpp & CF_3GPP_USER_SESSION_ID) {
		str user_session_id = cscf_get_call_id(req, 0);
		if (user_session_id.len)
			if (!Rf_add_user_session_id(&ls_ims, user_session_id)) goto error;
	}
		
	/* P-Asserted-Identity --> Calling-Party-Address AVP */
	/*
	 * Obtained from P-Asserted-Identity of non-REGISTER SIP request.
	 * 
	 * TODO may appear several times when the P-Asserted-Identity header
	 * contains both a SIP URI and a TEL URI.
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLING_PARTY_ADDRESS) && 
		(req->REQ_METHOD != METHOD_REGISTER)) {
		str calling_addr = cscf_get_asserted_identity(req);
		if (calling_addr.len)
			if (!Rf_add_calling_party_address(&ls_ims, calling_addr)) goto error; 
	}
	
	/* Requst URI / To URI --> Called-Party-Address AVP */
	/* 
	 * If REG from To URI
	 * otherwise from Request URI
	 */
	if (cflag.cf_3gpp & CF_3GPP_CALLED_PARTY_ADDRESS) {
		
		str called_addr;
		
		if (req->REQ_METHOD & METHOD_REGISTER) {
			called_addr = cscf_get_public_identity(req);
		} else {
			called_addr = cscf_get_public_identity_from_requri(req);
		}
		
		if (called_addr.len)
			if (!Rf_add_called_party_address(&ls_ims, called_addr)) goto error;
	}
	
	/* P-Associated-URI --> Asssocited URI AVP */
	/* 
	 * If REG, from P-Associated-URIs in 200 OK
	 * TODO may appear several times, when the P-Associated-URI header contains
	 * more than one public user identity.
	 */
	if ((cflag.cf_3gpp & CF_3GPP_ASSOCIATED_URI) &&
		(req->REQ_METHOD & METHOD_REGISTER)) {
		
		str associated_uri;
		if (cscf_get_first_p_associated_uri(res, &associated_uri))
			if (associated_uri.len)
				if (!Rf_add_associated_uri(&ls_ims, associated_uri)) goto error; 	
			
		//str **associated_uri;
		//int cnt;
		//int i;
		
		//if (cscf_get_p_associated_uri(res, associated_uri, &cnt)) {
		//	for (i=0; i < cnt; i++) {
		//		if ((*associated_uri)[i].len) {
		//			if (!Rf_add_associated_uri(&ls_ims, (*associated_uri)[i])) goto error; 	
		//		}
		//	}
		//}
		//pkg_free(associated_uri);
	}
	
	/* Add Timestamps AVP */
	if (cflag.cf_3gpp & CF_3GPP_TIME_STAMPS)
		if (!add_time_stamps(req, res, &ls_ims, &ls_tmp)) goto error;
	
	/* Inter-Operator-Identifier AVP is got from P-Charging-Vector
	 *   	P-Charging-Vector: orig-ioi -> Originating-IOI
	 * 		P-Charging-Vector: term-ioi -> Terminating-IOI
	 */
	if (cflag.cf_3gpp & CF_3GPP_INTER_OPERATOR_IDENTIFIER)
		if (!add_inter_operator_identifier(req, &ls_ims, &ls_tmp)) goto error;
	
	/* P-Charging-Vector: icid-value -> IMS-Charging-Identifier AVP */
	if (cflag.cf_3gpp & CF_3GPP_IMS_CHARGING_IDENTIFIER) {
		str icid = cscf_get_p_charging_vector_icid(req);
		if (icid.len)
			if (!Rf_add_ims_charging_identifier(&ls_ims, icid)) goto error; 
	}
	
	/* IP from Contact -> Served-Party-IP-Address */
	if (cflag.cf_3gpp & CF_3GPP_SERVED_PARTY_IP_ADDRESS) {
	//	str ip_contact = cscf_get_ip_contact(req);
	//	if (ip_contact.len)
	//		if (!Rf_add_served_party_ip_address(&ls_ims, ip_contact)) goto error;
	}
	
	/* Cause-Code AVP */
	/* If this function is called, registration successful */
	if (cflag.cf_3gpp & CF_3GPP_CAUSE_CODE)
		if (!Rf_add_cause_code(&ls_ims, -1)) goto error;
		
	/* P-Access-Network-Info -> Access-Network-Information AVP */
	if (cflag.cf_3gpp & CF_3GPP_ACCESS_NETWORK_INFORMATION) {
		str access_network_info = cscf_get_p_access_network_info(req);	
		if (access_network_info.len)
			if (!Rf_add_access_network_information(&ls_ims, access_network_info))
				goto error;
	}
	
	if (!Rf_add_ims_information(&ls_ser, &ls_ims)) goto error;
	if (!Rf_add_service_information(acr, &ls_ser)) goto error;
	
	/*---------- 3. Send ACR to CDF ----------*/
	aca = cdpb.AAASendRecvMessage(acr, &cdf_peer);
	
	/*---------- 4. Destroy the session ----------*/
	cdpb.AAADropSession(&sessId);
	
	return aca;
	
error:
	//free stuff
	if (sessId.s) cdpb.AAADropSession(&sessId);
	if (acr) cdpb.AAAFreeMessage(&acr);
	return 0;
}



AAAMessage* Rf_ACR_start(struct sip_msg* req, struct sip_msg* res) 
{
	return 0;
}

 
AAAMessage* Rf_ACR_interim(struct sip_msg* req, struct sip_msg* res) 
{
	return 0;
}

AAAMessage* Rf_ACR_stop(struct sip_msg *req, struct sip_msg* res) 
{
	return 0;
}

