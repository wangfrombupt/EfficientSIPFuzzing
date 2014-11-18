/**
 * $Id: cx.c 187 2007-03-09 13:05:47Z vingarzan $
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
 * Interrogating-CSCF - Cx operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#include "cx.h"

#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"
#include "cx_avp.h"
#include "sip.h"
#include "registration.h"
#include "location.h"

extern struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/
//extern int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 
										/**< link to the stateless reply function in sl module */

extern struct cdp_binds cdpb;            /**< Structure with pointers to cdp funcs 		*/

str aaa_peer;	/**< FQDN of the Diameter peer to send requests to */

/**
 * Handler for incoming Diameter responses.
 * Not used now as all answers are handled by transactional callbacks.
 * @param response - the diameter response
 * @param param - generic parameter - here the AAATransaction
 */
void CxAnswerHandler(AAAMessage *response, void *param)
{
	AAATransaction *t;
	t = (AAATransaction*)param;
	if (!param){
		LOG(L_ERR,"ERR:CxAnswerHandler: Answer received but no transaction\n");
		return;
	}

	switch(response->commandCode){
		default:
			LOG(L_ERR,"ERR:CxAnswerHandler: Unkown Command Code %d\n",
				response->commandCode);
	}
	return;
}

/**
 * Handler for Diameter Failures.
 * Not used now as all answers are handled by transactional callbacks.
 * @param is_tiemout - if this is a time-out
 * @param param - generic parameter - here the AAATransaction
 * @param ans - the Diameter answer if available, NULL if not
 */
void CxFailureHandler(int is_timeout,void *param,AAAMessage *ans)
{
	AAATransaction *t;
	t = (AAATransaction*)param;
	/* filter only failures */
//	if (!is_timeout) return;
	if (!param){
		LOG(L_ERR,"ERR:CxFailureHandler: NULL transaction parameter received\n");
		return;
	}
	switch(t->command_code){
		default:
		LOG(L_ERR,"ERR:CxFailureHandler: Unkown Command Code %d\n",
			t->command_code);
	}
	cdpb.AAADropTransaction(t);
	return;
}

/**
 * Handler for Diameter incoming requests
 * Not used now as here there are no expected incoming requests
 * @param request - the diameter response
 * @param param - generic parameter - here the AAATransaction
 */
AAAMessage* CxRequestHandler(AAAMessage *request,void *param)
{
	LOG(L_INFO,"INF:CxSIRequestHandler: NOT IMPLEMENTED\n");
	
	return 0;	
}



/**
 * Sends an UAR and returns the UAA.
 * @param msg - the SIP message
 * @param private_identity - the username
 * @param public_identity - the public identity
 * @param visited_network_id - id of the roaming network
 * @param authorization_type - if registration or de-registration
 * @param realm - Realm
 * @returns the UAA message received or NULL on error
 */
AAAMessage* Cx_UAR(struct sip_msg *msg,str private_identity, str public_identity, str visited_network_id,
			int authorization_type,str realm)
{
	AAAMessage *uar=0;
	AAASessionId sessId={0,0};
	AAAMessage *uaa=0;
	
	sessId = cdpb.AAACreateSession();

	uar = cdpb.AAACreateRequest(IMS_Cx,IMS_UAR,Flag_Proxyable,&sessId);
	if (!uar) goto error;

	if (!Cx_add_destination_realm(uar,realm)) goto error;
		
	if (!Cx_add_vendor_specific_appid(uar,IMS_vendor_id_3GPP,IMS_Cx,0/*IMS_Cx*/)) goto error;
	if (!Cx_add_auth_session_state(uar,1)) goto error;

	if (!Cx_add_user_name(uar,private_identity)) goto error;
	if (!Cx_add_public_identity(uar,public_identity)) goto error;
	if (!Cx_add_visited_network_id(uar,visited_network_id)) goto error;
	if (authorization_type!=AVP_IMS_UAR_REGISTRATION)
		if (!Cx_add_authorization_type(uar,authorization_type)) goto error;
			
	uaa = cdpb.AAASendRecvMessage(uar,&aaa_peer);

	cdpb.AAADropSession(&sessId);
	
	return uaa;
	
error:
	//free stuff
	if (sessId.s) cdpb.AAADropSession(&sessId);
	if (uar) cdpb.AAAFreeMessage(&uar);
	return 0;
}	



/**
 * Sends an LIR and returns the LIA.
 * @param msg - the SIP message
 * @param public_identity - the public identity
 * @param realm - Realm
 * @returns the LIA message received or NULL on error
 */
AAAMessage* Cx_LIR(struct sip_msg *msg, str public_identity,str realm)
{
	AAAMessage *lir=0,*lia=0;
	AAASessionId sessId={0,0};
	
	sessId = cdpb.AAACreateSession();

	lir = cdpb.AAACreateRequest(IMS_Cx,IMS_LIR,Flag_Proxyable,&sessId);
	if (!lir) goto error;

	if (!Cx_add_destination_realm(lir,realm)) goto error;
	
	if (!Cx_add_vendor_specific_appid(lir,IMS_vendor_id_3GPP,IMS_Cx,0/*IMS_Cx*/)) goto error;
	if (!Cx_add_auth_session_state(lir,1)) goto error;		

	if (!Cx_add_public_identity(lir,public_identity)) goto error;
			
	lia = cdpb.AAASendRecvMessage(lir,&aaa_peer);

	cdpb.AAADropSession(&sessId);
	
	return lia;
	
error:
	//free stuff
	if (sessId.s) cdpb.AAADropSession(&sessId);
	if (lir) cdpb.AAAFreeMessage(&lir);
	return 0;
}	

