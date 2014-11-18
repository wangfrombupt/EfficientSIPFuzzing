/**
 * $Id: location.c 423 2007-07-29 13:21:41Z vingarzan $
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
 * Interrogating-CSCF - Location Information Operations LIR/LIA
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include <values.h>

#include "location.h"

#include "../../mem/shm_mem.h"

#include "mod.h"
#include "db.h"
#include "sip.h"
#include "cx.h"
#include "cx_avp.h"
#include "registration.h"

#include "../../action.h" /* run_actions */
#include "../../route.h" /* route_get */

#include "../../parser/parse_uri.h"

extern struct tm_binds tmb;				/**< Structure with pointers to tm funcs 		*/
extern struct cdp_binds cdpb;           /**< Structure with pointers to cdp funcs 		*/

extern str icscf_default_realm_str;		/**< fixed default realm */

extern int route_on_term_user_unknown_n;/**< script route number for Initial request processing after HSS says User Unknown */	

/**
 * Perform Location-Information-Request.
 * creates and send the user location query
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns true if OK, false if not
 */
int I_LIR(struct sip_msg* msg, char* str1, char* str2)
{
	int result=CSCF_RETURN_FALSE;
	str public_identity={0,0};
	str realm={0,0};
	AAAMessage *lia=0;	
	int orig = 0;

	
	LOG(L_DBG,"DBG:"M_NAME":I_LIR: Starting ...\n");
	/* check if we received what we should */
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_ERR,"ERR:"M_NAME":I_LIR: The message is not a request\n");
		goto done;
	}
	
	/* check orig uri parameter in topmost Route */
	if (I_originating(msg, str1, str2)==CSCF_RETURN_TRUE) {
		orig = 1;
		LOG(L_DBG,"DBG:"M_NAME":I_LIR: orig\n");
	}
	
	/* extract data from message */	
	if (orig) {
		public_identity = cscf_get_asserted_identity(msg);
		realm = cscf_get_realm_from_uri(public_identity);
	} else {
		realm = cscf_get_realm_from_ruri(msg);
		public_identity=cscf_get_public_identity_from_requri(msg);
	}
	if (!public_identity.len) {
		LOG(L_ERR,"ERR:"M_NAME":I_LIR: Public Identity not found, responding with 400\n");
		cscf_reply_transactional(msg,400,MSG_400_NO_PUBLIC);
		result=CSCF_RETURN_BREAK;
		goto done;
	}
	if(!realm.len) realm = icscf_default_realm_str;

	lia = Cx_LIR(msg,public_identity,realm);
	
	if (public_identity.s && !orig) 
		shm_free(public_identity.s); // shm_malloc in cscf_get_public_identity_from_requri		
	
	
	if (!lia){
		LOG(L_ERR,"ERR:"M_NAME":I_LIR: Error creating/sending LIR\n");
		cscf_reply_transactional(msg,480,MSG_480_DIAMETER_ERROR);
		result=CSCF_RETURN_BREAK;
		goto done;		
	}

    result = I_LIA(msg, &lia, orig);

done:
	if (lia) cdpb.AAAFreeMessage(&lia);	
	LOG(L_DBG,"DBG:"M_NAME":I_LIR: ... Done\n");
	return result;	
}

/**
 * Process a Location-Information-Answer.
 * Called from the Cx_LIA handler when the LIA is received
 * @param msg - the original SIP message retrieved from the transaction
 * @param lia - the LIA diameter answer received from the HSS
 * @returns #CSCF_RETURN_TRUE on success or #CSCF_RETURN_FALSE on error
 */
