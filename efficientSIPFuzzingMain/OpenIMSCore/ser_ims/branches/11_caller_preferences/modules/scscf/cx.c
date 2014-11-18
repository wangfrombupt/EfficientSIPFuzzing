/*
 * $Id: cx.c 452 2007-09-26 16:16:48Z vingarzan $
 *  
 * Copyright (C) 2004-2006 FhG Fokus
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
 
/**
 * \file
 * 
 * Serving-CSCF - Cx Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#include "cx.h"

#include "../tm/tm_load.h"
#include "cx_avp.h"
#include "sip.h"
#include "registrar_storage.h"
#include "registrar_parser.h"
#include "registration.h"
#include "ims_pm_scscf.h"

extern struct tm_binds tmb;			/**< Structure with pointers to tm funcs 		*/
extern struct cdp_binds cdpb;		/**< Structure with pointers to cdp funcs 		*/

extern str scscf_forced_hss_peer_str;		/**< FQDN of the forced Diameter Peer (HSS) */
 
extern int r_hash_size;				/**< records tables parameters 	*/
extern r_hash_slot *registrar;		/**< the contacts */

/**
 * Handler for incoming Diameter answers.
 * This is not used as all diameter answers are handled transactionaly
 * @param response - received response
 * @param t - transaction
 * @returns 1
 */
int CxAnswerHandler(AAAMessage *response, AAATransaction *t)
{
	#ifdef WITH_IMS_PM
		ims_pm_diameter_answer(response);
	#endif		
	switch(response->commandCode){
		default:
			LOG(L_ERR,"ERR:"M_NAME":CxAnswerHandler: Unkown Command Code %d\n",
				response->commandCode);
	}
	return 1;
}

/**
 * Handler for incoming Diameter failures
 * This is not used as all diameter failures are handled transactionaly
 * @param t - transaction
 * @param reason - failure reason
 * @returns 1
 */
