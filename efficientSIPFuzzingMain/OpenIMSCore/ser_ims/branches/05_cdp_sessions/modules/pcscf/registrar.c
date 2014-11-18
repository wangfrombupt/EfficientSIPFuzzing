/*
 * $Id: registrar.c 158 2007-02-27 19:49:56Z vingarzan $
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
 * Proxy-CSCF -Registrar Related Operations
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
#include "../../dset.h"

#include "registrar.h"
#include "registrar_subscribe.h"
#include "mod.h"
#include "sip.h"
#include "nat_helper.h"
#include "security.h"


extern struct tm_binds tmb;				/**< Structure with pointers to tm funcs 	*/
extern time_t time_now;					/**< current time 							*/
extern r_hash_slot *registrar;			/**< the actual registrar					*/
extern int 	   r_hash_size;				/**< number of hash slots in the registrar	*/
extern int pcscf_nat_enable; 			/**< whether to enable NAT					*/
extern int pcscf_nat_ping; 				/**< whether to ping anything 				*/
extern int pcscf_nat_pingall; 			/**< whether to ping also the UA that don't look like being behind a NAT */
extern int pcscf_nat_detection_type; 	/**< the NAT detection tests 				*/


/**
 * The Registrar timer looks for expires contacts and removes them.
 * For the non-deleted contacts a ping is sent if the UA is behind a NAT.
 * @param ticks - the current time
 * @param param - pointer to the domain_list
 */
void registrar_timer(unsigned int ticks, void* param)
{
	r_contact *c,*cn;
	int i;
	
	LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Called at %d\n",ticks);
	if (!registrar) registrar = (r_hash_slot*)param;

	r_act_time();
	
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
			c = registrar[i].head;
			while(c){
				cn = c->next;
				switch (c->reg_state){
					case NOT_REGISTERED:
						LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> Not Registered and removed.\n",
								c->uri.len,c->uri.s);
						del_r_contact(c);
						break;
					case REGISTERED:
						if (c->expires<=time_now) {
							LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> expired and Deregistered.\n",
								c->uri.len,c->uri.s);		
							if (c->ipsec){
								/* If we have IPSec SAs, we keep them 60 seconds more to relay further messages */
								c->reg_state = DEREGISTERED;
								c->expires = time_now + 60;
							}else{
								LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> expired and removed.\n",
									c->uri.len,c->uri.s);						
								del_r_contact(c);
							}
						}
						break;
					case DEREGISTERED:
						if (c->expires<=time_now) {
							LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> expired and removed.\n",
								c->uri.len,c->uri.s);		
							P_drop_ipsec(c);
							del_r_contact(c);
						}
						break;
					case REG_PENDING:
						if (c->expires<=time_now) {
							LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> Registration pending expired and removed.\n",
								c->uri.len,c->uri.s);		
							P_drop_ipsec(c);
							del_r_contact(c);
						}
						break;
				}				
				if (pcscf_nat_enable && pcscf_nat_ping) nat_send_ping(c);
				c = cn;
			}
		r_unlock(i);
	}
	print_r(L_INFO);
}


/**
 * Calculates the expiration time for one contact.
 * Tries to use the Expiration header, if not present then use the 
 * expires parameter of the contact, if param not present it defaults
 * to the default value.
 * Also checks 
 * @param c - the contact to calculate for
 * @param expires_hdr - value of expires hdr if present, if not -1
 * @param local_time_now - the local time
 * @returns the time of expiration
 */
static inline int r_calc_expires(contact_t *c,int expires_hdr, int local_time_now)
{
	unsigned int r;
	if (expires_hdr>=0) r = expires_hdr;
	if (c) 
		str2int(&(c->expires->body), (unsigned int*)&r);
		
	return local_time_now+r;
}


/**
 * Updates the registrar with the new values
 * @param c - contact to update
 * @param is_star - whether this was a STAR contact header
 * @param expires_hdr - value of the Expires header
 * @param public_id - array of public identities attached to this contact
 * @param public_id_cnt - size of the public_id array
 * @param service_route - array of Service-Routes
 * @param service_route_cnt - size of the service_route array
 * @param pinhole - NAT pinhole 
 * @returns the maximum expiration time, -1 on error
 */
