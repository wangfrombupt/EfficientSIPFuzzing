/*
 * $Id: registrar.c 430 2007-08-01 13:18:42Z vingarzan $
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
 * Serving-CSCF - Registrar-Related Operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include <time.h>

#include "../../parser/contact/contact.h"
#include "../../parser/contact/parse_contact.h"
#include "../../ut.h"
#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"
#include "../../dset.h"

#include "registrar.h"
#include "registration.h"
#include "mod.h"
#include "registrar_parser.h"
#include "registrar_storage.h"
#include "sip.h"
#include "cx.h"
#include "cx_avp.h"
#include "sip_messages.h"
#include "dlg_state.h"


extern struct tm_binds tmb;            	/**< Structure with pointers to tm funcs 		*/
extern struct cdp_binds cdpb;           /**< Structure with pointers to cdp funcs 		*/

extern int r_hash_size;					/**< Size of S-CSCF registrar hash table		*/
extern r_hash_slot *registrar;		/**< The S-CSCF registrar 						*/

extern str scscf_name_str;				/**< fixed name of the S-CSCF 					*/
extern str scscf_service_route_uri;		/**< just the service route uri 				*/
extern str scscf_registration_min_expires;/**< fixed minimum registration expiration time*/
extern int server_assignment_store_data;/**< whether to ask to keep the data in SAR 	*/


extern int registration_default_expires;/**< the default value for expires if none found*/
extern int registration_min_expires;	/**< minimum registration expiration time 		*/
extern int registration_max_expires;	/**< maximum registration expiration time 		*/
extern int registration_disable_early_ims;	/**< if to disable the Early-IMS checks			*/

extern time_t time_now;					/**< Current time of the S-CSCF registrar 		*/

extern int append_branches;				/**< if to append branches						*/


#ifdef WITH_IMS_PM
        static str zero={0,0};
#endif


/**
 * The Registrar timer looks for expires contacts and removes them
 * @param ticks - the current time
 * @param param - pointer to the domain_list
 */
void registrar_timer(unsigned int ticks, void* param)
{
	r_public *p,*pn,*rpublic;
	r_contact *c,*c2=0,*cn,*cn2=0;
	r_subscriber *s,*sn,*s2=0,*sn2=0;
	int i,j,n,assignment_type,sar_res;
	ims_public_identity *pi=0;
	r_hash_slot *r;
	int rpublic_hash;
	
	#ifdef WITH_IMS_PM
		int impu_cnt=0,contact_cnt=0,subs_cnt=0;
	#endif


	r = param;
	
	LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Called at %d\n",ticks);

	r_act_time();

	for(i=0;i<r_hash_size;i++){
		r_lock(i);
			p = r[i].head;
			while(p){
				pn = p->next;
				c = p->head;
									
				while(c){
					cn = c->next;
					if (!r_valid_contact(c)){
						LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> expired and removed.\n",
							c->uri.len,c->uri.s);							
						S_event_reg(p,c,0,IMS_REGISTRAR_CONTACT_EXPIRED,1);/* send now because we might drop the dialog soon */	
						del_r_contact(p,c);
					}
					#ifdef WITH_IMS_PM
						else contact_cnt++;
					#endif
					c = cn;
				}
				s = p->shead;
				while(s){
					sn = s->next;
					if (!r_valid_subscriber(s)){
						LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Subscriber <%.*s> expired and removed.\n",
							s->subscriber.len,s->subscriber.s);
						del_r_subscriber(p,s);
					}
					#ifdef WITH_IMS_PM
						else subs_cnt++;
					#endif
					s = sn;
				}
				
				if (!p->head){/* no more contacts, then deregister it */
					switch (p->reg_state){
						case REGISTERED:
							if (server_assignment_store_data) 
								assignment_type = AVP_IMS_SAR_TIMEOUT_DEREGISTRATION_STORE_SERVER_NAME;
							else assignment_type = AVP_IMS_SAR_TIMEOUT_DEREGISTRATION;
							sar_res = SAR(0,cscf_get_realm_from_uri(p->aor),p->aor,p->s->private_identity,assignment_type,0);
							
							if (sar_res==1){
								/* SAR successful, de-register everything in this implicit set */
								if (!p->s){
									LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Problem r_public does not contain IMS Subscription <%.*s> \n",
										p->aor.len,p->aor.s);
									break;
								}
								for(j=0;j<p->s->service_profiles_cnt;j++)
									for(n=0;n<p->s->service_profiles[j].public_identities_cnt;n++){
										pi = &(p->s->service_profiles[j].public_identities[n]);
										if (pi->public_identity.len == p->aor.len &&
											strncasecmp(pi->public_identity.s,p->aor.s,p->aor.len)==0) continue;
										rpublic = get_r_public_previous_lock(pi->public_identity,p->hash);
										if(!rpublic){
											LOG(L_INFO,"INFO:"M_NAME":registrar_timer: The implicit set public identity <%.*s> was not found in the registrar",
												pi->public_identity.len,pi->public_identity.s);
											continue;
										}
										rpublic_hash = rpublic->hash;
										
										c2 = rpublic->head;
										while(c2){
											cn2 = c2->next;
											if (!r_valid_contact(c2)){
												LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> expired and removed (because of implicit set).\n",c2->uri.len,c2->uri.s);
												S_event_reg(rpublic,c2,0,IMS_REGISTRAR_CONTACT_EXPIRED,1);/* send now because we might drop the dialog soon */	
												del_r_contact(rpublic,c2);
											}
											c2 = cn2;
										}
										
										s2 = rpublic->shead;
										while(s2){
											sn2 = s2->next;
											if (!r_valid_subscriber(s2)){
												LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Subscriber <%.*s> expired and removed (because of implicit set).\n",s2->subscriber.len,s2->subscriber.s);
												del_r_subscriber(rpublic,s2);
											}										  
											s2 = sn2;
										}
								
										if (!rpublic->head){/* no more contacts, then deregister it */
											if (!rpublic->shead)  {/* delete it if there are no more subscribers for it */     
												LOG(L_DBG,"DBG:"M_NAME":registrar_timer: User <%.*s> removed (because of implicit set).\n",
													rpublic->aor.len,rpublic->aor.s);
												del_r_public(rpublic);
											}else{/* else mark it unregistered - to avoid more SAR */
												LOG(L_DBG,"DBG:"M_NAME":registrar_timer: User <%.*s> kept unregistered - has subscribers (because of implicit set).\n",
											        rpublic->aor.len,rpublic->aor.s);                                                
												rpublic->reg_state = NOT_REGISTERED;                                       
											}
										}
										/* because the lock was taken with a previous lock */
										if (rpublic_hash!=i) r_unlock(rpublic_hash);
									}

							  LOG(L_DBG,"DBG:"M_NAME":registrar_timer: User <%.*s> deregistered.\n",
									p->aor.len,p->aor.s);
							  if (!p->shead)	{/* delete it if there are no more subscribers for it */						
								LOG(L_DBG,"DBG:"M_NAME":registrar_timer: User <%.*s> removed.\n",
									p->aor.len,p->aor.s);
								del_r_public(p);
							  }else{/* else mark it unregistered - to avoid more SAR */
								LOG(L_DBG,"DBG:"M_NAME":registrar_timer: User <%.*s> kept unregistered - has subscribers.\n",
								p->aor.len,p->aor.s);								
								p->reg_state = NOT_REGISTERED;								
							  }
							}else{
								LOG(L_DBG,"DBG:"M_NAME":registrar_timer: User <%.*s> deregistration SAR failed.Keeping into registrar, but with no contacts\n",
									p->aor.len,p->aor.s);
								#ifdef WITH_IMS_PM
									impu_cnt++;
								#endif
							}										
							break;
							
						case UNREGISTERED:
							/* Don't drop it, just keep it for unregistered triggering*/
							 #ifdef WITH_IMS_PM
								impu_cnt++;
							#endif
							break;
							
						case NOT_REGISTERED:								
							/* to avoid sending SAR when we still have subscribers, but no contact */
							if (!p->shead)	{/* delete it if there are no more subscribers for it */						
								LOG(L_DBG,"DBG:"M_NAME":registrar_timer: User <%.*s> removed - no more subscribers.\n",
									p->aor.len,p->aor.s);
								del_r_public(p);							
							}
							break;
					}
				}
				#ifdef WITH_IMS_PM
					else impu_cnt++;
				#endif
				p = pn;
			}
		r_unlock(i);		
	}
	print_r(L_INFO);
	#ifdef WITH_IMS_PM
		IMS_PM_LOG01(RD_NbrIMPU,impu_cnt);
		IMS_PM_LOG01(RD_NbrContact,contact_cnt);
		IMS_PM_LOG01(RD_NbrSubs,subs_cnt);
	#endif
}

