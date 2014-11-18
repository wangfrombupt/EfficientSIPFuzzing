/**
 * $Id: sip.c 398 2007-07-20 16:37:54Z placido $
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
 * IMS Service Control - SIP Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../../str.h"
#include "../../parser/parse_expires.h"
#include "../../dprint.h"
#include "../../mem/mem.h"
#include "../scscf/scscf_load.h"
#include "../tm/tm_load.h"


#include "sip.h"
#include "mod.h"

extern struct tm_binds isc_tmb;            /**< Structure with pointers to tm funcs 		*/
extern struct scscf_binds isc_scscfb;            /**< Structure with pointers to S-CSCF funcs 		*/

/**
 *	Compare 2 strings
 *  @param x - first string
 *  @param y - second string
 *	@returns 0 if equal, -1 or 1 else
 */
int str2cmp(str x, str y) {
	if (x.len < y.len) return -1;
	if (x.len > y.len) return +1;
	return strncmp(x.s, y.s, x.len);
}

/**
 *	Compare 2 strings case insensitive
 *  @param x - first string
 *  @param y - second string
 *	@returns 0 if equal, -1 or 1 else
 */
int str2icmp(str x, str y) {
	if (x.len < y.len) return -1;
	if (x.len > y.len) return +1;
	return strncasecmp(x.s, y.s, x.len);
}


static str bye_s={"BYE",3};
static str ack_s={"ACK",3};
static str prack_s={"PRACK",5};
static str update_s={"UPDATE",6};
static str notify_s={"NOTIFY",6};
/**
 * Check if the message is an initial request for a dialog. 
 *		- BYE, PRACK, UPDATE, NOTIFY belong to an already existing dialog
 * @param msg - the message to check
 * @returns 1 if initial, 0 if not
 */
int isc_is_initial_request(struct sip_msg *msg)
{
	if (msg->first_line.type != SIP_REQUEST ) return 0;
	if (strncasecmp(msg->first_line.u.request.method.s,bye_s.s,bye_s.len)==0) return 0;
	if (strncasecmp(msg->first_line.u.request.method.s,ack_s.s,ack_s.len)==0) return 0;
	if (strncasecmp(msg->first_line.u.request.method.s,prack_s.s,prack_s.len)==0) return 0;
	if (strncasecmp(msg->first_line.u.request.method.s,update_s.s,update_s.len)==0) return 0;
	if (strncasecmp(msg->first_line.u.request.method.s,notify_s.s,notify_s.len)==0) return 0;
	return 1;
}


static str register_s={"REGISTER",8};
/**
 * Check if the message is a REGISTER request 
 * @param msg - the message to check
 * @returns 1 if initial, 0 if not
 */
int isc_is_register(struct sip_msg *msg)
{
	if (msg->first_line.type != SIP_REQUEST ) return 0;
	if (strncasecmp(msg->first_line.u.request.method.s,register_s.s,register_s.len)==0) return 1;			
	return 0;
}


/**
 *	Delete parameters and stuff from uri.
 * @param uri - the string to operate on 
 */
static inline void isc_strip_uri(str *uri)
{
	int i;
	/* Strip the ending */
	i=0;
	while(i<uri->len&&uri->s[i]!='@')
		i++;
	while(i<uri->len&&
			uri->s[i]!=':'&&
			uri->s[i]!='/'&&
			uri->s[i]!='&')
		i++;
	uri->len=i;
}


str s_asserted_identity={"P-Asserted-Identity",19};
/**
 * Looks for the P-Asserted-Identity header and extracts its content
 * @param msg - the sip message
 * @returns the asserted identity
 */