static inline int update_contacts(struct sip_msg *msg,unsigned char is_star,int expires_hdr,
	str *public_id,int public_id_cnt,str *service_route,int service_route_cnt, r_nat_dest ** pinhole)
{
	r_contact *rc;
	enum Reg_States reg_state=REGISTERED;
	int expires=0;
	int is_default=0,i;
	struct sip_uri puri;
	int max_expires=-1;
	int local_time_now;
	struct hdr_field *h;
	contact_t *c;
	
	
	r_act_time();
	local_time_now = time_now;
	if (is_star){
		/* first of all, we shouldn't get here...
		 * then, we will update on NOTIFY */
		return 0;
	}	
	for(h=msg->contact;h;h=h->next)
		if (h->type==HDR_CONTACT_T && h->parsed)
		 for(c=((contact_body_t*)h->parsed)->contacts;c;c=c->next){
			LOG(L_DBG,"DBG:"M_NAME":update_contact: <%.*s>\n",c->uri.len,c->uri.s);
			
			expires = r_calc_expires(c,expires_hdr,local_time_now);
			
			if (parse_uri(c->uri.s,c->uri.len,&puri)<0){
				LOG(L_DBG,"DBG:"M_NAME":update_contact: Error parsing Contact URI <%.*s>\n",c->uri.len,c->uri.s);
				continue;			
			}
			if (puri.port_no==0) puri.port_no=5060;
			LOG(L_DBG,"DBG:"M_NAME":update_contact: %d %.*s : %d\n",
				puri.proto, puri.host.len,puri.host.s,puri.port_no);
			
			if (expires>local_time_now) {		
				rc = update_r_contact(puri.host,puri.port_no,puri.proto,
					&(c->uri),&reg_state,&expires,&service_route,&service_route_cnt, pinhole);
				if (expires-time_now>max_expires) max_expires=expires-time_now;
			}
			else {
				reg_state = DEREGISTERED;
				expires = local_time_now+30;
				rc = update_r_contact(puri.host,puri.port_no,puri.proto,
						0,&reg_state,&expires,0,0,0);
				if (rc) r_unlock(rc->hash);
				if (0>max_expires) max_expires = 0;
				rc = 0;
			}
			
			/** Add the public identities */
			if (rc){
				is_default=1;
				if (public_id_cnt){
					update_r_public(rc,public_id[0],&is_default);
					is_default=0;
					for(i=1;i<public_id_cnt;i++)
						update_r_public(rc,public_id[i],&is_default);
				}
				r_unlock(rc->hash);
			}
	}
	return max_expires;
}




/**
 * Save the contacts and their associated public ids.
 * @param rpl - the SIP Register 200 OK response that contains the Expire and Contact headers
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if OK, #CSCF_RETURN_ERROR on error
 */
int P_save_location(struct sip_msg *rpl,char *str1, char *str2)
{
	struct sip_msg *req;
	contact_body_t* b=0;	
	str realm;
	int expires_hdr=0;
	str *public_id=0;
	int public_id_cnt=0;
	int expires;
	str *service_route=0;
	int service_route_cnt;
	r_nat_dest * pinhole=0;
	
	req = cscf_get_request_from_reply(rpl);
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":P_save_location: No transactional request found.\n");
		goto error;
	}
	
	expires_hdr = cscf_get_expires_hdr(rpl);
	/** Removed because this would parse the hdr, but then it will fail to free the hdr->parsed */
//	if (expires_hdr<0) 
//		expires_hdr = cscf_get_expires_hdr(req);
	
	if (parse_headers(rpl, HDR_EOH_F, 0) <0) {
		LOG(L_ERR,"ERR:"M_NAME":r_save_location: error parsing headers\n");
		return CSCF_RETURN_ERROR;
	}	
	
	b = cscf_parse_contacts(rpl);
	
	if (!b||(!b->contacts && !b->star)) {
		LOG(L_DBG,"DBG:"M_NAME":r_save_location: No contacts found\n");
		return 0;
	}
	
	realm = cscf_get_realm(req);
	
	cscf_get_p_associated_uri(rpl,&public_id,&public_id_cnt);
	
	service_route = cscf_get_service_route(rpl,&service_route_cnt);
	
	if(pcscf_nat_enable && nat_msg_origin(req, &pinhole) < 0) {
		LOG(L_ERR,"ERR:"M_NAME":P_save_location: error on determining nat pinhole\n");
	}
	