/**
 * Does the Server Assignment procedures, assigning this S-CSCF to the user.
 * Covered cases:
 * AVP_IMS_SAR_NO_ASSIGNMENT							= 0			
 * AVP_IMS_SAR_REGISTRATION								= 1,		YES,HERE
 * AVP_IMS_SAR_RE_REGISTRATION							= 2,
 * AVP_IMS_SAR_UNREGISTERED_USER						= 3,		in S_assign_server_unreg
 * AVP_IMS_SAR_TIMEOUT_DEREGISTRATION					= 4,
 * AVP_IMS_SAR_USER_DEREGISTRATION						= 5,		YES,HERE
 * AVP_IMS_SAR_TIMEOUT_DEREGISTRATION_STORE_SERVER_NAME = 6,
 * AVP_IMS_SAR_USER_DEREGISTRATION_STORE_SERVER_NAME	= 7,		YES,HERE
 * AVP_IMS_SAR_ADMINISTRATIVE_DEREGISTRATION			= 8,
 * AVP_IMS_SAR_AUTHENTICATION_FAILURE					= 9,
 * AVP_IMS_SAR_AUTHENTICATION_TIMEOUT					= 10,
 * AVP_IMS_SAR_DEREGISTRATION_TOO_MUCH_DATA
 * 
 * @param msg - the SIP REGISTER message (that is authorized)
 * @param str1 - the realm to look for in Authorization
 * @param str2 - not used
 * @returns true if ok, false if not, break on error
 */
int S_assign_server(struct sip_msg *msg,char *str1,char *str2 )
{
	int ret=CSCF_RETURN_FALSE;
	str private_identity,public_identity,realm;
	int assignment_type = AVP_IMS_SAR_NO_ASSIGNMENT;
	int data_available = AVP_IMS_SAR_USER_DATA_NOT_AVAILABLE;
	int expires;

	LOG(L_DBG,"DBG:"M_NAME":S_assign_server: Assigning server...\n");
	
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_assign_server: This message is not a request\n");
		goto error;
	}		

	realm.s = str1; realm.len = strlen(str1);
	if (!realm.len) {
		LOG(L_ERR,"ERR:"M_NAME":S_assign_server: No realm found\n");
		return CSCF_RETURN_BREAK;
	}
		
	private_identity = cscf_get_private_identity(msg,realm);
	
	public_identity = cscf_get_public_identity(msg);
	if (!public_identity.len) {
		LOG(L_ERR,"ERR:"M_NAME":S_assign_server: public identity missing\n");		
		return ret;
	}
		
	expires = cscf_get_max_expires(msg);
	
	if (expires>0) {
		if (r_is_registered_id(public_identity)) 
			assignment_type = AVP_IMS_SAR_RE_REGISTRATION;
		else
			assignment_type = AVP_IMS_SAR_REGISTRATION;
	}
	else {
		if (server_assignment_store_data) 
			assignment_type = AVP_IMS_SAR_USER_DEREGISTRATION_STORE_SERVER_NAME;
		else assignment_type = AVP_IMS_SAR_USER_DEREGISTRATION;
	}
	
	//TODO
	//if (userdataavailable) data_available = AVP_IMS_SAR_USER_DATA_ALREADY_AVAILABLE;
	//else
	data_available = AVP_IMS_SAR_USER_DATA_NOT_AVAILABLE;
	
	ret = SAR(msg,realm,public_identity,private_identity,assignment_type,data_available);
			
	return ret;	
error:
	ret = CSCF_RETURN_ERROR;		
	return ret;
}

/**
 * Does the Server Assignment procedures, assigning this S-CSCF to the user without previous registration.
 * Covered cases:
 * AVP_IMS_SAR_NO_ASSIGNMENT							= 0			
 * AVP_IMS_SAR_REGISTRATION								= 1,		YES, NOT HERE
 * AVP_IMS_SAR_RE_REGISTRATION							= 2,
 * AVP_IMS_SAR_UNREGISTERED_USER						= 3,		YES, HERE
 * AVP_IMS_SAR_TIMEOUT_DEREGISTRATION					= 4,
 * AVP_IMS_SAR_USER_DEREGISTRATION						= 5,		YES, NOT HERE
 * AVP_IMS_SAR_TIMEOUT_DEREGISTRATION_STORE_SERVER_NAME = 6,
 * AVP_IMS_SAR_USER_DEREGISTRATION_STORE_SERVER_NAME	= 7,		YES, NOT HERE
 * AVP_IMS_SAR_ADMINISTRATIVE_DEREGISTRATION			= 8,
 * AVP_IMS_SAR_AUTHENTICATION_FAILURE					= 9,
 * AVP_IMS_SAR_AUTHENTICATION_TIMEOUT					= 10,
 * AVP_IMS_SAR_DEREGISTRATION_TOO_MUCH_DATA
 * 
 * @param msg - the SIP REGISTER message (that is authorized)
 * @param str1 - the realm to look for in Authorization
 * @param str2 - direction - "orig" or "term"
 * @returns true if ok, false if not, break on error
 */ 
int S_assign_server_unreg(struct sip_msg *msg,char *str1,char *str2 )
{
	int ret=CSCF_RETURN_FALSE;
	str private_identity={0,0},public_identity={0,0},realm={0,0};
	int assignment_type = AVP_IMS_SAR_NO_ASSIGNMENT;
	int data_available = AVP_IMS_SAR_USER_DATA_NOT_AVAILABLE;

	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_assign_server_unreg: This message is not a request\n");
		goto error;
	}		

	realm.s = str1; realm.len = strlen(str1);
	if (!realm.len) {
		LOG(L_ERR,"ERR:"M_NAME":S_assign_server_unreg: No realm found\n");
		return CSCF_RETURN_BREAK;
	}

	enum s_dialog_direction dir = get_dialog_direction(str2);
	
	switch (dir){
		case DLG_MOBILE_ORIGINATING:
			public_identity = cscf_get_asserted_identity(msg);
			if (!public_identity.len) {
				LOG(L_DBG,"DBG:"M_NAME":S_assign_server_unreg(orig): public identity missing\n");		
				return ret;
			}
			break;
		case DLG_MOBILE_TERMINATING:
			public_identity = cscf_get_public_identity_from_requri(msg);
			if (!public_identity.len) {
				LOG(L_DBG,"DBG:"M_NAME":S_assign_server_unreg(term): public identity missing\n");		
				return ret;
			}
			break;
		default:
			LOG(L_ERR,"ERR:"M_NAME":S_assign_server_unreg: bad dialog direction parameter\n");
			goto error;
	}
	
	assignment_type = AVP_IMS_SAR_UNREGISTERED_USER;
	
	//TODO
	//if (userdataavailable) data_available = AVP_IMS_SAR_USER_DATA_ALREADY_AVAILABLE;
	//else
	data_available = AVP_IMS_SAR_USER_DATA_NOT_AVAILABLE;
	
	ret = SAR(msg,realm,public_identity,private_identity,assignment_type,data_available);
	
	return ret;	
error:
	ret = CSCF_RETURN_ERROR;		
	return ret;
}


/**
 * Sends a SAR.
 * Can respond with a SIP reply if msg!=0
 * @param msg - the SIP message
 * @param realm - the realm
 * @param public_identity - public identity
 * @param private_identity - private identity
 * @param assignment_type - assignment type
 * @param data_available - if the data is already available
 * @returns CSCF_RETURN_TRUE if ok, CSCF_RETURN_FALSE on error or CSCF_RETURN_BREAK on response sent out
 */