str cscf_get_asserted_identity(struct sip_msg *msg)
{
	name_addr_t id;
	struct hdr_field *h;
	rr_t *r;
	memset(&id,0,sizeof(name_addr_t));
	if (!msg) return id.uri;
	if (parse_headers(msg, HDR_EOH_F, 0)<0) {
		return id.uri;
	}
	h = msg->headers;
	while(h)
	{
		if (h->name.len == s_asserted_identity.len  &&
			strncasecmp(h->name.s,s_asserted_identity.s,s_asserted_identity.len)==0)
		{
			if (parse_rr(h)<0){
				//This might be an old client
				LOG(L_CRIT,"WARN:"M_NAME":cscf_get_asserted_identity: P-Asserted-Identity header must contain a Nameaddr!!! Fix the client!\n");
				id.name.s = h->body.s;
				id.name.len = 0;
				id.len = h->body.len;
				id.uri = h->body;
				while(id.uri.len && (id.uri.s[0]==' ' || id.uri.s[0]=='\t' || id.uri.s[0]=='<')){
					id.uri.s = id.uri.s+1;
					id.uri.len --;
				}
				while(id.uri.len && (id.uri.s[id.uri.len-1]==' ' || id.uri.s[id.uri.len-1]=='\t' || id.uri.s[id.uri.len-1]=='>')){
					id.uri.len--;
				}
				return id.uri;	
			}
			r = (rr_t*) h->parsed;
			id = r->nameaddr;			
			free_rr(&r);
			h->parsed=r;
			//LOG(L_RIT,"%.*s",id.uri.len,id.uri.s);
			return id.uri;
		}
		h = h->next;
	}
	return id.uri;
}
/**
 *	Get the public identity from P-Asserted-Identity, or From if asserted not found.
 * @param msg - the SIP message
 * @param uri - uri to fill into
 * @returns 1 if found, 0 if not
 */
int isc_get_originating_user( struct sip_msg * msg, str *uri )
{
	struct to_body * from;
	*uri = cscf_get_asserted_identity(msg);
	if (!uri->len) {		
		/* Fallback to From header */
		if ( parse_from_header( msg ) == -1 ) {
			LOG(L_ERR,"ERROR:"M_NAME":isc_get_originating_user: unable to extract URI from FROM header\n" );
			return 0;
		}
		if (!msg->from) return 0;
		from = (struct to_body*) msg->from->parsed;
		*uri = from->uri;
		isc_strip_uri(uri);
	}
	DBG("DEBUG:"M_NAME":isc_get_originating_user: From %.*s\n", uri->len,uri->s );
	return 1;
}

/**
 *	Find if user is registered or not => TRUE/FALSE.
 * This uses the S-CSCF registrar to get the state.
 * @param uri - uri of the user to check
 * @returns the reg_state
 */
int isc_is_registered(str *uri)
{
	int result = 0;
	r_public *p;
	
	p = isc_scscfb.get_r_public(*uri);
	if (p) {
		result = p->reg_state;
		isc_scscfb.r_unlock(p->hash);
	}	
	return result;
}

/**
 *	Get terminating type for a user.
 * This uses the S-CSCF registrar to get the state.
 * @param uri - uri of the user to check
 * @returns #IFC_TERMINATING_SESSION if the user is registered or else #IFC_TERMINATING_UNREGISTERED
 */
inline int isc_get_terminating_type(str *uri)
{
	if (isc_is_registered(uri)) return IFC_TERMINATING_SESSION;
		else return IFC_TERMINATING_UNREGISTERED;
}

/**
 * Get the Public Identity from the Request URI of the message.
 * 	returns the result in duplicated pkg
 * @param msg - the SIP message
 * @returns the public identity
 */
str cscf_get_public_identity_from_requri(struct sip_msg *msg)
{
	str pu={0,0};
	
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_INFO,"ERR:"M_NAME":cscf_get_public_identity_from_requri: This ain't a request \n");	
		return pu;
	}
	if (parse_sip_msg_uri(msg)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_public_identity_from_requri: Error parsing requesturi \n");	
		return pu;
	}
	if (msg->parsed_uri.user.len) {
		switch (msg->parsed_uri.type) {
			case SIP_URI_T:
				pu.len = 4 + msg->parsed_uri.user.len + 1 + msg->parsed_uri.host.len;
				pu.s = pkg_malloc(pu.len+1);
				sprintf(pu.s,"sip:%.*s@%.*s",
					msg->parsed_uri.user.len,	
					msg->parsed_uri.user.s,	
					msg->parsed_uri.host.len,	
					msg->parsed_uri.host.s);
				break;
			case TEL_URI_T:
				pu.len = 4 + msg->parsed_uri.user.len;
				pu.s = pkg_malloc(pu.len+1);
				sprintf(pu.s,"tel:%.*s",
					msg->parsed_uri.user.len,	
					msg->parsed_uri.user.s);
				break;
			case SIPS_URI_T:
				LOG(L_ERR,"ERR:"M_NAME":cscf_get_public_identity_from_requri: uri type not supported: sips\n");	
				return pu;
			case TELS_URI_T:
				LOG(L_ERR,"ERR:"M_NAME":cscf_get_public_identity_from_requri: uri type not supported: tels\n");	
				return pu;
			default:
				LOG(L_ERR,"ERR:"M_NAME":cscf_get_public_identity_from_requri: uri type not supported: unknown\n");	
				return pu;
		}
			
	}else{
		pu.len = 4 + msg->parsed_uri.host.len;
		pu.s = pkg_malloc(pu.len+1);
		sprintf(pu.s,"sip:%.*s",
			msg->parsed_uri.host.len,	
			msg->parsed_uri.host.s);	
	}
	
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_public_identity_from_requri: <%.*s> \n",
		pu.len,pu.s);	
	return pu;
}

