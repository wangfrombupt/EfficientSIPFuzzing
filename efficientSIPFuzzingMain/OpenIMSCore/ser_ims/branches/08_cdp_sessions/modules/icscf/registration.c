/**
 * $Id: registration.c 396 2007-07-19 18:09:57Z placido $
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
 * Interrogating-CSCF - User-Authorization Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include <stdlib.h>

#include "registration.h"

#include "../../mem/shm_mem.h"
#include "../../dset.h"

#include "mod.h"
#include "sip.h"
#include "cx.h"
#include "cx_avp.h"

extern struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/
//extern int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 
										/**< link to the stateless reply function in sl module */
extern struct cdp_binds cdpb;            /**< Structure with pointers to cdp funcs 		*/
										
/**
 * Perform User Authorization Request.
 * creates and send the user authorization query
 * @param msg - the SIP message
 * @param str1 - the realm
 * @param str2 - if to do capabilities
 * @returns true if OK, false if not
 */
int I_UAR(struct sip_msg* msg, char* str1, char* str2)
{
	int result=CSCF_RETURN_FALSE;
	str private_identity,public_identity,visited_network_id;
	int authorization_type=AVP_IMS_UAR_REGISTRATION;	
	int expires=3600;
	struct hdr_field *hdr ;
	str realm;
	AAAMessage* uaa;
	
	realm = cscf_get_realm_from_ruri(msg);
	
	LOG(L_DBG,"DBG:"M_NAME":I_UAR: Starting ... <%.*s>\n",realm.len,realm.s);
	/* check if we received what we should */
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_ERR,"ERR:"M_NAME":I_UAR: The message is not a request\n");
		goto done;
	}
	if (msg->first_line.u.request.method.len!=8||
		memcmp(msg->first_line.u.request.method.s,"REGISTER",8)!=0)
	{
		LOG(L_ERR,"ERR:"M_NAME":I_UAR: The method is not a REGISTER\n");
		goto done;		
	}
	
	/* extract data from message */
	private_identity=cscf_get_private_identity(msg,realm);
	if (!private_identity.len) {
		LOG(L_ERR,"ERR:"M_NAME":I_UAR: Private Identity not found, responding with 400\n");
		cscf_reply_transactional(msg,400,MSG_400_NO_PRIVATE);
		result=CSCF_RETURN_BREAK;
		goto done;		
	}
	
	public_identity=cscf_get_public_identity(msg);
	if (!public_identity.len) {
		LOG(L_ERR,"ERR:"M_NAME":I_UAR: Public Identity not found, responding with 400\n");
		cscf_reply_transactional(msg,400,MSG_400_NO_PUBLIC);
		result=CSCF_RETURN_BREAK;
		goto done;		
	}
	
	visited_network_id=cscf_get_visited_network_id(msg , &hdr);
	if (!visited_network_id.len) {
		LOG(L_ERR,"ERR:"M_NAME":I_UAR: Visited Network Identity not found, responding with 400\n");
		cscf_reply_transactional(msg,400,MSG_400_NO_VISITED);
		result=CSCF_RETURN_BREAK;
		goto done;		
	}
	
	
	if (atoi(str1)) authorization_type=AVP_IMS_UAR_REGISTRATION_AND_CAPABILITIES;
	else {
		expires = cscf_get_max_expires(msg);
		if (expires == 0) authorization_type=AVP_IMS_UAR_DE_REGISTRATION;
	}
	
	uaa = Cx_UAR(msg,private_identity,public_identity,visited_network_id, 
				authorization_type,realm);
	if (!uaa){
		LOG(L_ERR,"ERR:"M_NAME":I_UAR: Error creating/sending UAR or UAR time-out\n");
		cscf_reply_transactional(msg,480,MSG_480_DIAMETER_ERROR);
		result=CSCF_RETURN_BREAK;
		goto done;		
	}

	result = I_UAA(msg,uaa);	
	if (uaa) cdpb.AAAFreeMessage(&uaa);
	return result;
done:	
	LOG(L_DBG,"DBG:"M_NAME":I_UAR: ... Done\n");
	return result;	
}

