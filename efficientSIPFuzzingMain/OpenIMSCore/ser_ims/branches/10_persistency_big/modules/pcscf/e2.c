/*
 * $Id: e2.c 297 2007-05-31 11:30:05Z vingarzan $
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
 * Serving-CSCF - e2 Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#include "e2.h"

#include "../tm/tm_load.h"
#include "e2_avp.h"
#include "sip.h"
#include "registrar_storage.h"
//#include "registrar_parser.h"
#include "registration.h"

extern struct tm_binds tmb;			/**< Structure with pointers to tm funcs 		*/
extern struct cdp_binds cdpb;		/**< Structure with pointers to cdp funcs 		*/
extern int pcscf_use_e2;

extern str forced_clf_peer_str;		/**< FQDN of the Diameter Peer (HSS) */
 
extern int r_hash_size;				/**< records tables parameters 	*/
extern r_hash_slot *registrar;		/**< the contacts */

/**
 * Handler for incoming Diameter answers.
 * This is not used as all diameter answers are handled transactionaly
 * @param response - received response
 * @param t - transaction
 * @returns 1
 */
int e2AnswerHandler(AAAMessage *response, AAATransaction *t)
{
	switch(response->commandCode){
		default:
			LOG(L_ERR,"ERR:"M_NAME":e2AnswerHandler: Unkown Command Code %d\n", response->commandCode);
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
int e2FailureHandler(AAATransaction *t,int reason)
{
	LOG(L_INFO,"INF:"M_NAME":AAAFailureHandler:  SIP transaction %u %u Reason %d\n",
		t->hash,t->label,reason);
	switch(t->command_code){
		default:
		LOG(L_ERR,"ERR:"M_NAME":e2FailureHandler: Unkown Command Code %d\n",
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
AAAMessage* e2RequestHandler(AAAMessage *request,void *param)
{
	if (is_req(request)){		
		LOG(L_INFO,"INFO:"M_NAME":e2RequestHandler(): We have received a request\n");
		switch(request->applicationId){
        	case IMS_e2:
				switch(request->commandCode){				
					default :
						LOG(L_ERR,"ERR:"M_NAME":e2RequestHandler(): - Received unknown request for e2 command %d\n",request->commandCode);
						break;	
				}
				break;
			default:
				LOG(L_ERR,"ERR:"M_NAME":e2RequestHandler(): - Received unknown request for app %d command %d\n",
					request->applicationId,
					request->commandCode);
				break;				
		}					
	}
	return 0;		
}


//static str s_empty={0,0};
//extern str auth_scheme_types[];

/**
 * Create and send a User-Data-Request and returns the Answer received for it.
 * @param msg - the SIP message to send for
 * @param ip - ip address of the UE
 * @param address_realm - Realm of the user
 * @param private_identity - user private identity
 * @param server_name - pcscf name
 * @param dest_realm - pcscf realm
 * @returns the MAA
 */ 
AAAMessage *e2_UDR(struct sip_msg *msg, str ip, str address_realm, str private_identity, str server_name,  str dest_realm)
{
	
	if (!pcscf_use_e2)
		return 0;
	
	AAAMessage *udr=0,*uda=0;
	AAASessionId sessId={0,0};
	AAATransaction *trans=0;
	unsigned int hash=0,label=0;	
	
	sessId = cdpb.AAACreateSession();
	trans=cdpb.AAACreateTransaction(IMS_e2,IMS_UDR);

	udr = cdpb.AAACreateRequest(IMS_e2,IMS_UDR,Flag_Proxyable,&sessId);
	if (!udr) goto error;

	if (!e2_add_destination_realm(udr,dest_realm)) goto error;
		
	if (!e2_add_vendor_specific_appid(udr,IMS_vendor_id_3GPP,IMS_e2,0 )) goto error;
	if (!e2_add_auth_session_state(udr,1)) goto error;			
	
	if (!e2_add_user_name(udr,private_identity)) goto error;
	//if (!e2_add_server_name(udr,server_name)) goto error;
	
	if (!e2_add_g_unique_address(udr,ip, address_realm)) goto error;
	
	if (!e2_add_app_identifier(udr, server_name)) goto error;
		
	if (tmb.t_get_trans_ident(msg,&hash,&label)<0){	
		LOG(L_ERR,"INF:"M_NAME":e2_udr: SIP message without transaction... very strange\n");
		return 0;
	}

	trans->hash=hash;
	trans->label=label;
	trans->application_id=udr->applicationId;
	trans->command_code=udr->commandCode;
	
	if (forced_clf_peer_str.len)
		uda = cdpb.AAASendRecvMessageToPeer(udr,&forced_clf_peer_str);
	else
		uda = cdpb.AAASendRecvMessage(udr);
	
	cdpb.AAADropSession(&sessId);
	cdpb.AAADropTransaction(trans);
	
	return uda;
	
	
error:
	//free stuff
	if (trans) cdpb.AAADropTransaction(trans);
	if (sessId.s) cdpb.AAADropSession(&sessId);
	if (udr) cdpb.AAAFreeMessage(&udr);
	return 0;	
}