/**
 *	Get public identity from Request-URI for terminating.
 * returns in uri the freshly pkg allocated uri - don't forget to free
 * @param msg - the SIP message
 * @param uri - uri to fill into
 * @returns #IMS_USER_REGISTERED if found, else #IMS_USER_NOT_REGISTERED 
 */
int isc_get_terminating_user( struct sip_msg * msg, str *uri )
{
	*uri = cscf_get_public_identity_from_requri(msg);
	if (!uri->len) return IMS_USER_NOT_REGISTERED;
	/*if ( isc_get_terminating_type( uri ) == IFC_TERMINATING_UNREGISTERED )
		DBG("DBG:"M_NAME":isc_get_terminating_type: To UNREGISTERED %.*s\n", uri->len,uri->s);
	else
		DBG("DBG:"M_NAME":isc_get_terminating_type: To REGISTERED %.*s\n", uri->len,uri->s);	*/
	return IMS_USER_REGISTERED;
}


/**
 *	Get the expires header value from a message. 
 * @param msg - the SIP message
 * @returns the expires value or -1 if not found
 */
int isc_get_expires(struct sip_msg *msg)
{	
	if (msg->expires) {
		if (parse_expires(msg->expires) < 0) {
 			LOG(L_INFO, "INFO:ifc:ifc_get_expires:Error while parsing Expires header\n");
		    return -1;
 		}
		 return ((exp_body_t*) msg->expires->parsed)->val;
 	} else {
		return -1;
	}
}


/**
 * Returns the tm transaction identifiers.
 * If no transaction, then creates one
 * @param msg - the SIP message
 * @param hash - where to write the hash
 * @param label - where to write the label
 * @returns 1 on success and creation of a new transaction, 0 if transaction existed,
 * -1 if failure
 */
int cscf_get_transaction(struct sip_msg *msg, unsigned int *hash,unsigned int *label)
{
	
	if (isc_tmb.t_get_trans_ident(msg,hash,label)<0){	
		LOG(L_DBG,"DBG:"M_NAME":cscf_get_transaction: SIP message without transaction. OK - first request\n");
		if (isc_tmb.t_newtran(msg)<0) 
			LOG(L_INFO,"INF:"M_NAME":cscf_get_transaction: Failed creating SIP transaction\n");
		if (isc_tmb.t_get_trans_ident(msg,hash,label)<0){	
			LOG(L_INFO,"INF:"M_NAME":cscf_get_transaction: SIP message still without transaction\n");
			return -1;
		}else {
			LOG(L_DBG,"DBG:"M_NAME":cscf_get_transaction: New SIP message transaction %u %u\n",
				*hash,*label);
			return 1;
		}						
	}else {
		LOG(L_INFO,"INF:"M_NAME":cscf_get_transaction: Transaction %u %u exists."
		"Retransmission?\n",*hash,*label);
		return 0;
	}
}


/** 
 * Returns the corresponding request for a reply, using tm transactions.
 * @param reply - the SIP Reply message
 * @returns the corresponding SIP Request or NULL if not found
 */
struct sip_msg* cscf_get_request_from_reply(struct sip_msg *reply)
{
	struct cell *t;
	t = isc_tmb.t_gett();
	if (!t || t==(void*) -1){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_request_from_reply: Reply without transaction\n");
		return 0;
	}
	return t->uas.request;
}

/**
 * Returns the expires value from the Expires header in the message.
 * It searches into the Expires header and if not found returns -1
 * @param msg - the SIP message, if available
 * @returns the value of the expire or -1 if not found
 */