int SAR(struct sip_msg *msg, str realm, str public_identity, str private_identity,
				int assignment_type,int data_available)
{
	AAAMessage *saa;
	int rc=-1,experimental_rc=-1;
	str xml={0,0},ccf1={0,0},ccf2={0,0},ecf1={0,0},ecf2={0,0};
		
	if (realm.len==0){
		realm = cscf_get_realm_from_uri(private_identity);
	}		
	saa = Cx_SAR(msg,public_identity,private_identity,scscf_name_str,realm,
		assignment_type,data_available);
	
	if (!saa){
		//TODO - add the warning code 99 in the reply	
		if (msg) S_REGISTER_reply(msg,480,MSG_480_DIAMETER_TIMEOUT_SAR);		
		goto error;
	}
	
	if (!Cx_get_result_code(saa,&rc)&&
		!Cx_get_experimental_result_code(saa,&experimental_rc))
	{
		if (msg) S_REGISTER_reply(msg,480,MSG_480_DIAMETER_MISSING_AVP);
		goto done;
	}
	
	switch(rc){
		case -1:
			switch(experimental_rc){
				case RC_IMS_DIAMETER_ERROR_USER_UNKNOWN:
					if (msg) S_REGISTER_reply(msg,403,MSG_403_USER_UNKNOWN);		
					break;
				case RC_IMS_DIAMETER_ERROR_IDENTITIES_DONT_MATCH:
					if (msg) S_REGISTER_reply(msg,403,MSG_403_IDENTITIES_DONT_MATCH);		
					break;
				case RC_IMS_DIAMETER_ERROR_AUTH_SCHEME_NOT_SUPPORTED:
					if (msg) S_REGISTER_reply(msg,403,MSG_403_AUTH_SCHEME_UNSOPPORTED);		
					break;
				
				default:
					if (msg) S_REGISTER_reply(msg,403,MSG_403_UNKOWN_EXPERIMENTAL_RC);		
			}
			break;
		
		case AAA_UNABLE_TO_COMPLY:
			if (msg) S_REGISTER_reply(msg,403,MSG_403_UNABLE_TO_COMPLY);		
			break;
				
		case AAA_SUCCESS:
			goto success;			
			break;
						
		default:
			if (msg) S_REGISTER_reply(msg,403,MSG_403_UNKOWN_RC);		
	}
	
goto done;		
	
success:
	xml = Cx_get_user_data(saa);
	
	if (assignment_type==AVP_IMS_SAR_TIMEOUT_DEREGISTRATION ||
		assignment_type==AVP_IMS_SAR_USER_DEREGISTRATION ||
		assignment_type==AVP_IMS_SAR_TIMEOUT_DEREGISTRATION_STORE_SERVER_NAME ||
		assignment_type==AVP_IMS_SAR_USER_DEREGISTRATION_STORE_SERVER_NAME ||
		assignment_type==AVP_IMS_SAR_ADMINISTRATIVE_DEREGISTRATION
	   )
	{
		drop_auth_userdata(private_identity,public_identity);
	}
	
	Cx_get_charging_info(saa,&ccf1,&ccf2,&ecf1,&ecf2);
	if (msg){
		int ret = save_location(msg,assignment_type,&xml,&ccf1,&ccf2,&ecf1,&ecf2);
		if (saa) cdpb.AAAFreeMessage(&saa);
		return ret;
	}else{
		/* it was called internally and there is no SIP message to respond to */		
	}
	if (saa) cdpb.AAAFreeMessage(&saa);
	return CSCF_RETURN_TRUE;
done:	
	if (saa) cdpb.AAAFreeMessage(&saa);
	return CSCF_RETURN_FALSE;
error:	
	if (saa) cdpb.AAAFreeMessage(&saa);
	return CSCF_RETURN_BREAK;
}



/**
 * Calculates the expiration time for one contact.
 * Tries to use the Expiration header, if not present then use the 
 * expires parameter of the contact, if param not present it defaults
 * to the default value.
 * Also checks 
 * @param c - the contact to calcualte for
 * @param expires_hdr - value of expires hdr if present, if not -1
 * @returns the time of expiration
 */
static inline int r_calc_expires(contact_t *c,unsigned int expires_hdr)
{
	unsigned int r;
	if (expires_hdr>=0) r = expires_hdr;
	else r = registration_default_expires;
	if (c && c->expires)
		str2int(&(c->expires->body), (unsigned int*)&r); 
	if (r>0 && r<registration_min_expires) r = registration_min_expires;
	if (r>registration_max_expires) r = registration_max_expires;
	return time_now+r;
}

/**
 * Adds a Contact header to the reply, containing the approved expires
 * @param msg - the SIP message to add contact header to its reply
 * @param uri - the contact uri
 * @param expires - the expiration time
 * @returns 1 if ok, 0 if not
 */
static int r_add_contact(struct sip_msg *msg,str uri,int expires)
{
	str hdr;
	int r;
	hdr.s = pkg_malloc(10+uri.len+10+12+3+1);
	if (!hdr.s) return 0;
	sprintf(hdr.s,"Contact: <%.*s>;expires=%d\r\n",uri.len,uri.s,expires);
	hdr.len = strlen(hdr.s);
	r = cscf_add_header_rpl(msg,&hdr);
	pkg_free(hdr.s);
	return r;
}



/**
 * Updates the contacts in the registrar with the new values received,
 * for the addresses received in the SAA
 * @param msg - the SIP REGISTER message, if available
 * @param assignment_type - the value sent in SAR 
 * @param ct - SIP contact list from message
 * @param is_star - if the contact header was of type STAR
 * @param s - the ims_subscription received in the SAA
 * @param ua - the user agent string
 * @param path - the path to save
 * @returns CSCF_RETURN_TRUE if ok, CSCF_RETURN_FALSE on error or CSCF_RETURN_BREAK on response sent out
 */