int CxFailureHandler(AAATransaction *t,int reason)
{
	LOG(L_INFO,"INF:"M_NAME":AAAFailureHandler:  SIP transaction %u %u Reason %d\n",
		t->hash,t->label,reason);
	switch(t->command_code){
		default:
		LOG(L_ERR,"ERR:"M_NAME":CxFailureHandler: Unkown Command Code %d\n",
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
AAAMessage* CxRequestHandler(AAAMessage *request,void *param)
{
	if (is_req(request)){		
		LOG(L_INFO,"INFO:"M_NAME":CxRequestHandler(): We have received a request\n");
		#ifdef WITH_IMS_PM
			ims_pm_diameter_request(request);
		#endif		
		switch(request->applicationId){
        	case IMS_Cx:
				switch(request->commandCode){				
					case IMS_RTR:
						LOG(L_INFO,"INFO:"M_NAME":CxRequestHandler():- Received an IMS_RTR \n");
						return Cx_RTA(request);
						break;
					case IMS_PPR:
						LOG(L_INFO,"INFO:"M_NAME":CxRequestHandler(): - Received an IMS_PPR \n");
						return Cx_PPA(request);
						break;
					default :
						LOG(L_ERR,"ERR:"M_NAME":CxRequestHandler(): - Received unknown request for Cx command %d\n",request->commandCode);
						break;	
				}
				break;
			default:
				LOG(L_ERR,"ERR:"M_NAME":CxRequestHandler(): - Received unknown request for app %d command %d\n",
					request->applicationId,
					request->commandCode);
				break;				
		}					
	}
	return 0;		
}


static str s_empty={0,0};
extern str auth_scheme_types[];

/**
 * Create and send a Multimedia-Authentication-Request and returns the Answer received for it.
 * This function retrieves authentication vectors from the HSS.
 * @param msg - the SIP message to send for
 * @parma public_identity - the public identity of the user
 * @param private_identity - the private identity of the user
 * @param count - how many authentication vectors to ask for
 * @param algorithm - for which algorithm
 * @param authorization - the authorization value
 * @param server_name - local name of the S-CSCF to save on the HSS
 * @param realm - Realm of the user
 * @returns the MAA
 */ 
AAAMessage *Cx_MAR(struct sip_msg *msg, str public_identity, str private_identity,
					unsigned int count,str algorithm,str authorization,str server_name,str realm)
{
	AAAMessage *mar=0,*maa=0;
	AAASessionId sessId={0,0};
	AAATransaction *trans=0;
	unsigned int hash=0,label=0;	
	
	sessId = cdpb.AAACreateSession();
	trans=cdpb.AAACreateTransaction(IMS_Cx,IMS_MAR);

	mar = cdpb.AAACreateRequest(IMS_Cx,IMS_MAR,Flag_Proxyable,&sessId);
	if (!mar) goto error;

	if (!Cx_add_destination_realm(mar,realm)) goto error;
		
	if (!Cx_add_vendor_specific_appid(mar,IMS_vendor_id_3GPP,IMS_Cx,0 /*IMS_Cx*/)) goto error;
	if (!Cx_add_auth_session_state(mar,1)) goto error;		
		
	if (!Cx_add_public_identity(mar,public_identity)) goto error;
	if (!Cx_add_user_name(mar,private_identity)) goto error;
	if (!Cx_add_sip_number_auth_items(mar, count)) goto error;
	if (algorithm.len==auth_scheme_types[AUTH_HTTP_DIGEST_MD5].len &&
		strncasecmp(algorithm.s,auth_scheme_types[AUTH_HTTP_DIGEST_MD5].s,algorithm.len)==0) {
		if (!Cx_add_sip_auth_data_item_request(mar, algorithm, authorization, private_identity, realm, 
			msg->first_line.u.request.method, server_name)) goto error;
	}else{
		if (!Cx_add_sip_auth_data_item_request(mar, algorithm, authorization, private_identity, realm, 
			msg->first_line.u.request.method, s_empty)) goto error;		
	}
	if (!Cx_add_server_name(mar,server_name)) goto error;
	//TODO - add the realm also - and don't add when sending if added here 
		
	if (tmb.t_get_trans_ident(msg,&hash,&label)<0){	
		LOG(L_ERR,"INF:"M_NAME":Cx_MAR: SIP message without transaction... very strange\n");
		return 0;
	}

	trans->hash=hash;
	trans->label=label;
	trans->application_id=mar->applicationId;
	trans->command_code=mar->commandCode;
	
	#ifdef WITH_IMS_PM
		ims_pm_diameter_request(mar);
	#endif				
	if (scscf_forced_hss_peer_str.len)
		maa = cdpb.AAASendRecvMessageToPeer(mar,&scscf_forced_hss_peer_str);
	else 
		maa = cdpb.AAASendRecvMessage(mar);
	#ifdef WITH_IMS_PM
		ims_pm_diameter_answer(maa);
	#endif			
	
	cdpb.AAADropSession(&sessId);
	cdpb.AAADropTransaction(trans);
	
	return maa;
	
error:
	//free stuff
	if (trans) cdpb.AAADropTransaction(trans);
	if (sessId.s) cdpb.AAADropSession(&sessId);
	if (mar) cdpb.AAAFreeMessage(&mar);
	return 0;	
}


/**
 * Create and send a Server-Assignment-Request and returns the Answer received for it.
 * This function performs the Server Assignment operation.
 * @param msg - the SIP message to send for
 * @parma public_identity - the public identity of the user
 * @param server_name - local name of the S-CSCF to save on the HSS
 * @param realm - Realm of the user
 * @param assignment_type - type of the assignment
 * @param data_available - if the data is already available
 * @returns the SAA
 */
AAAMessage *Cx_SAR(struct sip_msg *msg, str public_identity, str private_identity,
					str server_name,str realm,int assignment_type, int data_available)
{
	AAAMessage *sar=0,*saa=0;
	AAASessionId sessId={0,0};
	AAATransaction *trans=0;
//	struct cell* t;
	unsigned int hash=0,label=0;	
	
	sessId = cdpb.AAACreateSession();
	trans=cdpb.AAACreateTransaction(IMS_Cx,IMS_SAR);

	sar = cdpb.AAACreateRequest(IMS_Cx,IMS_SAR,Flag_Proxyable,&sessId);
	if (!sar) goto error;

	if (!Cx_add_destination_realm(sar,realm)) goto error;
		
	if (!Cx_add_vendor_specific_appid(sar,IMS_vendor_id_3GPP,IMS_Cx,0 /*IMS_Cx*/)) goto error;
	if (!Cx_add_auth_session_state(sar,1)) goto error;		
		
	if (!Cx_add_public_identity(sar,public_identity)) goto error;
	if (!Cx_add_server_name(sar,server_name)) goto error;
	if (private_identity.len)
		if (!Cx_add_user_name(sar,private_identity)) goto error;
	if (!Cx_add_server_assignment_type(sar,assignment_type)) goto error;
	if (!Cx_add_userdata_available(sar,data_available)) goto error;
	
	if (msg&&tmb.t_get_trans_ident(msg,&hash,&label)<0){	
		// it's ok cause we can call this async with a message
		//LOG(L_ERR,"INF:"M_NAME":Cx_MAR: SIP message without transaction... very strange\n");
		//return 0;
	}

	trans->hash=hash;
	trans->label=label;
	trans->application_id=sar->applicationId;
	trans->command_code=sar->commandCode;
	
	#ifdef WITH_IMS_PM
		ims_pm_diameter_request(sar);
	#endif				
	if (scscf_forced_hss_peer_str.len)
		saa = cdpb.AAASendRecvMessageToPeer(sar,&scscf_forced_hss_peer_str);
	else 
		saa = cdpb.AAASendRecvMessage(sar);
	#ifdef WITH_IMS_PM
		ims_pm_diameter_answer(saa);
	#endif				
	
	cdpb.AAADropSession(&sessId);
	cdpb.AAADropTransaction(trans);
	
	return saa;
	
error:
	//free stuff
	if (trans) cdpb.AAADropTransaction(trans);
	if (sessId.s)	cdpb.AAADropSession(&sessId);
	if (sar) cdpb.AAAFreeMessage(&sar);
	return 0;	
}

/**
 * Process a Registration Termination Request and return the Answer for it.
 * @param rtr - the RTR Diameter request
 * @returns the RTA Diameter answer
 */
AAAMessage* Cx_RTA(AAAMessage * rtr)
{	
	AAAMessage	*rta_msg;
	AAA_AVP* avp;
	str public_id;
	str private_id;
	
	rta_msg	= cdpb.AAACreateResponse(rtr);//session ID?
	if (!rta_msg) return 0;

	avp = Cx_get_next_public_identity(rtr,0,AVP_IMS_Public_Identity,IMS_vendor_id_3GPP,__FUNCTION__);	
	if(avp==0){
		private_id=Cx_get_user_name(rtr);	
		r_private_expire(private_id);			 
	}else{
		public_id=avp->data;
		r_public_expire(public_id);
		while(cdpb.AAAGetNextAVP(avp) && (avp=Cx_get_next_public_identity(rtr,cdpb.AAAGetNextAVP(avp),AVP_IMS_Public_Identity,IMS_vendor_id_3GPP,__FUNCTION__))!=0){
			public_id=avp->data;
			r_public_expire(public_id);
		}		
	}
	Cx_add_vendor_specific_appid(rta_msg,IMS_vendor_id_3GPP,IMS_Cx,0 /*IMS_Cx*/);
	Cx_add_auth_session_state(rta_msg,1);		

	/* send an RTA back to the HSS */
	Cx_add_result_code(rta_msg,DIAMETER_SUCCESS);
	#ifdef WITH_IMS_PM
		ims_pm_diameter_answer(rta_msg);
	#endif		
	
	return rta_msg;
}


/**
 * Process a Push Profile Request and return the Answer for it.
 * @param ppr - the PPR Diameter request
 * @returns the PPA Diameter answer
 */
AAAMessage* Cx_PPA(AAAMessage * ppr)
{
	AAAMessage	*ppa_msg;
	str ppr_data;
	ims_subscription *imss;
	int i,j;
	r_public *pu;
	str ccf1,ccf2,ecf1,ecf2;

	ppa_msg	= cdpb.AAACreateResponse(ppr);
	if (!ppa_msg) return 0;	
	
	if((ppr_data=Cx_get_user_data(ppr)).len != 0){
		LOG(L_INFO,"INFO:"M_NAME":Cx_PPA(): Received a User_Data PPR!\n");
		imss=parse_user_data(ppr_data);
		print_user_data(L_ALERT,imss);
		
		for(i=0;i<imss->service_profiles_cnt;i++)
			for(j=0;j<imss->service_profiles[i].public_identities_cnt;j++){				
				pu = update_r_public(imss->service_profiles[i].public_identities[j].public_identity,
					0,&imss,0,0,0,0);
				if (!pu) continue;
				r_unlock(pu->hash);
			}			
	}
	else{
		if (Cx_get_charging_info(ppr,&ccf1,&ccf2,&ecf1,&ecf2)){
			LOG(L_INFO,"INFO:"M_NAME":Cx_PPA(): Received a Charging Info PPR - NOT IMPLEMENTED\n");
			//TODO find all r_public that should be updated and update
		}
	}	
	Cx_add_vendor_specific_appid(ppa_msg,IMS_vendor_id_3GPP,IMS_Cx,0 /*IMS_Cx*/);
	Cx_add_auth_session_state(ppa_msg,1);		
	
	Cx_add_result_code(ppa_msg,DIAMETER_SUCCESS);
	#ifdef WITH_IMS_PM
		ims_pm_diameter_answer(ppa_msg);
	#endif			
	return ppa_msg;
}