int cscf_get_expires_hdr(struct sip_msg *msg) {
	exp_body_t *exp;
	int expires;
	if (!msg) return -1;
	/*first search in Expires header */         
	if (parse_headers(msg,HDR_EXPIRES_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_expires_hdr: Error parsing until header EXPIRES: \n");
		return -1;
	}
	if (msg->expires){
		if (!msg->expires->parsed) {
		        parse_expires(msg->expires);
		}
		if (msg->expires->parsed) {
		        exp = (exp_body_t*) msg->expires->parsed;
		        if (exp->valid) {
		                expires = exp->val;
		                LOG(L_DBG,"DBG:"M_NAME":cscf_get_expires_hdr: <%d> \n",expires);
		                return expires;
		        }
		}
	}
	
	return -1;
}
/**
 * Returns the expires value from the message.
 * First it searches into the Expires header and if not found it also looks 
 * into the expires parameter in the contact header
 * @param msg - the SIP message
 * @returns the value of the expire or the default 3600 if none found
 */
int cscf_get_max_expires(struct sip_msg *msg)
{
	unsigned int exp;
	int max_expires = -1;
	struct hdr_field *h;
	contact_t *c;
	/*first search in Expires header */
	max_expires = cscf_get_expires_hdr(msg);
	
	cscf_parse_contacts(msg);
	for(h=msg->contact;h;h=h->next){
		if (h->type==HDR_CONTACT_T && h->parsed) {
			for(c=((contact_body_t *) h->parsed)->contacts;c;c=c->next){
				if(c->expires){
					if (!str2int(&(c->expires->body), (unsigned int*)&exp) && (int)exp>max_expires) max_expires = exp;
				}
			}
		}	
	}
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_max_expires: <%d> \n",max_expires);
	return max_expires;
}

/**
 * Parses all the contact headers.
 * @param msg - the SIP message
 * @returns the first contact_body
 */
contact_body_t *cscf_parse_contacts(struct sip_msg *msg)
{
	struct hdr_field* ptr;
	if (!msg) return 0;
	
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_parse_contacts: Error parsing headers \n");
		return 0;
	}
	if (msg->contact) {
		ptr = msg->contact;
		while(ptr) {
			if (ptr->type == HDR_CONTACT_T) {
				if (msg->contact->parsed==0){					
					if (parse_contact(ptr)<0){
						LOG(L_ERR,"ERR:"M_NAME":cscf_parse_contacts: error parsing contacts [%.*s]\n",
							ptr->body.len,ptr->body.s); 
					}
				}
			}
			ptr = ptr->next;
		}
	}
	if (!msg->contact) return 0;
	return msg->contact->parsed;
}

/**
 * Returns the first entry of the P-Associated-URI header.
 * @param msg - the SIP message to look into
 * @param public_id - the public identity to be filled with the result
 * @returns 1 on success or 0 on failure
 */
int cscf_get_first_p_associated_uri(struct sip_msg *msg,str *public_id)
{
	struct hdr_field *h;
	rr_t *r;
	public_id->s=0;public_id->len=0;
	
	if (!msg) return 0;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_p_associated_uri: error parsing headers\n");
		return 0;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==16 && strncasecmp(h->name.s,"P-Associated-URI",16)==0)
			break;
		h = h->next;
	}
	if (!h){
		LOG(L_DBG,"DBG:"M_NAME":cscf_get_p_associated_uri: Header P-Associated-URI not found\n");
		return 0;
	}
	if (parse_rr(h)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_p_associated_uri: Error parsing as Route header\n");
		return 0;
	}
	r = (rr_t*)h->parsed;
	h->type = HDR_ROUTE_T;
	
	if (r) {
		*public_id=r->nameaddr.uri;
		return 1;
	}
	else
		return 0;
}

/**
 * Returns the Public Identity extracted from the To header
 * @param msg - the SIP message
 * @returns the str containing the private id, no mem dup
 */
str cscf_get_public_identity(struct sip_msg *msg)
{
	str pu={0,0};
	struct to_body *to;
	
	if (parse_headers(msg,HDR_TO_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_public_identity: Error parsing until header To: \n");
		return pu;
	}
	
	if ( get_to(msg) == NULL ) {
		to = (struct to_body*) pkg_malloc(sizeof(struct to_body));
		parse_to( msg->to->body.s, msg->to->body.s + msg->to->body.len, to );
		msg->to->parsed = to;
	}
		else to=(struct to_body *) msg->to->parsed;
	
	pu = to->uri;
	
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_public_identity: <%.*s> \n",
		pu.len,pu.s);
	return pu;
}

str cscf_p_access_network_info={"P-Access-Network-Info",21};

/**
 * Return the P-Access-Network-Info header
 * @param msg - the SIP message
 * @returns the str with the header's body
 */