static inline int update_contacts(struct sip_msg* msg, int assignment_type,
	 unsigned char is_star, ims_subscription **s, str* ua,str *path, str *ccf1,str *ccf2,str *ecf1,str *ecf2)
{
	int i,j,s_used=0;
	r_public *p,*rpublic;
	r_contact *c=0;
	ims_public_identity *pi=0;
	struct hdr_field *h;
	contact_t *ci;
	int reg_state,expires_hdr,expires,hash,rpublic_hash;
	str public_identity,sent_by={0,0};
	
//	if (!*s) return 1;
	
	/* check for Early-IMS case */
	if (!registration_disable_early_ims && !msg->authorization){
		str received={0,0};		
		sent_by = cscf_get_last_via_sent_by(msg);
		if (sent_by.len){
			received = cscf_get_last_via_received(msg);
			if (received.len) sent_by=received;				
		}
	}
	expires_hdr = cscf_get_expires_hdr(msg);
	r_act_time();
	LOG(L_DBG,"DBG:"M_NAME":update_contacts: Assign Type %d\n",assignment_type);
	switch (assignment_type){
		case AVP_IMS_SAR_REGISTRATION:
			reg_state = IMS_USER_REGISTERED;
			if (!*s) break;
			for(i=0;i<(*s)->service_profiles_cnt;i++)
				for(j=0;j<(*s)->service_profiles[i].public_identities_cnt;j++){
					pi = &((*s)->service_profiles[i].public_identities[j]);
					if (!pi->barring){
						if (!(p=update_r_public(pi->public_identity,&reg_state,s,ccf1,ccf2,ecf1,ecf2))){
							LOG(L_ERR,"ERR:"M_NAME":update_contacts: error on <%.*s>\n",
								pi->public_identity.len,pi->public_identity.s);
							goto error;
						}
						s_used++;
						if (!registration_disable_early_ims && sent_by.len) {
							if (p->early_ims_ip.s) shm_free(p->early_ims_ip.s);
							STR_SHM_DUP(p->early_ims_ip,sent_by,"IP Early IMS");
						}
						if (is_star){
							LOG(L_ERR,"ERR:"M_NAME":update_contacts: STAR not accepted in contact for Registration.\n");
						}else{
							for(h=msg->contact;h;h=h->next)
								if (h->type==HDR_CONTACT_T && h->parsed)
								 for(ci=((contact_body_t*)h->parsed)->contacts;ci;ci=ci->next){
									expires = r_calc_expires(ci,expires_hdr);
									if (!(c=update_r_contact(p,ci->uri,&expires,ua,path))){
										LOG(L_ERR,"ERR:"M_NAME":update_contacts: error on <%.*s>\n",
											ci->uri.len,ci->uri.s);
										goto error;
									}
									if (assignment_type == AVP_IMS_SAR_REGISTRATION)
										S_event_reg(p,c,0,IMS_REGISTRAR_CONTACT_REGISTERED,0);
									else 
										S_event_reg(p,c,0,IMS_REGISTRAR_CONTACT_REFRESHED,0);
									
								}
						}
						r_unlock(p->hash);
					}
				}
			if(c) r_add_contact(msg,c->uri,c->expires-time_now);
			
			break;
		case AVP_IMS_SAR_RE_REGISTRATION:
			reg_state = IMS_USER_REGISTERED;
			public_identity = cscf_get_public_identity(msg);
            if (!public_identity.len) {
                    LOG(L_ERR,"ERR:"M_NAME":update_contacts: message contains no public identity\n");
                    goto error;
            }

            p = get_r_public(public_identity);
			if (!p){
				LOG(L_ERR,"ERR:"M_NAME":update_contacts: error on <%.*s> - not found in registrar\n",
					public_identity.len,public_identity.s);
				goto error;
			}

			if (!registration_disable_early_ims && sent_by.len) {
				if (p->early_ims_ip.s) shm_free(p->early_ims_ip.s);
				STR_SHM_DUP(p->early_ims_ip,sent_by,"IP Early IMS");
			}
            if (is_star){
                LOG(L_ERR,"ERR:"M_NAME":update_contacts: STAR not accepted in contact for Re-Registration.\n");
            }else{
                for(h=msg->contact;h;h=h->next)
                    if (h->type==HDR_CONTACT_T && h->parsed)
                     for(ci=((contact_body_t*)h->parsed)->contacts;ci;ci=ci->next){
                        expires = r_calc_expires(ci,expires_hdr);
                        if (!(c=update_r_contact(p,ci->uri,&expires,ua,path))){
                            LOG(L_ERR,"ERR:"M_NAME":update_contacts: error on <%.*s>\n",
                                ci->uri.len,ci->uri.s);
                            goto error;
                        }
                        if (assignment_type == AVP_IMS_SAR_REGISTRATION)
                            S_event_reg(p,c,0,IMS_REGISTRAR_CONTACT_REGISTERED,0);
                        else
                            S_event_reg(p,c,0,IMS_REGISTRAR_CONTACT_REFRESHED,0);
                        r_add_contact(msg,c->uri,c->expires-time_now);
                    }
            }            

			/* now update the implicit set */
			if (p->s)
            for(i=0;i<p->s->service_profiles_cnt;i++)
				for(j=0;j<p->s->service_profiles[i].public_identities_cnt;j++){
					pi = &(p->s->service_profiles[i].public_identities[j]);
					if (public_identity.len==pi->public_identity.len &&
						strncasecmp(pi->public_identity.s,public_identity.s,public_identity.len)==0) continue;

					rpublic = update_r_public_previous_lock(pi->public_identity,p->hash,&reg_state,s,ccf1,ccf2,ecf1,ecf2);
					if (!rpublic){
						LOG(L_ERR,"ERR:"M_NAME":update_contacts: error on <%.*s>\n",
							pi->public_identity.len,pi->public_identity.s);
						continue;
					}

		 			if (!registration_disable_early_ims && sent_by.len) {
						if (rpublic->early_ims_ip.s) shm_free(rpublic->early_ims_ip.s);
							STR_SHM_DUP(rpublic->early_ims_ip,sent_by,"IP Early IMS");
                    }
    		        if (is_star){
                    		LOG(L_ERR,"ERR:"M_NAME":update_contacts: STAR not accepted in contact for Re-Registration.\n");
                    }else{
		                for(h=msg->contact;h;h=h->next)
    		                if (h->type==HDR_CONTACT_T && h->parsed)
                		        for(ci=((contact_body_t*)h->parsed)->contacts;ci;ci=ci->next){
                            		expires = r_calc_expires(ci,expires_hdr);
                                    if (!(c=update_r_contact(rpublic,ci->uri,&expires,ua,path))){
                                        LOG(L_ERR,"ERR:"M_NAME":update_contacts: error on <%.*s> - implicit identity not found in registrar\n",
                                            ci->uri.len,ci->uri.s);
		                                goto error;
            		                }
                    		        if (assignment_type == AVP_IMS_SAR_REGISTRATION)
                        		        S_event_reg(rpublic,c,0,IMS_REGISTRAR_CONTACT_REGISTERED,0);
                                    else
                                        S_event_reg(rpublic,c,0,IMS_REGISTRAR_CONTACT_REFRESHED,0);
        		                }
           			}
                    if (rpublic->hash!=p->hash) r_unlock(rpublic->hash);                            
			}
			r_unlock(p->hash);
			break;		
			
		case AVP_IMS_SAR_USER_DEREGISTRATION:
			public_identity = cscf_get_public_identity(msg);
			if (!public_identity.len) {
				LOG(L_ERR,"ERR:"M_NAME":update_contacts: message contains no public identity\n");
				goto error;
			}
			p = get_r_public(public_identity);

			if (!p){
				LOG(L_ERR,"ERR:"M_NAME":update_contacts: error on <%.*s> - not found in registrar\n",
					public_identity.len,public_identity.s);
				goto error;
			}

			//deregister public identity that is on the message
			if (is_star){
				c = p->head;
				while(c){
					c->expires = time_now;
					S_event_reg(p,c,0,IMS_REGISTRAR_CONTACT_UNREGISTERED,0);
					r_add_contact(msg,c->uri,0);
					del_r_contact(p,c);
					c = c->next;
				}
			}else{
				for(h=msg->contact;h;h=h->next)
					if (h->type==HDR_CONTACT_T && h->parsed)
						for(ci=((contact_body_t*)h->parsed)->contacts;ci;ci=ci->next){
							c = get_r_contact(p,ci->uri);
							if (c) {
								c->expires = time_now;
								S_event_reg(p,c,0,IMS_REGISTRAR_CONTACT_UNREGISTERED,0);
								r_add_contact(msg,c->uri,0);
								del_r_contact(p,c);
							}
						}
			}

			/* now update the implicit set */
			if (p->s)
            for(i=0;i<p->s->service_profiles_cnt;i++)
                for(j=0;j<p->s->service_profiles[i].public_identities_cnt;j++){
                    pi = &(p->s->service_profiles[i].public_identities[j]);
					if (public_identity.len == pi->public_identity.len && 
						strncasecmp(pi->public_identity.s,public_identity.s,public_identity.len)==0)	continue;
						
					rpublic = update_r_public_previous_lock(pi->public_identity,p->hash,&reg_state,s,ccf1,ccf2,ecf1,ecf2);				
                    if (!rpublic){
                        LOG(L_ERR,"ERR:"M_NAME":update_contacts: error on <%.*s> - implicit identity not found in registrar\n",
                            pi->public_identity.len,pi->public_identity.s);
	                    continue;
                    }
                    s_used++;
					if (is_star){
			            c = rpublic->head;
				        while(c){
		                    c->expires = time_now;
		                    S_event_reg(rpublic,c,0,IMS_REGISTRAR_CONTACT_UNREGISTERED,0);                                     	                          			
		                    del_r_contact(rpublic,c);	
		                    c = c->next;
			            }
					}else{
						for(h=msg->contact;h;h=h->next)
	                    	if (h->type==HDR_CONTACT_T && h->parsed)
								for(ci=((contact_body_t*)h->parsed)->contacts;ci;ci=ci->next){
									c = get_r_contact(rpublic,ci->uri);
									if (c) {
										c->expires = time_now;
										S_event_reg(rpublic,c,0,IMS_REGISTRAR_CONTACT_UNREGISTERED,0);                     
										del_r_contact(rpublic,c);
									}
							}
					}
					rpublic_hash = rpublic->hash;
                    if (!rpublic->head) 
						del_r_public(rpublic);
                                                
					if (p->hash!=rpublic_hash) r_unlock(rpublic_hash);
				}

			hash = p->hash;
			if (!p->head) 
				del_r_public(p);
			r_unlock(hash);			
			break;
						
		case AVP_IMS_SAR_UNREGISTERED_USER:			
			reg_state = IMS_USER_UNREGISTERED;
			r_act_time();
			if (!*s) break;
			for(i=0;i<(*s)->service_profiles_cnt;i++)
				for(j=0;j<(*s)->service_profiles[i].public_identities_cnt;j++){
					pi = &((*s)->service_profiles[i].public_identities[j]);
					if (!pi->barring){
						if (!(p=update_r_public(pi->public_identity,&reg_state,s,ccf1,ccf2,ecf1,ecf2))){
							LOG(L_ERR,"ERR:"M_NAME":update_contacts: error on <%.*s>\n",
								pi->public_identity.len,pi->public_identity.s);
							goto error;
						}
						s_used++;
						r_unlock(p->hash);
					}
				}
			break;			
			
		default:
			LOG(L_ERR,"ERR:"M_NAME":update_contacts: unimplemented case for assign. %d\n",
				assignment_type);
			return 0;
	}

	//print_r(L_CRIT);
	return CSCF_RETURN_TRUE;
error:
out_of_memory:
	if (!s_used&&*s) free_user_data(*s);
	return CSCF_RETURN_FALSE;	
}