int I_LIA(struct sip_msg* msg, AAAMessage** lia, int originating)
{
	int rc=-1,experimental_rc=-1;
	str server_name;
	int *m_capab=0,m_capab_cnt=0;
	int *o_capab=0,o_capab_cnt=0;
	scscf_entry *list=0;
	str call_id;
	int status_code=999;
	char *reason_phrase=0;	
	
	if (!*lia){
		//TODO - add the warning code 99 in the reply	
		cscf_reply_transactional(msg,480,MSG_480_DIAMETER_TIMEOUT_LIA);		
		return CSCF_RETURN_BREAK;
	}
	
	if (!Cx_get_result_code(*lia,&rc)&&
		!Cx_get_experimental_result_code(*lia,&experimental_rc))
	{		
		status_code = 480;
		reason_phrase = MSG_480_DIAMETER_MISSING_AVP_LIA;		
		goto reply_final_response;			
	}
	
	switch(rc){
		case -1:
			switch(experimental_rc){
				
				case RC_IMS_DIAMETER_ERROR_USER_UNKNOWN:
					if (!originating && route_on_term_user_unknown_n > -1) {
						if (*lia) cdpb.AAAFreeMessage(lia);
						if (run_actions(main_rt.rlist[route_on_term_user_unknown_n], msg)<0){
							LOG(L_WARN,"ERR:"M_NAME":I_LIA: error while trying script\n");
						}
						return CSCF_RETURN_BREAK;
					} else {
						status_code = 604;
						reason_phrase = MSG_604_USER_UNKNOWN;		
						goto reply_final_response;								
					}
					break;

				case RC_IMS_DIAMETER_ERROR_IDENTITY_NOT_REGISTERED:
					status_code = 480;
					reason_phrase = MSG_480_NOT_REGISTERED;		
					goto reply_final_response;								
					
				case RC_IMS_DIAMETER_UNREGISTERED_SERVICE:
					goto success;	
				
				default:
					status_code = 403;
					reason_phrase = MSG_403_UNKOWN_EXPERIMENTAL_RC;		
					goto reply_final_response;								
			}
			break;
		
		case AAA_UNABLE_TO_COMPLY:
			status_code = 403;
			reason_phrase = MSG_403_UNABLE_TO_COMPLY;		
			goto reply_final_response;								
				
		case AAA_SUCCESS:
			goto success;			
						
		default:
			status_code = 403;
			reason_phrase = MSG_403_UNKOWN_RC;		
			goto reply_final_response;								
	}
	
success:
	server_name = Cx_get_server_name(*lia);
	if (server_name.len){
		list = new_scscf_entry(server_name,MAXINT, originating);
	}else{
		Cx_get_capabilities(*lia,&m_capab,&m_capab_cnt,&o_capab,&o_capab_cnt);
		list = I_get_capab_ordered(server_name,m_capab,m_capab_cnt,o_capab,o_capab_cnt, originating);
		if (m_capab) shm_free(m_capab);
		if (o_capab) shm_free(o_capab);
	}

	if (!list) {
		status_code = 600;
		reason_phrase = MSG_600_EMPTY_LIST;		
		goto reply_final_response;				
	}
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len||!add_scscf_list(call_id,list)){
		status_code = 500;
		reason_phrase = MSG_500_ERROR_SAVING_LIST;		
		goto reply_final_response;				
	}
	//print_s_list(L_ERR);	
		
	return CSCF_RETURN_TRUE;

reply_final_response:	
	if (*lia) cdpb.AAAFreeMessage(lia);
	cscf_reply_transactional(msg,status_code,reason_phrase);		
	return CSCF_RETURN_BREAK;	
}


/**
 * Finds if the message contains the orig parameter in the first Route header
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if yes, else #CSCF_RETURN_FALSE
 */
int I_originating(struct sip_msg *msg,char *str1,char *str2)
{
	//int ret=CSCF_RETURN_FALSE;
	struct hdr_field *h;
	str* uri;
	rr_t *r;
	
	if (parse_headers(msg, HDR_ROUTE_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":I_originating: error parsing headers\n");
		return CSCF_RETURN_FALSE;
	}
	h = msg->route;
	if (!h){
		LOG(L_DBG,"DBG:"M_NAME":I_originating: Header Route not found\n");
		return CSCF_RETURN_FALSE;
	}
	if (parse_rr(h)<0){
		LOG(L_ERR,"ERR:"M_NAME":I_originating: Error parsing as Route header\n");
		return CSCF_RETURN_FALSE;
	}
	r = (rr_t*)h->parsed;
	
	uri = &r->nameaddr.uri;
	struct sip_uri puri;
	if (parse_uri(uri->s, uri->len, &puri) < 0) {
		LOG(L_ERR, "DBG:"M_NAME":I_originating: Error while parsing the first route URI\n");
		return -1;
	}
	if (puri.params.len < 4) return CSCF_RETURN_FALSE;
	//LOG(L_DBG,"DBG:"M_NAME":I_originating: uri params <%.*s>\n", puri.params.len, puri.params.s);
	// parse_uri does not support orig param...
	int c = 0;
	int state = 0; 
	while (c < puri.params.len) {
		switch (puri.params.s[c]) {
			case 'o': if (state==0) state=1;
					break;
			case 'r': if (state==1) state=2;
					break;
			case 'i': if (state==2) state=3;
					break;
			case 'g': if (state==3) state=4;
					break;
			case ' ': 
			case '\t':
			case '\r':
			case '\n':
			case ',':
			case ';':
					  if (state==4) return CSCF_RETURN_TRUE;
					  state=0;
					  break;
			case '=': if (state==4) return CSCF_RETURN_TRUE;
					  state=-1;
					  break;
			default: state=-1;
		}
		c++;
	}
	
	return state==4 ? CSCF_RETURN_TRUE : CSCF_RETURN_FALSE;
}	