//	if(pinhole != NULL)
//		LOG(L_ERR, "**************************\n");
//	else LOG(L_ERR, "***************************%d**********************\n",pcscf_nat_pingall); 
		
	if ((expires=update_contacts(rpl,b->star,expires_hdr,public_id,public_id_cnt,service_route,service_route_cnt,&pinhole))<0) 
		goto error;

	//print_r(L_ERR);
	
	
	if (service_route)	pkg_free(service_route);
	if (public_id) pkg_free(public_id);
	return CSCF_RETURN_TRUE;
error:
	if (service_route)	pkg_free(service_route);
	if (public_id) pkg_free(public_id);
	return CSCF_RETURN_ERROR;
}


/**
 * Finds if the message is integrity protected
 * @param host - host of the UE
 * @param port - port of the UE
 * @param transport - transport of the UE
 * @returns 1 if registered, 0 if not or error
 */
int r_is_integrity_protected(str host,int port,int transport)
{
	int ret=0;
	r_contact *c;

	if (port==0) port=5060;
//	LOG(L_ERR,"DBG:"M_NAME":r_is_registered: Looking if registered <%d://%.*s:%d>\n",
//		transport,host.len,host.s,port);

//	print_r(L_INFO);
	c = get_r_contact(host,port,transport);

	if (!c) return 0;
	
	if (!c->ipsec){
		r_unlock(c->hash);
		return 0;
	}
	
	if (c->ipsec->port_uc==port || c->ipsec->port_us==port){
		ret = 1;
	}
	r_unlock(c->hash);
	return ret;
}

/**
 * Finds if the user is registered.
 * @param host - host of the UE
 * @param port - port of the UE
 * @param transport - transport of the UE
 * @returns 1 if registered, 0 if not or error
 */
int r_is_registered(str host,int port,int transport)
{
	int ret=0;
	r_contact *c;

	if (port==0) port=5060;
//	LOG(L_ERR,"DBG:"M_NAME":r_is_registered: Looking if registered <%d://%.*s:%d>\n",
//		transport,host.len,host.s,port);

//	print_r(L_INFO);
	c = get_r_contact(host,port,transport);

	if (!c){		
		return 0;
	}	
	r_act_time();
	if (r_reg_contact(c)){
		ret = 1;
	}
	r_unlock(c->hash);
	
	return ret;
}


/**
 * Asserts the identity of the user and returns the value
 * @param host - host of the UE
 * @param port - port of the UE
 * @param transport - transport of the UE
 * @param preferred - the P-Preferred-Identity header value
 * @returns 1 if registered, {0,0} if not or error
 */
name_addr_t r_assert_identity(str host,int port,int transport,name_addr_t preferred)
{
	r_contact *c;
	r_public *p;
	name_addr_t id;
	if (port==0) port=5060;

	memset(&id,0,sizeof(name_addr_t));
	
	LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: Asserting preferred id <%.*s>\n",
		preferred.uri.len,preferred.uri.s);
//	print_r(L_INFO);
	c = get_r_contact(host,port,transport);

	if (!c){
		LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: Contact not found\n");		
		return id;
	}
	r_act_time();
	if (!r_reg_contact(c)){
		LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: Contact expired\n");
		r_unlock(c->hash);	
		return id;
	}
	
	if (!c->head){
		LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: No public ids for this user\n");
		r_unlock(c->hash);
		return id;	
	}
	id.name = preferred.name;	
	if (!preferred.uri.len){
		p = c->head;
		while(p&&!p->is_default)
			p = p->next;
		if (p) {
			LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: to <%.*s>\n",p->aor.len,p->aor.s);
			id.uri=p->aor;
			r_unlock(c->hash);
			return id;
		} else {
			LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: to <%.*s>\n",c->head->aor.len,c->head->aor.s);
			id.uri=c->head->aor;
			r_unlock(c->hash);
			return id;	
		}
	}else{
		p = c->head;
		while(p){
			if (p->aor.len==preferred.uri.len &&
				strncasecmp(p->aor.s,preferred.uri.s,preferred.uri.len)==0)
			{
				LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: to <%.*s>\n",p->aor.len,p->aor.s);
				id.uri = preferred.uri;
				r_unlock(c->hash);
				return id;					
			}
			p = p->next;
		}
	}
	r_unlock(c->hash);
	return id;
}