str hdr_p_associated_uri1={"P-Associated-URI: <",19};
str hdr_p_associated_uri2={">, <",4};
str hdr_p_associated_uri3={">\r\n",3};

/**
 * Adds to the response the P-Associated-URI header.
 * This adds all the public identities for the user in the header
 * @param msg - the SIP message
 * @param s - the ims_subscription to add the header from
 */
int insert_p_associated_uri(struct sip_msg *msg,ims_subscription *s)
{
	str public_identity;
	str hdr={0,0};
	ims_public_identity *id;
	int i,j;
	
	public_identity = cscf_get_public_identity(msg);
	if (!public_identity.len){
		LOG(L_ERR,"ERR:"M_NAME":insert_p_associated_uri: error getting public id\n");
		goto error;
	}
	hdr.len=hdr_p_associated_uri1.len+public_identity.len+hdr_p_associated_uri3.len;
	if (s){
		for(i=0;i<s->service_profiles_cnt;i++)
		 for(j=0;j<s->service_profiles[i].public_identities_cnt;j++){
		 	id = &(s->service_profiles[i].public_identities[j]);
		 	if (!id->barring&&
		 		(id->public_identity.len != public_identity.len ||
		 		 memcmp(id->public_identity.s,public_identity.s,public_identity.len) != 0
		 		)
		 	   )	
		 		   hdr.len += id->public_identity.len+hdr_p_associated_uri2.len;	 
		 }
	}
	hdr.s = pkg_malloc(hdr.len);
	if (!hdr.s){
		LOG(L_ERR,"ERR:"M_NAME":insert_p_associated_uri: Error allocating %d bytes\n",
			hdr.len);
		goto error;
	}
	hdr.len=0;
	memcpy(hdr.s+hdr.len,hdr_p_associated_uri1.s,hdr_p_associated_uri1.len);
	hdr.len+=hdr_p_associated_uri1.len;

	memcpy(hdr.s+hdr.len,public_identity.s,public_identity.len);
	hdr.len+=public_identity.len;

	if (s){
		for(i=0;i<s->service_profiles_cnt;i++)
		 for(j=0;j<s->service_profiles[i].public_identities_cnt;j++){
		 	id = &(s->service_profiles[i].public_identities[j]);
		 	if (!id->barring&&
		 		(id->public_identity.len != public_identity.len ||
		 		 memcmp(id->public_identity.s,public_identity.s,public_identity.len) != 0
		 		)
		 	   )	
		 	{
				memcpy(hdr.s+hdr.len,hdr_p_associated_uri2.s,hdr_p_associated_uri2.len);
				hdr.len+=hdr_p_associated_uri2.len;
				
				memcpy(hdr.s+hdr.len,id->public_identity.s,id->public_identity.len);
				hdr.len+=id->public_identity.len;
		 	}
		 }
	}


	memcpy(hdr.s+hdr.len,hdr_p_associated_uri3.s,hdr_p_associated_uri3.len);
	hdr.len+=hdr_p_associated_uri3.len;

	LOG(L_DBG,"DBG:"M_NAME":insert_p_associated_uri: <%.*s>\n",
		hdr.len,hdr.s);
	
	if (!cscf_add_header_rpl(msg,&hdr))	goto error;


	if (hdr.s) pkg_free(hdr.s);	
	return 1;
error:
	if (hdr.s) pkg_free(hdr.s);
	return 0;	
}

/**
 * Save the contacts.
 * 1. Parse the user data
 * 2. Parse contacts in the message
 * 3. Update the contacts accordingly
 * 4. Call function to add to the reply the P-Associated-URI header
 * @param msg - the SIP Register that contains the Expire and Contact headers
 * @param assignment_type - as sent in the SAR
 * @param xml - the user data as received in the SAA
 * @returns CSCF_RETURN_TRUE if ok, CSCF_RETURN_FALSE on error or CSCF_RETURN_BREAK on response sent out
 */
int save_location(struct sip_msg *msg,int assignment_type,str *xml,str *ccf1,str *ccf2,str *ecf1,str *ecf2)
{
	ims_subscription *s=0,*s_copy=0;
	contact_t *ci;
	contact_body_t* b=0;	
	struct hdr_field *h;
	str ua={0,0};
	str path={0,0};
	int result = CSCF_RETURN_FALSE;
	int max_expires, expires_hdr,expires;
	unsigned int exp;
	
	if (xml && xml->len) {
		s = parse_user_data(*xml);
		s_copy = s;
		if (!s){
			LOG(L_ERR,"ERR:"M_NAME":save_location: error parsing user data\n");
			goto error;
		}	
		print_user_data(L_DBG,s);
	}
	
	if (parse_headers(msg, HDR_EOH_F, 0) <0) {
		LOG(L_ERR,"ERR:"M_NAME":save_location: error parsing headers\n");
		goto error;
	}	
	
	b = cscf_parse_contacts(msg);
	
	if (!b||(!b->contacts && !b->star)) {
		LOG(L_ERR,"ERR:"M_NAME":save_location: No contacts found\n");
		goto error;
	}
			
	/* check for too brief interval for registration */
	expires_hdr = cscf_get_expires_hdr(msg);
	max_expires = expires_hdr;		
	
	for(h=msg->contact;h;h=h->next)
		if (h->type==HDR_CONTACT_T && h->parsed)
		 for(ci=((contact_body_t*)h->parsed)->contacts;ci;ci=ci->next){
			if(ci->expires){
				if (!str2int(&(ci->expires->body), (unsigned int*)&exp)){
					expires = exp;
					if (expires>max_expires) max_expires = expires;
				}
				else expires = -1;
			}
			else expires = expires_hdr;
			if (expires>0 && expires<registration_min_expires){
				if (!cscf_add_header_rpl(msg,&scscf_registration_min_expires)) return CSCF_RETURN_ERROR;			
				S_REGISTER_reply(msg,423,MSG_423_INTERVAL_TOO_BRIEF);		
				return CSCF_RETURN_BREAK;
			}		
		}
	/* we might get gere and not know what to do actually - e.g. from S_update_contacts */
	if (assignment_type<0){
		if (max_expires>0) {
			str public_identity;
			public_identity=cscf_get_public_identity(msg);

			if (r_is_registered_id(public_identity)) 
				assignment_type = AVP_IMS_SAR_RE_REGISTRATION;
			else
				assignment_type = AVP_IMS_SAR_REGISTRATION;
		}
		else {
			if (server_assignment_store_data) 
				assignment_type = AVP_IMS_SAR_USER_DEREGISTRATION_STORE_SERVER_NAME;
			else assignment_type = AVP_IMS_SAR_USER_DEREGISTRATION;
		}
	}
 	//LOG(L_CRIT,"max_expires %d assign_type %d\n",max_expires, assignment_type);

	ua = cscf_get_user_agent(msg);
	
	path = cscf_get_path(msg);
	
	/* we insert p associated uri first because update will destroy s */
	if (assignment_type==AVP_IMS_SAR_REGISTRATION ||
		assignment_type==AVP_IMS_SAR_RE_REGISTRATION)
		if (!insert_p_associated_uri(msg,s)) goto error;
	
	result = update_contacts(msg,assignment_type, b->star,  &s, &ua,&path,ccf1,ccf2,ecf1,ecf2); 

	if (path.s) pkg_free(path.s);
	return result;
error:
	if (path.s) pkg_free(path.s);
	if (s && s==s_copy) free_user_data(s);	
	return result;
}