/**
 * Process a UAA.
 * Called from the Cx_UAA handler when the UAA is received
 * @param msg - the original SIP message retrieved from the transaction
 * @param uaa - the UAA diameter answer received from the HSS
 */
int I_UAA(struct sip_msg* msg, AAAMessage* uaa)
{
	int rc=-1,experimental_rc=-1;
	str server_name;
	int *m_capab=0,m_capab_cnt=0;
	int *o_capab=0,o_capab_cnt=0;
	scscf_entry *list=0;
	str call_id;
	
	if (!uaa){
		//TODO - add the warning code 99 in the reply	
		cscf_reply_transactional(msg,480,MSG_480_DIAMETER_TIMEOUT);		
		return CSCF_RETURN_BREAK;
	}
	
	if (!Cx_get_result_code(uaa,&rc)&&
		!Cx_get_experimental_result_code(uaa,&experimental_rc))
	{
		cscf_reply_transactional(msg,480,MSG_480_DIAMETER_MISSING_AVP);		
		return CSCF_RETURN_BREAK;			
	}
	
	switch(rc){
		case -1:
			switch(experimental_rc){
				case RC_IMS_DIAMETER_ERROR_USER_UNKNOWN:
					cscf_reply_transactional(msg,403,MSG_403_USER_UNKNOWN);		
					return CSCF_RETURN_BREAK;			
					break;
				case RC_IMS_DIAMETER_ERROR_IDENTITIES_DONT_MATCH:
					cscf_reply_transactional(msg,403,MSG_403_IDENTITIES_DONT_MATCH);		
					return CSCF_RETURN_BREAK;			
					break;
				case RC_IMS_DIAMETER_ERROR_ROAMING_NOT_ALLOWED:
					cscf_reply_transactional(msg,403,MSG_403_ROAMING_NOT_ALLOWED);		
					return CSCF_RETURN_BREAK;			
					break;
				case RC_IMS_DIAMETER_ERROR_IDENTITY_NOT_REGISTERED:
					cscf_reply_transactional(msg,403,MSG_403_IDENTITY_NOT_REGISTERED);		
					return CSCF_RETURN_BREAK;			
					break;
				 
				case RC_IMS_DIAMETER_FIRST_REGISTRATION:
					goto success;
				case RC_IMS_DIAMETER_SUBSEQUENT_REGISTRATION:
					goto success;
					break;
				case RC_IMS_DIAMETER_SERVER_SELECTION:
					goto success;
					break;
				
				default:
					cscf_reply_transactional(msg,403,MSG_403_UNKOWN_EXPERIMENTAL_RC);		
					return CSCF_RETURN_BREAK;			
			}
			break;
		
		case AAA_AUTHORIZATION_REJECTED:
			cscf_reply_transactional(msg,403,MSG_403_AUTHORIZATION_REJECTED);		
			return CSCF_RETURN_BREAK;			
			break;
		case AAA_UNABLE_TO_COMPLY:
			cscf_reply_transactional(msg,403,MSG_403_UNABLE_TO_COMPLY);		
			return CSCF_RETURN_BREAK;			
			break;
				
		case AAA_SUCCESS:
			goto success;			
			break;
						
		default:
			cscf_reply_transactional(msg,403,MSG_403_UNKOWN_RC);		
			return CSCF_RETURN_BREAK;			
	}
	
success:
	server_name = Cx_get_server_name(uaa);
	Cx_get_capabilities(uaa,&m_capab,&m_capab_cnt,&o_capab,&o_capab_cnt);

	list = I_get_capab_ordered(server_name,m_capab,m_capab_cnt,o_capab,o_capab_cnt,0);
	if (m_capab) shm_free(m_capab);
	if (o_capab) shm_free(o_capab);

	if (!list) {
		cscf_reply_transactional(msg,600,MSG_600_EMPTY_LIST);	
		return CSCF_RETURN_BREAK;
	}
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len||!add_scscf_list(call_id,list)){
		cscf_reply_transactional(msg,500,MSG_500_ERROR_SAVING_LIST);	
		return CSCF_RETURN_BREAK;
	}
	//print_s_list(L_ERR);
	return CSCF_RETURN_TRUE;;
}