str cscf_get_access_network_info(struct sip_msg *msg, struct hdr_field **h)
{
	str ani={0,0};
	struct hdr_field *hdr;
	
	*h=0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_DBG,"DBG:"M_NAME":cscf_get_access_network_info: Error parsing until header EOH: \n");
		return ani;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len==cscf_p_access_network_info.len &&
			strncasecmp(hdr->name.s,cscf_p_access_network_info.s,hdr->name.len)==0)
		{
			*h = hdr;
			ani = hdr->body;
			goto done;
		}                 
		hdr = hdr->next;
	}
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_access_network_info: P-Access-Network-Info header not found \n");
	
done:
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_access_network_info: <%.*s> \n",
		ani.len,ani.s);
	return ani;
}

str cscf_p_visited_network_id={"P-Visited-Network-ID",20};

/**
 * Return the P-Visited-Network-ID header
 * @param msg - the SIP message
 * @returns the str with the header's body
 */

str cscf_get_visited_network_id(struct sip_msg *msg, struct hdr_field **h)
{
	str vnid={0,0};
	struct hdr_field *hdr;
	
	*h=0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_DBG,"DBG:"M_NAME":cscf_get_public_identity: Error parsing until header EOH: \n");
		return vnid;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len==cscf_p_visited_network_id.len &&
			strncasecmp(hdr->name.s,cscf_p_visited_network_id.s,hdr->name.len)==0)
		{
			*h = hdr;
			vnid = hdr->body;
			goto done;
		}
		hdr = hdr->next;
	}
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_visited_network_id: P-Visited-Network-ID header not found \n");

done:
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_visited_network_id: <%.*s> \n",
		vnid.len,vnid.s);
	return vnid;
}

str cscf_p_charging_vector={"P-Charging-Vector",17};

/**
 * Return the P-Charging-Vector header
 * @param msg - the SIP message
 * @returns the str with the header's body
 */

str cscf_get_charging_vector(struct sip_msg *msg, struct hdr_field **h)
{
	str cv={0,0};
	struct hdr_field *hdr;
	
	*h=0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_DBG,"DBG:"M_NAME":cscf_get_charging_vector: Error parsing until header EOH: \n");
		return cv;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len==cscf_p_charging_vector.len &&
			strncasecmp(hdr->name.s,cscf_p_charging_vector.s,hdr->name.len)==0)
		{
			*h = hdr;
			cv = hdr->body;
			goto done;
		}
		hdr = hdr->next;
	}
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_charging_vector: P-Charging-Vector header not found \n");

done:
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_charging_vector: <%.*s> \n",
		cv.len,cv.s);
	return cv;
}

/**
 * Looks for the Call-ID header
 * @param msg - the sip message
 * @param hr - ptr to return the found hdr_field 
 * @returns the callid value
 */
str cscf_get_call_id(struct sip_msg *msg,struct hdr_field **hr)
{
	struct hdr_field *h;
	str call_id={0,0};
	if (hr) *hr = 0;	
	if (!msg) return call_id;
	if (parse_headers(msg, HDR_CALLID_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_call_id: error parsing headers\n");
		return call_id;
	}
	h = msg->callid;
	if (!h){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_call_id: Header Call-ID not found\n");
		return call_id;
	}
	if (hr) *hr = h;
	call_id = h->body;	
	return call_id;
}


/**
 * Looks for the Call-ID header
 * @param msg - the sip message
 * @param hr - ptr to return the found hdr_field 
 * @returns the callid value
 */
int cscf_get_cseq(struct sip_msg *msg,struct hdr_field **hr)
{
	struct hdr_field *h;
	struct cseq_body *cseq;
	int nr = 0,i;
	
	if (hr) *hr = 0;	
	if (!msg) return 0;
	if (parse_headers(msg, HDR_CSEQ_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_cseq: error parsing headers\n");
		return 0;
	}
	h = msg->cseq;
	if (!h){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_cseq: Header CSeq not found\n");
		return 0;
	}
	if (hr) *hr = h;
	if (!h->parsed){
		cseq = pkg_malloc(sizeof(struct cseq_body));
		if (!cseq){
			LOG(L_ERR,"ERR:"M_NAME":cscf_get_cseq: Header CSeq not found\n");
			return 0;
		}
		parse_cseq(h->body.s,h->body.s+h->body.len,cseq);
		h->parsed = cseq;
	}else
		cseq = (struct cseq_body*) h->parsed;		
	for(i=0;i<cseq->number.len;i++)
		nr = (nr*10)+(cseq->number.s[i]-'0');
	return nr;
}