/**
 * Save the contacts.
 * @param msg - the SIP Register that contains the Expire and Contact headers
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if OK, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_ERROR on error
 */
int S_update_contacts(struct sip_msg *msg,char *str1,char *str2)
{
	return save_location(msg, -1,0,0,0,0,0);
}

str route_hdr1={"Route: ",7};
str route_hdr2={"\r\n",2};
/** 
 * Adds a Route header containing the saved Path.
 * So that the message will be Routed through the P-CSCF of the user
 * @param msg - the SIP message to process
 * @param c - the registrar contact structure of this user
 * @return 1 on success, 0 on failure
 */
static inline int r_add_route_path(struct sip_msg *msg,r_contact *c)
{
	str hdr={0,0};
	if (!c->path.len) return 0;
	hdr.len = route_hdr1.len + c->path.len + route_hdr2.len;
	hdr.s = pkg_malloc(hdr.len);
	if (!hdr.s){
		LOG(L_ERR,"ERR:"M_NAME":r_add_route_path: Error allocating %d bytes\n",hdr.len);	
		return 0;
	}
	hdr.len=0;
	memcpy(hdr.s+hdr.len,route_hdr1.s,route_hdr1.len);
	hdr.len+=route_hdr1.len;

	memcpy(hdr.s+hdr.len,c->path.s,c->path.len);
	hdr.len+=c->path.len;

	memcpy(hdr.s+hdr.len,route_hdr2.s,route_hdr2.len);
	hdr.len+=route_hdr2.len;
	
	if (!cscf_add_header_first(msg,&hdr,HDR_ROUTE_T)){
		//pkg_free(hdr.s);
		return 0;
	}
	
	//pkg_free(hdr.s);
	return 1;
}



str p_called_party_id_hdr1={"P-Called-Party-ID: <",20};
str p_called_party_id_hdr2={">\r\n",3};
/** 
 * Adds the P-Called-Party-ID header containing the Request-URI before that will be
 * resolved to the contact address.
 * @param msg - the SIP message to process
 * @return 1 on success, 0 on failure
 */
static inline int r_add_p_called_party_id(struct sip_msg *msg)
{
	str uri;
	str hdr={0,0};
	if (!msg||!msg->first_line.type==SIP_REQUEST) return 0;
	uri = msg->first_line.u.request.uri;
	
	hdr.len = p_called_party_id_hdr1.len + uri.len + p_called_party_id_hdr2.len;
	hdr.s = pkg_malloc(hdr.len);
	if (!hdr.s){
		LOG(L_ERR,"ERR:"M_NAME":r_add_p_called_party_id: Error allocating %d bytes\n",hdr.len);	
		return 0;
	}
	hdr.len=0;
	memcpy(hdr.s+hdr.len,p_called_party_id_hdr1.s,p_called_party_id_hdr1.len);
	hdr.len+=p_called_party_id_hdr1.len;

	memcpy(hdr.s+hdr.len,uri.s,uri.len);
	hdr.len+=uri.len;

	memcpy(hdr.s+hdr.len,p_called_party_id_hdr2.s,p_called_party_id_hdr2.len);
	hdr.len+=p_called_party_id_hdr2.len;
	
	if (!cscf_add_header(msg,&hdr,HDR_OTHER_T)){
		//pkg_free(hdr.s);
		return 0;
	}
	
	//pkg_free(hdr.s);
	return 1;
}



//static str lookup_sip={"sip:",4};
/** 
 * Lookup for the Request-URI in registrar and rewrite request URI with the contact address.
 * Also adds routing to the destination's path.
 * Depending on the value of append_branches, the request is forked or not on multiple contacts.
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if found, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_lookup(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE,i;
	str uri,dst={0,0};
	r_public *p=0;
	r_contact *c=0;

	LOG(L_DBG,"DBG:"M_NAME":S_lookup: Looking up for contacts\n");
	//print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_lookup: This message is not a request\n");
		goto error;
	}		
	
	if (!cscf_get_terminating_identity(msg,&uri)){
		LOG(L_ERR,"ERR:"M_NAME":S_lookup: Error extracting terminating uri!!!\n");
		return CSCF_RETURN_ERROR;
	}
	
	LOG(L_DBG,"DBG:"M_NAME":S_lookup: Looking for <%.*s>\n",uri.len,uri.s);
	
	p = get_r_public(uri);
//	pkg_free(uri.s);
	if (!p) return CSCF_RETURN_FALSE;
	if (p->reg_state!=IMS_USER_REGISTERED){
		r_unlock(p->hash);
		return CSCF_RETURN_FALSE;
	}
	
	r_act_time();
	c = p->head;
	while(c){
		if (r_valid_contact(c)){
			LOG(L_DBG,"DBG:"M_NAME":S_lookup: Found at <%.*s>\n",
				c->uri.len,c->uri.s);
			
			r_add_route_path(msg,c);	
			r_add_p_called_party_id(msg);
			
			if (rewrite_uri(msg, &(c->uri)) < 0) {
				LOG(L_ERR,"ERR:"M_NAME":S_lookup: Error rewritting uri with <%.*s>\n",
					c->uri.len,c->uri.s);
				ret = CSCF_RETURN_ERROR;	
			} else {
				if (c->path.len) {
					dst=c->path;
					i=0;
					while(i<dst.len && dst.s[i]!='<')
						i++;
					i++;
					dst.s += i;
					dst.len -= i;
					i=0;					
					while(i<dst.len && (dst.s[i]!='>'))
						i++;
					dst.len = i;
				}
	
				if (*tmb.route_mode==MODE_ONFAILURE) {
					LOG(L_DBG,"DEBUG:"M_NAME":S_lookup: MODE_ONFAILURE, appending branch\n");
					/* need to append_branch for this first contact */
					if (append_branch(msg, c->uri.s, c->uri.len, dst.s,dst.len, 0, 0) == -1) {
						LOG(L_ERR,"ERR:"M_NAME":S_lookup: Error appending branch <%.*s>\n",
							c->uri.len,c->uri.s);
					}
				}
			
				msg->dst_uri.s = pkg_malloc(dst.len);
				if (!msg->dst_uri.s){
					LOG(L_ERR, "ERR:"M_NAME":S_lookup: Error allocating %d bytes\n",
						dst.len);
					msg->dst_uri.len=0;
					goto error;
				}
				memcpy(msg->dst_uri.s,dst.s,dst.len);
				msg->dst_uri.len = dst.len;
				ret = CSCF_RETURN_TRUE;	
				if (append_branches){
					c = c->next;
					while(c){
						if (r_valid_contact(c)) 														
							if (append_branch(msg, c->uri.s, c->uri.len, dst.s,dst.len, 0, 0) == -1) {
								LOG(L_ERR,"ERR:"M_NAME":S_lookup: Error appending branch <%.*s>\n",
									c->uri.len,c->uri.s);
							} 
						c = c->next;
					}
				}

			}
			
			
			break;
		}
		c = c->next;
	}
	
	if (p) r_unlock(p->hash);		
	return ret;
error:
	if (p) r_unlock(p->hash);		
	ret=CSCF_RETURN_ERROR;
	return ret;	
}


/**
 * Finds if the user is registered at this S-CSCF
 * @param public_identity - the public identity
 * @returns 1 if registered, 0 if not or error
 */
int r_is_registered_id(str public_identity)
{
	int ret=0;
	r_public *p;
	r_contact *c;

	LOG(L_DBG,"DBG:"M_NAME":S_is_registered_id: Looking if registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	
	p = get_r_public(public_identity);
	if (!p) return 0;
	if (p->reg_state!=IMS_USER_REGISTERED){
		r_unlock(p->hash);
		return 0;
	}
	
	r_act_time();
	c = p->head;
	while(c){
		if (r_valid_contact(c)){
			LOG(L_DBG,"DBG:"M_NAME":S_is_registered_id: Found at <%.*s>\n",
				c->uri.len,c->uri.s);
			ret = 1;			
			break;
		}
		c = c->next;
	}
	r_unlock(p->hash);
	return ret;
}

/**
 * Finds if the user is not registered at this S-CSCF
 * @param public_identity - the SIP message
 * @returns 1 if registered, 0 if not or error
 */
int r_is_not_registered_id(str public_identity)
{
	int ret=0;
	r_public *p;

	LOG(L_DBG,"DBG:"M_NAME":S_is_not_registered_id: Looking if NOT registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	
	p = get_r_public(public_identity);
	if (!p) return 1;
	if (p->reg_state==IMS_USER_NOT_REGISTERED){
		r_unlock(p->hash);
		return 1;
	}
	r_unlock(p->hash);
	return ret;
}

/**
 * Finds if the user is unregistered at this S-CSCF
 * @param public_identity - the SIP message
 * @returns 1 if registered, 0 if not or error
 */
int r_is_unregistered_id(str public_identity)
{
	int ret=0;
	r_public *p;
	r_contact *c;

	LOG(L_DBG,"DBG:"M_NAME":S_is_unregistered_id: Looking if registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	
	p = get_r_public(public_identity);
	if (!p) return 0;
	if (p->reg_state!=IMS_USER_UNREGISTERED){
		r_unlock(p->hash);
		return 0;
	}
	
	r_act_time();
	c = p->head;
	while(c){
		if (r_valid_contact(c)){
			LOG(L_DBG,"DBG:"M_NAME":S_is_unregistered_id: Found at <%.*s>\n",
				c->uri.len,c->uri.s);
			ret = 1;			
			break;
		}
		c = c->next;
	}
	r_unlock(p->hash);
	return ret;
}


/**
 * Finds if the terminating user (in the RequestURI) is registered at this S-CSCF.
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if registered, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_term_registered(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str uri={0,0};	

	LOG(L_DBG,"DBG:"M_NAME":S_term_registered: Looking if registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_term_registered: This message is not a request\n");
		goto error;
	}		

	if (!cscf_get_terminating_identity(msg,&uri)){
		LOG(L_ERR,"ERR:"M_NAME":S_term_registered: Error extracting terminating uri!!!\n");
		return CSCF_RETURN_ERROR;
	}	
	
	LOG(L_DBG,"DBG:"M_NAME":S_term_registered: Looking for <%.*s>\n",uri.len,uri.s);
	
	if (r_is_registered_id(uri)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	
}


/**
 * Finds if the terminating user (in the RequestURI) is not registered at this S-CSCF
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if not registered, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_term_not_registered(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str uri={0,0};	

	LOG(L_DBG,"DBG:"M_NAME":S_term_not_registered: Looking if registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_term_not_registered: This message is not a request\n");
		goto error;
	}		
	
	if (!cscf_get_terminating_identity(msg,&uri)){
		LOG(L_ERR,"ERR:"M_NAME":S_term_not_registered: Error extracting terminating uri!!!\n");
		return CSCF_RETURN_ERROR;
	}	
	
	LOG(L_DBG,"DBG:"M_NAME":S_term_not_registered: Looking for <%.*s>\n",uri.len,uri.s);
	
	if (r_is_not_registered_id(uri)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	
}

/**
 * Finds if the terminating user (in the RequestURI) is unregistered at this S-CSCF
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if unregistered, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_term_unregistered(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str uri={0,0};	

	LOG(L_DBG,"DBG:"M_NAME":S_term_unregistered: Looking if registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_term_unregistered: This message is not a request\n");
		goto error;
	}		

	
	if (!cscf_get_terminating_identity(msg,&uri)){
		LOG(L_ERR,"ERR:"M_NAME":S_term_unregistered: Error extracting terminating uri!!!\n");
		return CSCF_RETURN_ERROR;
	}
	
	LOG(L_DBG,"DBG:"M_NAME":S_term_unregistered: Looking for <%.*s>\n",uri.len,uri.s);
	
	if (r_is_unregistered_id(uri)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	
}


/**
 * Finds if the originating user (in P-Asserted-Identity) is registered at this S-CSCF
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if not registered, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_orig_registered(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str public_identity;	

	LOG(L_DBG,"DBG:"M_NAME":S_orig_registered: Looking if registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_orig_registered: This message is not a request\n");
		goto error;
	}		

	public_identity = cscf_get_asserted_identity(msg);
	
	LOG(L_DBG,"DBG:"M_NAME":S_orig_registered: Looking for <%.*s>\n",public_identity.len,public_identity.s);
	
	if (r_is_registered_id(public_identity)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	
}

static str p_asserted_identity_s={"P-Asserted-Identity: <",22};
static str p_asserted_identity_e={">\r\n",3};
static str p_asserted_identity_param={";user=phone",11};
static str p_asserted_identity_tel={"tel:",4};
static str route_header={"Route",5};
/**
 * Adds suplimentary P-Asserted-Identity.
 * - if the original header is a SIP URI like sip:+(...) then adds a tel:+(...);user=phone		
 * - if the original header is a TEL URL then it adds an alias SIP URI
 * @param msg - the message to operate on
 * @returns #CSCF_RETURN_TRUE if added, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_ERROR on error
 */
int S_add_p_asserted_identity(struct sip_msg *msg,char *str1,char *str2)
{
	str public_identity;	
	r_public *p;
	str asserted_id_h;
	struct hdr_field* route;
	int i,j;

	LOG(L_DBG,"DBG:"M_NAME":S_add_p_asserted_identity: Looking if registered\n");
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST){
		LOG(L_ERR,"ERR:"M_NAME":S_add_p_asserted_identity: This message is not a request\n");
		goto error;
	}
	
	//get first route header
	route =	cscf_get_header(msg,route_header);
	if(route && strncasecmp(route->body.s,"sip:iscmark",11)==0){
		 LOG(L_ERR, "DBG"M_NAME":S_add_p_asserted_identity: Request came from AS\n");
		 return CSCF_RETURN_FALSE;
	}
	public_identity = cscf_get_asserted_identity(msg);
	if(!public_identity.len) goto error;
	
	//check if it is a sip:+ uri
	if(strncasecmp(public_identity.s,"sip:+",5)==0){
		asserted_id_h.len = p_asserted_identity_s.len+public_identity.len+p_asserted_identity_param.len+p_asserted_identity_e.len;
		asserted_id_h.s = pkg_malloc(asserted_id_h.len);
		if (!asserted_id_h.s){
			LOG(L_ERR, "ERR"M_NAME":S_add_p_asserted_identity: Error allocating %d bytes\n",asserted_id_h.len);
            asserted_id_h.len=0;
			goto error;
		}else{
			asserted_id_h.len = 0;
			str aux;
			aux.len=public_identity.len-4;
			aux.s=public_identity.s+4;
			STR_APPEND(asserted_id_h,p_asserted_identity_s);
			STR_APPEND(asserted_id_h,p_asserted_identity_tel);
			STR_APPEND(asserted_id_h,aux);
			STR_APPEND(asserted_id_h,p_asserted_identity_param);
			STR_APPEND(asserted_id_h,p_asserted_identity_e);
			if (!cscf_add_header_first(msg,&asserted_id_h,HDR_OTHER_T))
				pkg_free(asserted_id_h.s);
				return CSCF_RETURN_TRUE;
			}
	}else{
	//check if it is a tel uri
		p = get_r_public(public_identity);
		if (!p||!p->s) goto error;
		
		for(i=0;i<p->s->service_profiles_cnt;i++)
			for(j=0;j<p->s->service_profiles[i].public_identities_cnt;j++)
				if (strncasecmp(p->s->service_profiles[i].public_identities[j].public_identity.s,"tel:",4)==0){
					asserted_id_h.len = p_asserted_identity_s.len+p->s->service_profiles[i].public_identities[j].public_identity.len+p_asserted_identity_e.len;
					asserted_id_h.s = pkg_malloc(asserted_id_h.len);
					if (!asserted_id_h.s){
						LOG(L_ERR, "ERR"M_NAME":S_add_p_asserted_identity: Error allocating %d bytes\n", asserted_id_h.len);
						asserted_id_h.len=0;
						goto error;
					}else{
						asserted_id_h.len = 0;
						STR_APPEND(asserted_id_h,p_asserted_identity_s);
						STR_APPEND(asserted_id_h,p->s->service_profiles[i].public_identities[j].public_identity);
						STR_APPEND(asserted_id_h,p_asserted_identity_e);
						if (!cscf_add_header_first(msg,&asserted_id_h,HDR_OTHER_T))
							pkg_free(asserted_id_h.s);
					}			
					r_unlock(p->hash);
					return CSCF_RETURN_TRUE;
				}
		r_unlock(p->hash);
	}

error:
	return CSCF_RETURN_ERROR;
}

/**
 * Finds if the originating user (in P-Asserted-Identity) is not registered at this S-CSCF
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if not registered, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_orig_not_registered(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str public_identity;	

	LOG(L_DBG,"DBG:"M_NAME":S_orig_not_registered: Looking if registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_orig_not_registered: This message is not a request\n");
		goto error;
	}		

	public_identity = cscf_get_asserted_identity(msg);
	
	if (public_identity.len==0 || public_identity.s==0) {
		LOG(L_ERR,"ERR:"M_NAME":S_orig_not_registered: Asserted Identity not found!\n");
		goto error;
	}
	
	LOG(L_DBG,"DBG:"M_NAME":S_orig_not_registered: Looking for <%.*s>\n",public_identity.len,public_identity.s);
	
	if (r_is_not_registered_id(public_identity)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	
}

/**
 * Finds if the originating user (in P-Asserted-Identity) is unregistered at this S-CSCF
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if not registered, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_orig_unregistered(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str public_identity;	

	LOG(L_DBG,"DBG:"M_NAME":S_orig_unregistered: Looking if registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_orig_unregistered: This message is not a request\n");
		goto error;
	}		

	public_identity = cscf_get_asserted_identity(msg);

	if (public_identity.len==0 || public_identity.s==0) {
		LOG(L_ERR,"ERR:"M_NAME":S_orig_unregistered: Asserted Identity not found!\n");
		goto error;
	}
	
	LOG(L_DBG,"DBG:"M_NAME":S_orig_unregistered: Looking for <%.*s>\n",public_identity.len,public_identity.s);
		
	if (r_is_unregistered_id(public_identity)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	
}

/**
 * Finds if the user in To header is not registered at this S-CSCF
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if not registered, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_is_not_registered(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	//struct sip_uri puri;
	//str uri;
	str public_identity;	

	LOG(L_DBG,"DBG:"M_NAME":S_is_not_registered: Looking if registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_is_not_registered: This message is not a request\n");
		goto error;
	}		

	public_identity = cscf_get_public_identity(msg);
	
	LOG(L_DBG,"DBG:"M_NAME":S_is_not_registered: Looking for <%.*s>\n",public_identity.len,public_identity.s);
	
	if (r_is_not_registered_id(public_identity)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	

/*	
	if (msg->new_uri.s) uri = msg->new_uri;
	else uri = msg->first_line.u.request.uri;
	
	if (parse_uri(uri.s, uri.len, &puri) < 0) {
		LOG(L_INFO,"INF:"M_NAME":S_is_not_registered: Error parsing uri <%.*s>\n",
			uri.len,uri.s);
		goto error;
	}
	uri.len = lookup_sip.len+puri.user.len+1+puri.host.len;
	uri.s = pkg_malloc(uri.len);
	if (!uri.s){
		LOG(L_ERR,"ERR:"M_NAME":S_is_not_registered: Error allocating %d bytes\n",uri.len);
		return CSCF_RETURN_ERROR;
	}
	uri.len=0;
	memcpy(uri.s,lookup_sip.s,lookup_sip.len);
	uri.len+=lookup_sip.len;
	memcpy(uri.s+uri.len,puri.user.s,puri.user.len);
	uri.len+=puri.user.len;
	uri.s[uri.len++]='@';
	memcpy(uri.s+uri.len,puri.host.s,puri.host.len);
	uri.len+=puri.host.len;
	
	
	LOG(L_DBG,"DBG:"M_NAME":S_is_not_registered: Looking for <%.*s>\n",uri.len,uri.s);
	
	if (r_is_not_registered_id(uri)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;
*/	
}	


/**
 * Finds if the message contains as first route the specific service route for originating case (indicated in the Service-Route at registration),
 *  or if the topmost route contains the "orig" parameter.
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if originating, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_mobile_originating(struct sip_msg *msg,char *str1,char *str2)
{
	//int ret=CSCF_RETURN_FALSE;
	str r={0,0};
	struct hdr_field *h;
	rr_t *rt;
	str* uri;

	r = cscf_get_first_route(msg,&h);
	LOG(L_DBG,"DBG:"M_NAME":S_mobile_originating: <%.*s>\n",r.len,r.s);	
	
	if (!r.len) return CSCF_RETURN_FALSE;
		
	if (r.len >= scscf_service_route_uri.len &&
		strncasecmp(scscf_service_route_uri.s,r.s,r.len)==0){
			return CSCF_RETURN_TRUE;
		}
	
	rt = (rr_t*)h->parsed; // first route exists and has been parsed by cscf_get_first_route
	uri = &rt->nameaddr.uri;
	struct sip_uri puri;
	if (parse_uri(uri->s, uri->len, &puri) < 0) {
		LOG(L_ERR, "DBG:"M_NAME":S_mobile_originating: Error while parsing the first route URI\n");
		return -1;
	}
	if (puri.params.len < 4) return CSCF_RETURN_FALSE;
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


/**
 * Finds if the user exists and it is barred
 * @param public_identity - the SIP message
 * @returns 1 if found and barred, 0 if not or error
 */
int S_is_barred(str public_identity)
{
	int ret=0,i,j;
	r_public *p;

	LOG(L_DBG,"DBG:"M_NAME":S_is_not_registered_id: Looking if NOT registered\n");
//	print_r(L_INFO);
	/* First check the parameters */
	
	p = get_r_public(public_identity);
	if (!p) return 0;
	if (!p->s) return 0;
	
	for(i=0;i<p->s->service_profiles_cnt;i++)
		for(j=0;j<p->s->service_profiles[i].public_identities_cnt;j++)
			if (p->s->service_profiles[i].public_identities[j].barring &&
				p->s->service_profiles[i].public_identities[j].public_identity.len == public_identity.len &&
				strncasecmp(p->s->service_profiles[i].public_identities[j].public_identity.s,public_identity.s,public_identity.len)==0){
					r_unlock(p->hash);					
					return 1;
				}
	r_unlock(p->hash);
	return ret;
}

/**
 * Finds if the originating user is  barred.
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if barred, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_originating_barred(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str public_identity;	

	LOG(L_DBG,"DBG:"M_NAME":S_originating_barred: Looking if barred\n");
//	print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_originating_barred: This message is not a request\n");
		goto error;
	}		

	public_identity = cscf_get_asserted_identity(msg);
	
	if (public_identity.len==0 || public_identity.s==0) {
		LOG(L_ERR,"ERR:"M_NAME":S_originating_barred: Asserted Identity not found!\n");
		goto error;
	}
	
	LOG(L_DBG,"DBG:"M_NAME":S_originating_barred: Looking for <%.*s>\n",public_identity.len,public_identity.s);
	
	if (S_is_barred(public_identity)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	
}

/**
 * Finds if the terminating user is  barred
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if barred, else #CSCF_RETURN_FALSE or #CSCF_RETURN_ERROR on error
 */
int S_terminating_barred(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str uri;	

	LOG(L_DBG,"DBG:"M_NAME":S_terminating_barred: Looking if barred\n");
//	print_r(L_INFO);
	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_terminating_barred: This message is not a request\n");
		goto error;
	}		

	if (!cscf_get_terminating_identity(msg,&uri)){
		LOG(L_ERR,"ERR:"M_NAME":S_terminating_barred: Error extracting terminating uri!!!\n");
		return CSCF_RETURN_ERROR;
	}
	
	LOG(L_DBG,"DBG:"M_NAME":S_terminating_barred: Looking for <%.*s>\n",uri.len,uri.s);
	
	if (S_is_barred(uri)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;
			
	
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	
}
