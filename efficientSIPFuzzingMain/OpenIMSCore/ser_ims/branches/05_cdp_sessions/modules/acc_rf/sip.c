/**
 * $Id: sip.c 439 2007-08-28 10:26:33Z albertoberlios $
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
 * P/I/S-CSCF Module - Main SIP Operations 
 * 
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * \author Alberto Diez - get_from_tag,get_to_tag,get_from_uri added
 * 
 * Copyright (C) 2005 FhG Fokus
 * 		
 */

#include "sip.h"

#include "../../mem/mem.h"
#include "../../data_lump.h"
#include "../../data_lump_rpl.h"
#include "../../parser/parse_to.h"
#include "../../parser/parse_from.h"
#include "../../parser/parse_expires.h"
#include "../../parser/parse_via.h"
#include "../../parser/parse_content.h"
#include "../../parser/parse_nameaddr.h"
#include "../../parser/digest/digest.h"
#include "../../parser/contact/contact.h"
#include "../../parser/contact/parse_contact.h"

#include "../tm/tm_load.h"

#include "mod.h"
#include "auth_api.h"

#define strtotime(src,dest) \
{\
	int i;\
	(dest)=0;\
	for(i=0;i<(src).len;i++)\
		if ((src).s[i]>='0' && (src).s[i]<='9')\
			(dest) = (dest)*10 + (src).s[i] -'0';\
}

#define get_param(src,name,dst) \
{\
	int i,j;\
	(dst).s=0;(dst).len=0;\
	for(i=0;i<(src).len-(name).len;i++)\
		if (strncasecmp((src).s+i,(name).s,(name).len)==0 &&\
			((src).s[i-1]==' ' ||(src).s[i-1]==';'||(src).s[i-1]=='\t')){\
			j=i+(name).len;\
			(dst).s = (src).s+j;\
			(dst).len = 0;\
			while(j<(src).len && (src).s[j]!=','&& (src).s[j]!=' '&& (src).s[j]!='\t'&& (src).s[j]!=';') \
				j++;			\
			(dst).len = j-i-(name).len;\
			break;\
		}		\
}

extern struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/

/**
 * Adds a header to the message as the first one in the message
 * @param msg - the message to add a header to
 * @param content - the str containing the new header
 * @returns 1 on succes, 0 on failure
 */
int cscf_add_header_first(struct sip_msg *msg, str *hdr,int type)
{
	struct hdr_field *first;
	struct lump* anchor,*l;

	first = msg->headers;
	anchor = anchor_lump(msg, first->name.s - msg->buf, 0 , 0 );

	if (anchor == NULL) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_add_header_first: anchor_lump failed\n");
		return 0;
	}

	if (!(l=insert_new_lump_before(anchor, hdr->s,hdr->len,type))){
		LOG(L_ERR, "ERR:"M_NAME":cscf_add_header_first: error creating lump for header\n" );
		return 0;
	}	
 	return 1;
}

/**
 * Adds a header to the message
 * @param msg - the message to add a header to
 * @param content - the str containing the new header
 * @returns 1 on succes, 0 on failure
 */
int cscf_add_header(struct sip_msg *msg, str *hdr,int type)
{
	struct hdr_field *last;
	struct lump* anchor;
	last = msg->headers;
	while(last->next) 
		last = last->next;
	anchor = anchor_lump(msg, last->name.s + last->len - msg->buf, 0 , 0);
	if (anchor == NULL) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_add_header_first: anchor_lump failed\n");
		return 0;
	}

	if (!insert_new_lump_after(anchor, hdr->s,hdr->len,type)){
		LOG(L_ERR, "ERR:"M_NAME":cscf_add_header_first: error creting lump for header\n" );
		return 0;
	}	
 	return 1;
}

/**
 * Adds a header to the reply message
 * @param msg - the request to add a header to its reply
 * @param content - the str containing the new header
 * @returns 1 on succes, 0 on failure
 */
int cscf_add_header_rpl(struct sip_msg *msg, str *hdr)
{
	if (add_lump_rpl( msg, hdr->s, hdr->len, LUMP_RPL_HDR)==0) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_add_header_rpl: Can't add header <%.*s>\n",
			hdr->len,hdr->s);
 		return 0;
 	}
 	return 1;
}



/**
 * Returns the Private Identity extracted from the Authorization header.
 * If none found there takes the SIP URI in To without the "sip:" prefix
 * \todo - remove the fallback case to the To header
 * @param msg - the SIP message
 * @param realm - the realm to match in an Authorization header
 * @returns the str containing the private id, no mem dup
 */
str cscf_get_private_identity(struct sip_msg *msg, str realm)
{
	str pi={0,0};
	struct hdr_field* h=0;
	int ret,i;

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_private_identity: Error parsing until header Authorization: \n");
		return pi;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_private_identity: Message does not contain Authorization header.\n");
		goto fallback;
	}

	ret = find_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_private_identity: Error while looking for credentials.\n");
		goto fallback;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":cscf_get_private_identity: No credentials for this realm found.\n");
			goto fallback;
		}
	
	if (h) pi=((auth_body_t*)h->parsed)->digest.username.whole;

	goto done;
		
fallback:
	LOG(L_INFO,"INF:"M_NAME":cscf_get_private_identity: Falling back to private_id=stripped(public_id)\n"
		"-> Message did not contain a valid Authorization Header!!! This fallback is deprecated outside Early-IMS!\n");	
	pi = cscf_get_public_identity(msg);
	if (pi.len>4&&strncasecmp(pi.s,"sip:",4)==0) {pi.s+=4;pi.len-=4;}
	for(i=0;i<pi.len;i++)
		if (pi.s[i]==';') {
			pi.len=i;
			break;
		}
done:	
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_private_identity: <%.*s> \n",
		pi.len,pi.s);
	return pi;	
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
	int i;
	
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
	
	/* truncate to sip:username@host or tel:number */
	for(i=4;i<pu.len;i++)
		if (pu.s[i]==';' || pu.s[i]=='?' ||pu.s[i]==':'){
			pu.len = i;
		}
	
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_public_identity: <%.*s> \n",
		pu.len,pu.s);	
	return pu;
}


	
/**
 * Returns the expires value from the Expires header in the message.
 * It searches into the Expires header and if not found returns -1
 * @param msg - the SIP message, if available
 * @returns the value of the expire or -1 if not found
 */
int cscf_get_expires_hdr(struct sip_msg *msg)
{
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
 * Get the Public Identity from the Request URI of the message
 * @param msg - the SIP message
 * @returns the public identity
 */
str cscf_get_public_identity_from_requri(struct sip_msg *msg)
{
	str pu={0,0};
	
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_ERR,"INF:"M_NAME":cscf_get_public_identity_from_requri: This ain't a request \n");	
		return pu;
	}
	if (parse_sip_msg_uri(msg)<0){
		LOG(L_ERR,"INF:"M_NAME":cscf_get_public_identity_from_requri: Error parsing requesturi \n");	
		return pu;
	}
	
	if(msg->parsed_uri.type==TEL_URI_T){
		pu.len = 4 + msg->parsed_uri.user.len ;
		pu.s = shm_malloc(pu.len+1);
		sprintf(pu.s,"tel:%.*s",
			msg->parsed_uri.user.len,
			msg->parsed_uri.user.s);
	}else{
		pu.len = 4 + msg->parsed_uri.user.len + 1 + msg->parsed_uri.host.len;
		pu.s = shm_malloc(pu.len+1);
		sprintf(pu.s,"sip:%.*s@%.*s",
			msg->parsed_uri.user.len,	
			msg->parsed_uri.user.s,	
			msg->parsed_uri.host.len,	
			msg->parsed_uri.host.s);
	}	
	
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_public_identity_from_requri: <%.*s> \n",
		pu.len,pu.s);	
	return pu;
}

/**
 * Retrieves the SIP request that generated a diameter transaction
 * @param hash - the tm hash value for this request
 * @param label - the tm label value for this request
 * @returns the SIP request
 */
struct sip_msg *cscf_get_request(unsigned int hash,unsigned int label)
{
	struct cell *c;
	if (tmb.t_lookup_ident(&c,hash,label)<0){
		LOG(L_INFO,"INF:"M_NAME":Cx_UAA_timeout: No transaction found for %u %u\n",
			hash,label);
		return 0;
	}
	return c->uas.request;
}

static str s_ip={"integrity-protected",19};
/**
 * Returns if the SIP message is integrity protected.
 * Looks for the integrity-protected field in the in the Authorization header.
 * \todo - optimize it by including the integrity protected parameter in the
 * digest parsed structures - will change the core though...
 * @param msg - the SIP message
 * @param realm - the realm to match in the authorization
 * @returns 1 if integrity protected is set to yes, 0 if it is not set to yes or is
 * missing
 */
int cscf_get_integrity_protected(struct sip_msg *msg,str realm)
{
	struct hdr_field* h=0;
	int ret,i,j;

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_integrity_protected: Error parsing until header Authorization: \n");
		return 0;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_integrity_protected: Message does not contain Authorization header.\n");
		return 0;
	}

	ret = find_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_integrity_protected: Error while looking for credentials.\n");
		return 0;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":cscf_get_integrity_protected: No credentials for this realm found.\n");
			return 0;
		}
	
	if (h) {
		for(i=0;i<h->body.len-s_ip.len;i++)
			if (strncasecmp(h->body.s+i,s_ip.s,s_ip.len)==0)
				for(j=i+s_ip.len;j<h->body.len;j++){
					if (h->body.s[j]=='y' || h->body.s[j]=='Y') return 1;
					if (h->body.s[j]==' ') return 0;
				}			
	}
	
	return 0;
}


/**
 * Returns the tm transaction identifiers for a message.
 * If no transaction, then creates one
 * @param msg - the SIP message
 * @param hash - where to write the hash
 * @param label - where to write the label
 * @returns 1 on success and creation of a new transaction, 0 if transaction existed,
 * -1 if failure
 */
int cscf_get_transaction(struct sip_msg *msg, unsigned int *hash,unsigned int *label)
{
	
	if (tmb.t_get_trans_ident(msg,hash,label)<0){	
		LOG(L_DBG,"DBG:"M_NAME":cscf_get_transaction: SIP message without transaction. OK - first request\n");
		if (tmb.t_newtran(msg)<0) 
			LOG(L_INFO,"INF:"M_NAME":cscf_get_transaction: Failed creating SIP transaction\n");
		if (tmb.t_get_trans_ident(msg,hash,label)<0){	
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
 * Transactional SIP response - tries to create a transaction if none found.
 * @param msg - message to reply to
 * @param code - the Status-code for the response
 * @param text - the Reason-Phrase for the response
 * @returns the tmb.t_repy() result
 */
int cscf_reply_transactional(struct sip_msg *msg, int code, char *text)
{
	unsigned int hash,label;
	if (tmb.t_get_trans_ident(msg,&hash,&label)<0){	
	
		if (tmb.t_newtran(msg)<0) 
			LOG(L_INFO,"INF:"M_NAME":cscf_get_transaction: Failed creating SIP transaction\n");
	}
	return tmb.t_reply(msg,code,text);
}

/**
 * Looks for the auts parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @returns the auts value or an empty string if not found
 */
str cscf_get_auts(struct sip_msg *msg, str realm)
{
	str name={"auts=\"",6};
	struct hdr_field* h=0;
	int i,ret;
	str auts={0,0};

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_auts: Error parsing until header Authorization: \n");
		return auts;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_auts: Message does not contain Authorization header.\n");
		return auts;
	}

	ret = find_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_auts: Error while looking for credentials.\n");
		return auts;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":cscf_get_auts: No credentials for this realm found.\n");
			return auts;
		}
	
	if (h) {
		for(i=0;i<h->body.len-name.len;i++)
			if (strncasecmp(h->body.s+i,name.s,name.len)==0){
				auts.s = h->body.s+i+name.len;
				while(i+auts.len<h->body.len && auts.s[auts.len]!='\"')
					auts.len++;
			}
	}
	
	return auts;	
}

/**
 * Looks for the nonce parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @returns the nonce or an empty string if none found
 */
str cscf_get_nonce(struct sip_msg *msg, str realm)
{
	struct hdr_field* h=0;
	int ret;
	str nonce={0,0};

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_nonce: Error parsing until header Authorization: \n");
		return nonce;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_nonce: Message does not contain Authorization header.\n");
		return nonce;
	}

	ret = find_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_nonce: Error while looking for credentials.\n");
		return nonce;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":cscf_get_nonce: No credentials for this realm found.\n");
			return nonce;
		}
	
	if (h&&h->parsed) {
		nonce = ((auth_body_t*)h->parsed)->digest.nonce;
	}
	
	return nonce;	
}

/**
 * Looks for the algorithm parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @returns the algorithm or an empty string if not found
 */
str cscf_get_algorithm(struct sip_msg *msg, str realm)
{
	struct hdr_field* h=0;
	int ret;
	str alg={0,0};

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_algorithm: Error parsing until header Authorization: \n");
		return alg;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_algorithm: Message does not contain Authorization header.\n");
		return alg;
	}

	ret = find_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_algorithm: Error while looking for credentials.\n");
		return alg;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":cscf_get_algorithm: No credentials for this realm found.\n");
			return alg;
		}
	
	if (h&&h->parsed) {
		alg = ((auth_body_t*)h->parsed)->digest.alg.alg_str;
	}
	return alg;	
}

/**
 * Looks for the uri parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @returns the uri in the digest
 */
str cscf_get_digest_uri(struct sip_msg *msg, str realm)
{
	struct hdr_field* h=0;
	int ret;
	str uri={0,0};

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_digest_uri: Error parsing until header Authorization: \n");
		return uri;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_digest_uri: Message does not contain Authorization header.\n");
		return uri;
	}

	ret = find_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_digest_uri: Error while looking for credentials.\n");
		return uri;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":cscf_get_digest_uri: No credentials for this realm found.\n");
			return uri;
		}
	
	if (h&&h->parsed) {
		uri = ((auth_body_t*)h->parsed)->digest.uri;
	}
	return uri;	
}

/**
 * Looks for the nonce and response parameters in the Authorization header and returns them
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @param nonce - param to fill with the nonce found
 * @param response - param to fill with the response
 * @returns 1 if found, 0 if not
 */
int cscf_get_nonce_response(struct sip_msg *msg, str realm,str *nonce,str *response)
{
	struct hdr_field* h=0;
	int ret;

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_nonce: Error parsing until header Authorization: \n");
		return 0;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_nonce: Message does not contain Authorization header.\n");
		return 0;
	}

	ret = find_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_nonce: Error while looking for credentials.\n");
		return 0;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":cscf_get_nonce: No credentials for this realm found.\n");
			return 0;
		}
	
	if (h&&h->parsed) {
		*nonce = ((auth_body_t*)h->parsed)->digest.nonce;
		*response = ((auth_body_t*)h->parsed)->digest.response;
	}
	
	return 1;	
}


str ua_dummy={"Unknown UA",10};
str ua_nomsg={"No Message",10};
/**
 * Looks for the User-Agent header and extracts its content.
 * @param msg - the sip message
 * @returns the user agent string, or ua_dummy if not found, or ua_nomsg if msg was NULL
 */
str cscf_get_user_agent(struct sip_msg *msg)
{
	str ua;
	if (!msg) return ua_nomsg;
	ua.len = 0;
	if (parse_headers(msg, HDR_USERAGENT_F, 0) != -1 && msg->user_agent &&
	    msg->user_agent->body.len > 0) 
	{
		ua.len = msg->user_agent->body.len;
		ua.s = msg->user_agent->body.s;
	}
	if (ua.len == 0) ua=ua_dummy;
	return ua;
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
				if (ptr->parsed==0){
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
 * Retrieve a list of concatenated contents of Path headers found in the message
 * @param msg - the SIP message containing the Path headers
 * @returns pkg mem allocated str with the headers values separated by ','
 */
str cscf_get_path(struct sip_msg *msg)
{
	str path={0,0};
	struct hdr_field *h;
	if (!msg) return path;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_path: error parsing headers\n");
		return path;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==4 &&
			strncasecmp(h->name.s,"Path",4)==0){
				path.len+=h->body.len+1;
			}
		h = h->next;
	}
	path.s = pkg_malloc(path.len);
	if (!path.s){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_path: error allocating %d bytes\n",
			path.len);
		path.len=0;
		return path;
	}
	h = msg->headers;
	path.len=0;
	while(h){
		if (h->name.len==4 &&
			strncasecmp(h->name.s,"Path",4)==0){
				if (path.len) path.s[path.len++]=',';
				memcpy(path.s+path.len,h->body.s,h->body.len);
				path.len+=h->body.len;
			}
		h = h->next;
	}

	return path;
}

/**
 * Looks for the Event header and extracts its content.
 * @param msg - the sip message
 * @returns the string event value or an empty string if none found
 */
str cscf_get_event(struct sip_msg *msg)
{
	str e={0,0};
	if (!msg) return e;
	if (parse_headers(msg, HDR_EVENT_F, 0) != -1 && msg->event &&
	    msg->event->body.len > 0) 
	{
		e.len = msg->event->body.len;
		e.s = msg->event->body.s;
	}
	return e;
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
			//LOG(L_CRIT,"%.*s",id.uri.len,id.uri.s);
			return id.uri;
		}
		h = h->next;
	}
	return id.uri;
}


/**
 * Looks for the Contact header and extracts its content
 * @param msg - the sip message
 * @returns the first contact in the message
 */
str cscf_get_contact(struct sip_msg *msg)
{
	str id={0,0};
	struct hdr_field *h;
	struct contact_body *cb;
	
	if (!msg) return id;
	if (parse_headers(msg, HDR_CONTACT_F, 0)<0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_contact: Error parsing headers until Contact.\n");
		return id;
	}

	h = msg->contact;
	if (!h) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_contact: Contact header not found.\n");
		return id;
	}
	if (h->parsed==0 &&
		parse_contact(h)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_contact: Error parsing contacts.\n");
		return id;
	}
	
	cb = (struct contact_body *)h->parsed;
	if (!cb || !cb->contacts){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_contact: No contacts in header.\n");
		return id;
	}
	id = cb->contacts->uri;
	
	return id;
}


/**
 * Looks for the First Route header
 * @param msg - the sip message
 * @param hr - param to return the ptr to the found header
 * @returns the first route string
 */
str cscf_get_first_route(struct sip_msg *msg,struct hdr_field **hr)
{
	struct hdr_field *h;
	rr_t *r;
	str route={0,0};
	if (hr) *hr = 0;	
	if (!msg) return route;
	if (parse_headers(msg, HDR_ROUTE_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_first_route: error parsing headers\n");
		return route;
	}
	h = msg->route;
	if (!h){
		LOG(L_DBG,"DBG:"M_NAME":cscf_get_first_route: Header Route not found\n");
		return route;
	}
	if (hr) *hr = h;
	if (parse_rr(h)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_first_route: Error parsing as Route header\n");
		return route;
	}
	r = (rr_t*)h->parsed;
	route = r->nameaddr.uri;
	
	return route;
}

static str route_hdr_s={"Route: ",7};
static str route_hdr_e={"\r\n",2};
/**
 * Looks if the first entry in the first Route points is equal to the given value and
 * deletes the old header. If there are more entries inserts a new header at top with those values.
 * @param msg - the SIP message
 * @param value - the value to look for
 * @returns 1 if removed, else 0
 */
int cscf_remove_first_route(struct sip_msg *msg,str value)
{
	struct hdr_field *h;
	str route={0,0},x;
	int i;
		
	route = cscf_get_first_route(msg,&h);
	if (!h||!route.len) return 0;
	
	if ((route.len == value.len || (route.len>value.len && route.s[value.len]==';')) &&
		strncasecmp(route.s,value.s,value.len)==0)
	{
		cscf_del_header(msg,h);
		route = h->body;
		i=0;
		while(i<route.len && route.s[i]!=',')
			i++;
		i++;
		if (i<route.len){
			route.s+=i;
			route.len-=i;
			x.s = pkg_malloc(route_hdr_s.len + route.len +route_hdr_e.len);
			if (!x.s){
				LOG(L_ERR, "ERR"M_NAME":cscf_remove_first_route: Error allocating %d bytes\n",
					route.len);
				x.len=0;
			}else{
				x.len = 0;
				STR_APPEND(x,route_hdr_s);
				STR_APPEND(x,route);
				STR_APPEND(x,route_hdr_e);				
				if (!cscf_add_header_first(msg,&x,HDR_ROUTE_T))
					pkg_free(x.s);
			}
		}
	}
	
	return 1;
}



/**
 * Returns if the uri is of myself
 * @param uri as string
 * @returns 1 on success 0 on fail
 */
inline int cscf_is_myself(str uri)
{
	int ret;
	struct sip_uri puri;
	
	if (parse_uri(uri.s,uri.len,&puri)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_is_myself: error parsing uri <%.*s>\n",
			uri.len,uri.s);
		return 0;			
	}
	
	ret = check_self(&(puri.host), puri.port_no ? puri.port_no : SIP_PORT, 0);
	if (ret < 0) return 0;
	
	return ret;
}

/**
 * Looks if the first entry in the first Route points to myself and if positive, it removes it
 * deletes the old header. If there are more entries inserts a new header at top with those values.
 * @param msg - the SIP message
 * @param h - ptr to return the found hdr_field 
 * @returns 1 if removed, else 0
 */
int cscf_remove_own_route(struct sip_msg *msg,struct hdr_field **h)
{
	str route={0,0},x;
	int i;
		
	route = cscf_get_first_route(msg,h);
	if (!h||!route.len) return 0;
	
	LOG(L_DBG,"DBG:"M_NAME":cscf_remove_own_route: <%.*s>\n",
		route.len,route.s);
	if (cscf_is_myself(route))
	{
		cscf_del_header(msg,*h);
		route = (*h)->body;
		i=0;
		while(i<route.len && route.s[i]!=',')
			i++;
		i++;
		if (i<route.len){
			route.s+=i;
			route.len-=i;
			x.s = pkg_malloc(route_hdr_s.len + route.len +route_hdr_e.len);
			if (!x.s){
				LOG(L_ERR, "ERR"M_NAME":cscf_remove_own_route: Error allocating %d bytes\n",
					route.len);
				x.len=0;
			}else{
				x.len = 0;
				STR_APPEND(x,route_hdr_s);
				STR_APPEND(x,route);
				STR_APPEND(x,route_hdr_e);				
				if (!cscf_add_header_first(msg,&x,HDR_ROUTE_T))
					pkg_free(x.s);
			}
		}
	}
	
	return 1;
}


static str s_record_route={"Record-route",12};
/**
 * Returns all the record routes, concatenated.
 * @param msg sip message
 * @returns concatenated routes as String
 */
str cscf_get_record_routes(struct sip_msg *msg)
{
	struct hdr_field *h;
	str route={0,0};
	
	if (!msg) return route;
	
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_record_routes: error parsing headers\n");
		return route;
	}
	h = msg->record_route;
	while (h){
		if (h->name.len == s_record_route.len &&
			strncasecmp(h->name.s,s_record_route.s,s_record_route.len)==0)
		{
			LOG(L_DBG,"DBG:"M_NAME":cscf_get_record_routes: RR %.*s\n",h->body.len,h->body.s);
			if (route.s){
				route.s = pkg_realloc(route.s,route.len+1+h->body.len);
				route.s[route.len++]=',';
			}else{
				route.s = pkg_malloc(h->body.len);			
			}
			memcpy(route.s+route.len,h->body.s,h->body.len);
			route.len+=h->body.len;
		}
		h = h->next;
	}	
	return route;
}

/**
 * Returns the next record route header
 * @param msg - the SIP message
 * @param start - The header to start searching from or NULL if from first header 
 * @returns header field on success or NULL on error 
 */
struct hdr_field* cscf_get_next_record_route(struct sip_msg *msg,struct hdr_field *start)
{
	struct hdr_field *h;
	
	if (!msg) return 0;
	
	
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_next_record_routes: error parsing headers\n");
		return 0;
	}
	if (start) h = start->next;
	else h = msg->record_route;
	while (h){
		if (h->type == HDR_RECORDROUTE_T)
		{
			LOG(L_DBG,"DBG:"M_NAME":cscf_get_next_record_routes: RR %.*s\n",h->body.len,h->body.s);
			if (!h->parsed){
				if (parse_rr(h)<0){
					LOG(L_ERR,"ERR:"M_NAME":cscf_get_next_record_routes: Error parsing as Route header\n");
					return 0;
				}				
			}
			return h;
		}
		h = h->next;
	}	
	return 0;
}

/**
 * Returns the next via header
 * @param msg - the SIP message
 * @param start - The header to start searching from or NULL if from first header 
 * @returns header field on success or NULL on error 
 */
struct hdr_field* cscf_get_next_via_hdr(struct sip_msg *msg,struct hdr_field *start)
{
	struct hdr_field *h;
	
	if (!msg) return 0;
	
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_next_via: error parsing headers\n");
		return 0;
	}
	if (start) h = start->next;
	else h = msg->headers;
	while (h){
		if (h->type == HDR_VIA_T)
			return h;
		h = h->next;
	}	
	return 0;
}

/**
 * trims the str 
 * @param s - str param to trim
 */
static int str_trim(str *s)
{
	int i;
	for (i = 0;i < s->len; i++)
	{
		if (s->s[i] != '\r' && s->s[i] != '\t' && s->s[i] != ' ')
		{
			break;
		}
	}
	s->s = s->s + i;	
	s->len -= i;

	for (i = s->len;i >=0; i--)
	{
		if (s->s[i] == '\r' && s->s[i] == '\t' && s->s[i] == ' ')
		{
			s->len--;
		}
		else
		{
			break;
		}
	}
	return 1;
}

/**
 * Returns the next via str
 * \note in order to check if there are more headers to parse, check param h_out with NULL after obtain a valid via 
 *  str. If you will pass a NULL param for h then first header will be returned again.
 * @param msg - the SIP message
 * @param h - The header to start searching from or NULL if from first header 
 * @param pos - The position in the header to start searching
 * @param h_out - The header to start next searching operations - it will be used for subsequent calls
 * @param pos_out - The position in the header to start next searching operation - it will be used for subsequent calls
 * @returns header field on success or NULL on error 
 */
str cscf_get_next_via_str(struct sip_msg *msg, struct hdr_field * h, int pos, struct hdr_field **h_out, int *pos_out)
{
	char *viab;
	int i, viab_start;
	str via_str={0,0};
	if (!h_out || !pos_out)
		return via_str; 
	
	if (!h)
	{
		h = cscf_get_next_via_hdr(msg,0);
		pos = 0;
	}

	viab = h->body.s;
	i = viab_start = pos;

	while (i < h->body.len)
	{
		if (viab[i] == ',')
		{
			via_str.s = viab + viab_start;
			via_str.len = i - viab_start;
			str_trim(&via_str);
			viab_start = i+1;
			*h_out = h;
			*pos_out = viab_start;
			return via_str;
		}
		i++;
	}
	via_str.s = viab + viab_start;
	via_str.len = i - viab_start;
	str_trim(&via_str);
	*h_out = cscf_get_next_via_hdr(msg,h);
	*pos_out = 0;	
	return via_str;
}

/**
 * cscf_via_matching 
 * @param req_via - first via body (request)
 * @param rpl_via - second via body (reply)
 * @returns true if the 2 via_body structures are matching 
 */
int cscf_via_matching( struct via_body *req_via, struct via_body *rpl_via )
{
	if ((!req_via->branch && rpl_via->branch) || (req_via->branch && !rpl_via->branch))
	{
		LOG(L_INFO,"DBG:"M_NAME":cscf_via_matching: branch param missing\n");
		return 0;;
	}
	if (req_via->branch && rpl_via->branch)
	{
		if (req_via->branch->value.len != rpl_via->branch->value.len ||
			strncasecmp(req_via->branch->value.s, rpl_via->branch->value.s,rpl_via->branch->value.len)!=0)
		{
			LOG(L_INFO,"DBG:"M_NAME":cscf_via_matching: different branch param\n");
			return 0;
		}
	}
	if (req_via->host.len!=rpl_via->host.len||
		strncasecmp(req_via->host.s, rpl_via->host.s,rpl_via->host.len)!=0)
	{
		LOG(L_INFO,"DBG:"M_NAME":cscf_via_matching: different host \n");
		return 0;
	}
	if (req_via->port!=rpl_via->port)
	{
		LOG(L_INFO,"DBG:"M_NAME":cscf_via_matching: different port \n");
		return 0;
	}
	
	if (req_via->transport.len!=rpl_via->transport.len||
		strncasecmp(req_via->transport.s, rpl_via->transport.s,rpl_via->transport.len)!=0)
	{
		LOG(L_INFO,"DBG:"M_NAME":cscf_via_matching: transport host \n");
		return 0;
	}
	/* everything matched -- we found it */
	return 1;
}





static inline void free_via_param_list(struct via_param* vp)
{
	struct via_param* foo;
	while(vp){
		foo=vp;
		vp=vp->next;
		pkg_free(foo);
	}
}

static str via_hdr_term={"\r\n.",3};
/**
 * Checks if 2 via strings are equal. 
 * \note The parameters must contain just one Via header field!!! else you'll have a memory leak
 * @param sreq_via - first via (request)
 * @param srpl_via - second via (reply)
 * @returns 1 if the 2 vias are matching, 0 if not
 */
int cscf_str_via_matching(str *sreq_via, str *srpl_via)
{	
	struct via_body req_via, rpl_via;
	str hdr1={0,0}, hdr2={0,0};
	int result = 0;
	
	memset(&req_via, 0, sizeof(struct via_body));
	memset(&rpl_via, 0, sizeof(struct via_body));

	if (sreq_via->s[sreq_via->len+1] !='\r' || sreq_via->s[sreq_via->len+2] != '\n') //check for CRLF 
	{
		hdr1.len = sreq_via->len + via_hdr_term.len;
		hdr1.s = pkg_malloc(hdr1.len);
		if (!hdr1.s)
		{
			LOG(L_ERR, "ERR:"M_NAME":cscf_str_via_matching: cannot alloc bytes : %d", hdr1.len);
			return 0;
		}
		hdr1.len=0;
		STR_APPEND(hdr1, *sreq_via);
		STR_APPEND(hdr1, via_hdr_term);
		parse_via(hdr1.s,hdr1.s+hdr1.len,&req_via);
	}
	else
		parse_via(sreq_via->s,sreq_via->s+sreq_via->len, &req_via);
	
	
	if (srpl_via->s[srpl_via->len+1] !='\r' || srpl_via->s[srpl_via->len+2] != '\n') //check for CRLF 
	{
		hdr2.len = srpl_via->len + via_hdr_term.len;
		hdr2.s = pkg_malloc(hdr2.len);
		if (!hdr2.s)
		{
			LOG(L_ERR, "ERR:"M_NAME":cscf_str_via_matching: cannot alloc bytes : %d", hdr2.len);
			goto done;
		}
		hdr2.len=0;
		STR_APPEND(hdr2, *srpl_via);
		STR_APPEND(hdr2, via_hdr_term);
		parse_via(hdr2.s,hdr2.s+hdr2.len,&rpl_via);
	}
	else
		parse_via(srpl_via->s, srpl_via->s+srpl_via->len, &rpl_via);
		
	result = cscf_via_matching(&req_via, &rpl_via);
	
done:	
	if (hdr1.s) pkg_free(hdr1.s);
	if (hdr1.s) pkg_free(hdr2.s);
	if (req_via.param_lst) free_via_param_list(req_via.param_lst);
	if (rpl_via.param_lst) free_via_param_list(rpl_via.param_lst);
	return result;
}



/** 
 * Delivers the Realm from request URI
 * @param msg sip message 
 * @returns realm as String on success 0 on fail
 */
str cscf_get_realm_from_ruri(struct sip_msg *msg)
{
	str realm={0,0};
	if (!msg || msg->first_line.type!=SIP_REQUEST){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_realm_from_ruri: This is not a request!!!\n");
		return realm;
	}
	if (!msg->parsed_orig_ruri_ok)
		if (parse_orig_ruri(msg) < 0) 
			return realm;
	
	realm = msg->parsed_orig_ruri.host;
	return realm;	
}

/**
 * Gets identity from the request URI 
 * @param msg sip message 
 * @returns Address of the request as String on success 0 on fail
 */
str cscf_get_identity_from_ruri(struct sip_msg *msg)
{
	str aor={0,0};
	if (!msg || msg->first_line.type!=SIP_REQUEST){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_identity_from_ruri: This is not a request!!!\n");
		return aor;
	}
	aor.s = msg->first_line.u.request.uri.s;
	aor.len = 0;
	while(aor.s[aor.len]!=' '&&aor.s[aor.len]!='\t'&&
		  aor.s[aor.len]!=';'&&aor.s[aor.len]!='&'&&
		  aor.s[aor.len]!='\r'&&aor.s[aor.len]!='\n')
		aor.len++;
//	if (!msg->parsed_orig_ruri_ok)
//		if (parse_orig_ruri(msg) < 0) 
//			return aor;
//			
//	aor = msg->parsed_orig_ruri.host;
//	if (msg->parsed_orig_ruri.user.len){
//		aor.s = msg->parsed_orig_ruri.;
//		aor.len = msg->parsed_orig_ruri.user.len+msg->parsed_orig_ruri.passwd.len+1+
//			msg->parsed_orig_ruri.host.len;
//	}
	return aor;	
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

/** 
 * Returns the corresponding request for a reply, using tm transactions.
 * @param reply - the reply to find request for
 * @returns the transactional request
 */
struct sip_msg* cscf_get_request_from_reply(struct sip_msg *reply)
{
	struct cell *t;
	t = tmb.t_gett();
	if (!t){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_request_from_reply: Reply without transaction\n");
		return 0;
	}
	return t->uas.request;
}


static str s_called_party_id={"P-Called-Party-ID",17};
/**
 * Looks for the P-Preferred-Identity header and extracts its content.
 * @param msg - the sip message
 * @param hr - ptr to return the found hdr_field 
 * @returns the P-Called_Party-ID
 */
str cscf_get_called_party_id(struct sip_msg *msg,struct hdr_field **hr)
{
	str id={0,0};
	struct hdr_field *h;
	if (hr) *hr=0;
	if (!msg) return id;
	if (parse_headers(msg, HDR_EOH_F, 0)<0) {
		return id;
	}
	h = msg->headers;
	while(h)
	{
		if (h->name.len == s_called_party_id.len  &&
			strncasecmp(h->name.s,s_called_party_id.s,s_called_party_id.len)==0)
		{
			id = h->body;
			while(id.len && (id.s[0]==' ' || id.s[0]=='\t' || id.s[0]=='<')){
				id.s = id.s+1;
				id.len --;
			}
			while(id.len && (id.s[id.len-1]==' ' || id.s[id.len-1]=='\t' || id.s[id.len-1]=='>')){
				id.len--;
			}	
			if (hr) *hr = h;
			return id;
		}
		h = h->next;
	}
	return id;
}


static str s_subscription_state={"Subscription-State",18};
static str s_active={"active",6};
static str s_terminated={"terminated",10};
static str s_expires={"expires=",8};
/**
 * Looks for the Subscription-State header and extracts its content
 * @param msg - the sip message
 * @returns expiration if active, -1 if not found, -2 if terminated
 */
int cscf_get_subscription_state(struct sip_msg *msg)
{
	struct hdr_field *h;
	int expires = 0,i;
	str state;
	
	if (!msg) return -1;
	if (parse_headers(msg, HDR_EOH_F, 0)<0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_subscription_state: Error parsing headers until Contact.\n");
		return -1;
	}

	h = msg->headers;
	while(h){
		if (h->name.len == s_subscription_state.len  &&
			strncasecmp(h->name.s,s_subscription_state.s,s_subscription_state.len)==0){
			state = h->body;
			while(state.len && (state.s[0]==' ' || state.s[0]=='\t')){
				state.s = state.s+1;
				state.len --;
			}
			while(state.len && (state.s[state.len-1]==' ' || state.s[state.len-1]=='\t' || state.s[state.len-1]=='>')){
				state.len--;
			}				
			
			if (state.len>=s_terminated.len && strncasecmp(state.s,s_terminated.s,s_terminated.len)==0)
				return 0;
			else if (state.len>=s_active.len && strncasecmp(state.s,s_active.s,s_active.len)==0){
				i=0;
				while(i<state.len-s_expires.len && strncasecmp(state.s+i,s_expires.s,s_expires.len)!=0)
					i++;
				if (i<state.len-s_expires.len){
					i+=s_expires.len;
					while(i<state.len && state.s[i]>='0' && state.s[i]<='9'){
						expires = expires * 10 + state.s[i]-'0';
						i++;
					}			
					return expires;			
				}
				else return -1;/* expires not present, but active */
			}else 
				return -1;/* unrecognizable state */
		}
		h = h->next;
	}		
	LOG(L_ERR,"ERR:"M_NAME":cscf_get_subscription_state: Subscription-state header not found.\n");
	return -1;
}

/**
 * Replace a string in a message with another one.
 * \note the orig string MUST be allocated in the limits of msg->buf.
 * @param msg - the SIP message to modify 
 * @param orig - the string to be replaced
 * @param repl - string to replace with
 * @returns 1 on success 0 on fail (the original string was not in the original SIP message)
 */ 
int cscf_replace_string(struct sip_msg *msg, str orig,str repl)
{
	struct lump* anchor;
	if (orig.s<msg->buf || orig.s > msg->buf + msg->len){
		LOG(L_ERR, "ERR:"M_NAME":cscf_replace_string: original string not inside msg buffer\n");
		return 0;
	}
	anchor = anchor_lump(msg, orig.s - msg->buf, 0 , 0);
	if (anchor == NULL) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_replace_string: anchor_lump failed\n");
		return 0;
	}

	if (!insert_new_lump_after(anchor, repl.s,repl.len,HDR_OTHER_T)){
		LOG(L_ERR, "ERR:"M_NAME":cscf_replace_string: error creating lump for string\n" );
		return 0;
	}	
	if (!del_lump(msg, orig.s - msg->buf, orig.len, 0)) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_replace_string: Can't remove string <%.*s>\n",
			orig.len,orig.s);
		return 0;
	}  
 	return 1;
}

/**
 * Get the content of a certain header.
 * @param msg - the SIP message 
 * @param header_name - the name of the SIP header to return the value for.
 * @returns the list of values in all corresponding headers, pkg_malloced
 */ 
str cscf_get_headers_content(struct sip_msg * msg , str header_name)
{	
	str path={0,0};
	struct hdr_field *h;
	if (!msg) return path;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_path: error parsing headers\n");
		return path;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==header_name.len &&
			strncasecmp(h->name.s,header_name.s,header_name.len)==0){
				path.len+=h->body.len+1;
			}
		h = h->next;
	}
	path.s = pkg_malloc(path.len);
	if (!path.s){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_path: error allocating %d bytes\n",
			path.len);
		path.len=0;
		return path;
	}
	h = msg->headers;
	path.len=0;
	while(h){
		if (h->name.len==header_name.len &&
			strncasecmp(h->name.s,header_name.s,header_name.len)==0){
				if (path.len) path.s[path.len++]=',';
				memcpy(path.s+path.len,h->body.s,h->body.len);
				path.len+=h->body.len;
			}
		h = h->next;
	}

	return path;	
}

/**
 * Returns the first header structure for a given header name. 
 * @param msg - the SIP message to look into
 * @param header_name - the name of the header to search for
 * @returns the hdr_field on success or NULL if not found  
 */
struct hdr_field* cscf_get_header(struct sip_msg * msg , str header_name)
{		
	struct hdr_field *h;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_path: error parsing headers\n");
		return NULL;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==header_name.len &&
			strncasecmp(h->name.s,header_name.s,header_name.len)==0)
				break;
		h = h->next;
	}
	return h;
}

/**
 * Returns the next header structure for a given header name.
 * @param msg - the SIP message to look into
 * @param header_name - the name of the header to search for
 * @param last_header - last header to ignore in the search, or NULL if to start from the first one
 * @returns the hdr_field on success or NULL if not found  
 */
struct hdr_field* cscf_get_next_header(struct sip_msg * msg ,
						 str header_name,struct hdr_field* last_header)
{	
	struct hdr_field *h;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_next_header_field: error parsing headers\n");
		return NULL;
	}
	if (last_header) h = last_header->next;
	else h = msg->headers;
	while(h){
		if (h->name.len==header_name.len &&strncasecmp(h->name.s,header_name.s,header_name.len)==0)
				break;
		h = h->next;
	}
	return h;
}

/**
 * Returns the next header structure for a given hdr_types_t type.
 * @param msg - the SIP message to look into
 * @param type - the type of the header to look for
 * @param last_header - last header to ignore in the search, or NULL if to start from the first one
 * @returns the hdr_field on success or NULL if not found  
 */
struct hdr_field* cscf_get_next_header_type(struct sip_msg * msg ,
						 hdr_types_t type, struct hdr_field* last_header)
{
	struct hdr_field *h;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_next_header_field: error parsing headers\n");
		return NULL;
	}
	if (last_header) h = last_header->next;
	else h = msg->headers;
	while(h){
		if (h->type==type)
				break;
		h = h->next;
	}
	return h;
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
		LOG(L_DBG,"DBG:"M_NAME":cscf_get_visited_network_id: Error parsing until header EOH: \n");
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

/**
 * Looks for the Authorization header and returns its body.
 * @param msg - the SIP message
 * @param h - the hdr_field to fill with the result
 * @returns the authorization body string
 */
str cscf_get_authorization(struct sip_msg *msg,struct hdr_field **h)
{
	str auth={0,0};
	*h = 0;
	auth_body_t *body;
	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_authorization: Error parsing until header Authorization: \n");
		return auth;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_authorization: Message does not contain Authorization header.\n");
		return auth;
	}
	msg->authorization->type = HDR_AUTHORIZATION_T;
	if (msg->authorization->parsed){
		body = (auth_body_t*)msg->authorization->parsed;
		free_credentials(&body);
		msg->authorization->parsed = body;
	}
	
	auth = msg->authorization->body;	
	*h = msg->authorization;
		
	return auth;	
}

/**
 * Looks for the WWW-Authenticate header and returns its body.
 * @param msg - the SIP message
 * @param h - the hdr_field to fill with the result
 * @returns the www-authenticate body
 */
str cscf_get_authenticate(struct sip_msg *msg,struct hdr_field **h)
{
	str auth={0,0};
	struct hdr_field *hdr;
	*h = 0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_authorization: Error parsing until header WWW-Authenticate: \n");
		return auth;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len ==16  &&
			strncasecmp(hdr->name.s,"WWW-Authenticate",16)==0)
		{
			*h = hdr;
			auth = hdr->body;
			break;
		}
		hdr = hdr->next;
	}
	if (!hdr){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_authorization: Message does not contain WWW-Authenticate header.\n");
		return auth;
	}

	return auth;	
}

/**
 * Looks for the Security-Client header header and returns its body.
 * @param msg - the SIP message
 * @param h - the hdr_field to fill with the result
 * @returns the security-client body
 */
str cscf_get_security_client(struct sip_msg *msg,struct hdr_field **h)
{
	str sec_cli={0,0};
	struct hdr_field *hdr;
	*h = 0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_security_client: Error parsing until header Security-Client: \n");
		return sec_cli;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len ==15  &&
			strncasecmp(hdr->name.s,"Security-Client",15)==0)
		{
			*h = hdr;
			sec_cli = hdr->body;
			break;
		}
		hdr = hdr->next;
	}
	if (!hdr){
		LOG(L_DBG, "DBG:"M_NAME":cscf_get_security_client: Message does not contain Security-Client header.\n");
		return sec_cli;
	}

	return sec_cli;	
}

/**
 * Looks for the Security-Verify header header and returns its body.
 * @param msg - the SIP message
 * @param h - the hdr_field to fill with the result
 * @returns the security-verify body
 */
str cscf_get_security_verify(struct sip_msg *msg,struct hdr_field **h)
{
	str sec_vrf={0,0};
	struct hdr_field *hdr;
	*h = 0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_security_verify: Error parsing until header Security-Verify: \n");
		return sec_vrf;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len ==15  &&
			strncasecmp(hdr->name.s,"Security-Verify",15)==0)
		{
			*h = hdr;
			sec_vrf = hdr->body;
			break;
		}
		hdr = hdr->next;
	}
	if (!hdr){
		LOG(L_DBG, "DBG:"M_NAME":cscf_get_security_verify: Message does not contain Security-Verify header.\n");
		return sec_vrf;
	}

	return sec_vrf;	
}



/**
 * Looks for the Session-Expires header and returns its body.
 * @param msg - the SIP message
 * @param h - the hdr_field to fill with the result
 * @returns the security-expire body
 */
str cscf_get_session_expires_body(struct sip_msg *msg,struct hdr_field **h)
{
	str ses_expr={0,0};
	struct hdr_field *hdr;
	*h = 0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_session_expires_body: Error parsing until header Session-Expires: \n");
		return ses_expr;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len ==15  &&
			strncasecmp(hdr->name.s,"Session-Expires",15)==0)
		{
			*h = hdr;
			ses_expr = hdr->body;
			break;
		}
		hdr = hdr->next;
	}
	if (!hdr){
		LOG(L_DBG, "DBG:"M_NAME":cscf_get_session_expires_body: Message does not contain Session-Expires header.\n");
		return ses_expr;
	}

	return ses_expr;	
}


static str s_refresher = {"refresher=", 10};
/**
 * get Session Expires Value .
 * @param expHdr - parsed Session-Expires Header
 * @param refresher - param for returning session refresher
 * @returns Session-Expires value on success or 0
 */
time_t cscf_get_session_expires(str expHdr, str *refresher)
{
	int i;
	time_t exptime;
	int afterExp = 0;
	str exp;
	exp.len = 0;
	exp.s = expHdr.s;
	for (i=0; i < expHdr.len; i++){
		if (expHdr.s[i] != ' ' && expHdr.s[i] != '\t'){
			if (expHdr.s[i] == ';')
				break;
			afterExp = 1;
			exp.len++;
		}
		else {
			if (!afterExp)
				exp.s++;
		}
	}
	if (exp.len == 0)	
		return 0;

	strtotime(exp, exptime);
	get_param(expHdr, s_refresher, *refresher);
	return exptime;
}


/**
 * Looks for the Min-SE header and returns its body.
 * @param msg - the SIP message
 * @param h - the hdr_field to fill with the result
 * @returns the min-se body
 */
str cscf_get_min_se(struct sip_msg *msg,struct hdr_field **h)
{
	str min_se={0,0};
	struct hdr_field *hdr;
	*h = 0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_min_se: Error parsing until header Min-SE: \n");
		return min_se;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len ==6  &&
			strncasecmp(hdr->name.s,"Min-SE",6)==0)
		{
			*h = hdr;
			min_se = hdr->body;
			break;
		}
		hdr = hdr->next;
	}
	if (!hdr){
		LOG(L_DBG, "DBG:"M_NAME":cscf_get_min_se: Message does not contain Min-Se header.\n");
		return min_se;
	}

	return min_se;	
}



/**
 * Deletes the given header.
 * @param msg - the SIP message
 * @param h - the header to delete
 * @returns 1 on success, 0 on error
 */
int cscf_del_header(struct sip_msg *msg,struct hdr_field *h)
{
	if (!h||!h->name.s){
		LOG(L_DBG, "DBG:"M_NAME":cscf_del_header: no header specified.\n");
		return 1;
	}

	if (!del_lump(msg,h->name.s-msg->buf,h->len,0)){
		LOG(L_ERR,"ERR:"M_NAME":cscf_del_header: Error adding del lump\n");
		return 0;		
	}		
	return 1;	
}

/**
 * Deletes all the headers of a given type.
 * @param msg - the SIP message
 * @param h - the header to delete
 * @returns 1 on success, 0 on error
 */
int cscf_del_all_headers(struct sip_msg *msg,int hdr_type)
{
	struct hdr_field *h;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_del_all_headers: Error parsing until last header\n");
		return 0;
	}	
	for(h = msg->headers;h;h=h->next)
		if (h->type == hdr_type)
			if (!cscf_del_header(msg,h)) return 0;
			
	return 1;	
}

/**
 * Looks for the First Via header and returns its body.
 * @param msg - the SIP message
 * @param h - the hdr_field to fill with the result
 * @returns the first via_body
 */
struct via_body* cscf_get_first_via(struct sip_msg *msg,struct hdr_field **h)
{
	if (h) *h = 0;
	
	if (!msg->h_via1 && parse_headers(msg,HDR_VIA_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_first_via: Error parsing until header Via: \n");
		return msg->h_via1->parsed;
	}

	if (!msg->via1){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_first_via: Message does not contain Via header.\n");
		return msg->h_via1->parsed;
	}
		
	return msg->h_via1->parsed;	
}

/**
 * Looks for the Last Via header and returns it.
 * @param msg - the SIP message
 * @returns the last via body body
 */
struct via_body* cscf_get_last_via(struct sip_msg *msg)
{
	struct hdr_field *h=0,*i;
	struct via_body *vb;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_last_via: Error parsing until last header\n");
		return 0;
	}

	i = msg->headers;
	while(i){
		if (i->type == HDR_VIA_T){
			h = i;
		}
		i = i->next;
	}
	if (!h) return 0;
	if (!h->parsed){
		vb = pkg_malloc(sizeof(struct via_body));
		if (!vb){
			LOG(L_ERR,"ERR:"M_NAME":cscf_get_last_via: Error allocating %d bytes\n",sizeof(struct via_body));
			return 0;
		}
		parse_via(h->body.s,h->body.s+h->body.len,vb);
		h->parsed = vb;
	}
	vb = h->parsed;
	while(vb->next)
		vb = vb->next;
	return vb;	
}


/**
 * Looks for the UE Via in First Via header if its a request
 * or in the last if its a response and returns its body
 * @param msg - the SIP message
 * @returns the via of the UE
 */
struct via_body* cscf_get_ue_via(struct sip_msg *msg)
{
	struct via_body *vb=0;
		
	if (msg->first_line.type==SIP_REQUEST) vb = cscf_get_first_via(msg,0);
	else vb = cscf_get_last_via(msg);
	
	if (!vb) return 0;
	
	if (vb->port == 0) vb->port=5060;
	return vb;	
}

static str realm_p={"realm=\"",7};
/**
 * Looks for the realm parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @returns the realm
 */
str cscf_get_realm(struct sip_msg *msg)
{
	str realm={0,0};
	int i,k;

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_realm: Error parsing until header Authorization: \n");
		return realm;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_realm: Message does not contain Authorization header.\n");
		return realm;
	}

	k = msg->authorization->body.len - realm_p.len;
	for(i=0;i<k;i++)
	 if (strncasecmp(msg->authorization->body.s+i,realm_p.s,realm_p.len)==0){
		realm.s = msg->authorization->body.s+ i + realm_p.len;
		i+=realm_p.len;
		while(i<msg->authorization->body.len && msg->authorization->body.s[i]!='\"'){
			i++;
			realm.len++;
		}
		break;
	 }
	
	if (!realm.len){
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_realm: Realm parameter not found.\n");
		return realm;
	}
   	LOG(L_DBG, "DBG:"M_NAME":cscf_get_realm: realm <%.*s>.\n",realm.len,realm.s);
	return realm;	
}


static str phone_context_s={";phone-context=",15};
/**
 * Extracts the realm from a SIP/TEL URI. 
 * - SIP - the hostname
 * - TEL - the phone-context parameter
 * @param msg - the SIP message
 * @returns the realm
 */
str cscf_get_realm_from_uri(str uri)
{
	str realm={0,0};
	int i;
	
	if (uri.len<5) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_realm_from_uri: Error trying to extra realm from too short URI <%.*s>.\n",uri.len,uri.s);
		return realm;
	}
   	//LOG(L_INFO, "DBG:"M_NAME":cscf_get_realm_from_uri: extracting realm from <%.*s>.\n",uri.len,uri.s);
	if (strncasecmp(uri.s,"sip:",4)==0||
		strncasecmp(uri.s,"sips:",5)==0) {
		/* SIP URI */
		realm = uri;
		for(i=0;i<realm.len;i++)
			if (realm.s[i]=='@'){
				realm.s = realm.s + i + 1;
				realm.len = realm.len - i - 1;
				break;
			}
		if (!realm.len) realm = uri;
		for(i=0;i<realm.len;i++)
			if (realm.s[i]==';'||realm.s[i]=='&') {
				realm.len = i;
				break;
			}		
	}else
	if (strncasecmp(uri.s,"tel:",4)==0) {
		/* TEL URI */
		realm = uri;
		while(realm.s[0]!=';' && realm.len>0){
			realm.s++;
			realm.len--;
		}		
		if (realm.len<1) {realm.len=0;return realm;}
		else{
			while(realm.len>phone_context_s.len){
				if (strncasecmp(realm.s,phone_context_s.s,phone_context_s.len)==0){
					realm.s+=phone_context_s.len;
					realm.len-=phone_context_s.len;
					for(i=0;i<realm.len;i++)
						if (realm.s[i]==';' || realm.s[i]=='&'){
							realm.len = i;
							break;
						}
					break;
				}					
				realm.s++;
				realm.len--;
			}
		}		
	}else{
		/* unknown... just extract between @ and ;? */
		realm = uri;
		for(i=0;i<realm.len;i++)
			if (realm.s[i]=='@'){
				realm.s = realm.s + i + 1;
				realm.len = realm.len - i - 1;
				break;
			}
		if (!realm.len) realm = uri;
		for(i=0;i<realm.len;i++)
			if (realm.s[i]==';'||realm.s[i]=='&') {
				realm.len = i;
				break;
			}		
	}

   	LOG(L_DBG, "DBG:"M_NAME":cscf_get_realm_from_uri: realm <%.*s>.\n",realm.len,realm.s);
	return realm;	
}

/**
 * Returns the content of the P-Associated-URI header
 * Public_id is pkg_alloced and should be later freed.
 * Inside values are not duplicated.
 * @param msg - the SIP message to look into
 * @param public_id - array to be allocated and filled with the result
 * @param public_id_cnt - the size of the public_id array
 * @returns 1 on success or 0 on error
 */
int cscf_get_p_associated_uri(struct sip_msg *msg,str **public_id,int *public_id_cnt)
{
	struct hdr_field *h;
	rr_t *r,*r2;
	*public_id = 0;
	*public_id_cnt = 0;
	
	if (!msg) return 0;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_p_associated_uri: error parsing headers\n");
		return 0;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==16 &&
			strncasecmp(h->name.s,"P-Associated-URI",16)==0){
				break;
			}
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
	*public_id_cnt=0;
	r2 = r;
	while(r2){
		(*public_id_cnt) = (*public_id_cnt)+1;
		r2 = r2->next;
	}
	*public_id = pkg_malloc(sizeof(str)*(*public_id_cnt));
	if (!public_id){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_p_associated_uri: Error allocating %d bytes\n",
			 sizeof(str)*(*public_id_cnt));
		return 0;
	}
	r2 = r;
	*public_id_cnt=0;
	while(r2){
		(*public_id)[(*public_id_cnt)]=r2->nameaddr.uri;
		(*public_id_cnt) = (*public_id_cnt)+1;
		r2 = r2->next;
	}
	
	return 1;
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
		if (h->name.len==16 &&
			strncasecmp(h->name.s,"P-Associated-URI",16)==0){
				break;
			}
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
	}else
		return 0;
}

static str s_preferred_id={"P-Preferred-Identity",20};
/**
 * Looks for the P-Preferred-Identity header and extracts its content
 * @param msg - the SIP message to look into
 * @param hr - the header ptr to be filled with the result
 * @returns the preferred identity string or an empty string if none found
 */
name_addr_t cscf_get_preferred_identity(struct sip_msg *msg,struct hdr_field **hr)
{
	name_addr_t id;
	struct hdr_field *h;
	rr_t *r;
	
	*hr=0;
	memset(&id,0,sizeof(name_addr_t));
	if (!msg) return id;
	if (parse_headers(msg, HDR_EOH_F, 0)<0) {
		return id;
	}
	h = msg->headers;
	while(h)
	{
		if (h->name.len == s_preferred_id.len  &&
			strncasecmp(h->name.s,s_preferred_id.s,s_preferred_id.len)==0)
		{
			if (parse_rr(h)<0){
				//This might be an old client
				LOG(L_CRIT,"WARN:"M_NAME":cscf_get_preferred_identity: P-Preferred-Identity header must contain a Nameaddr!!! Fix the client!\n");
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
				if (hr) *hr = h;			
				return id;	
			}
			r = (rr_t*) h->parsed;
			id = r->nameaddr; 
			free_rr(&r);
			h->parsed=r;
			if (hr) *hr = h;			
			return id;
		}
		h = h->next;
	}
	return id;
}


/**
 * Returns the next route.
 * @param msg - the sip message
 * @param start - where to start look for, ignoring itself
 * @returns the the next route header or NULL if no more found
 */
struct hdr_field* cscf_get_next_route(struct sip_msg *msg,struct hdr_field *start)
{
	struct hdr_field *h;
	
	if (!msg) return 0;
	
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_next_route: error parsing headers\n");
		return 0;
	}
	if (start) h = start->next;
	else h = msg->route;
	while (h){
		if (h->type == HDR_ROUTE_T)
		{
			LOG(L_DBG,"DBG:"M_NAME":cscf_get_next_route: Route %.*s\n",h->body.len,h->body.s);
			if (!h->parsed){
				if (parse_rr(h)<0){
					LOG(L_ERR,"ERR:"M_NAME":cscf_get_next_route: Error parsing as Route header\n");
					return 0;
				}				
			}
			return h;
		}
		h = h->next;
	}	
	return 0;
}


/**
 * Looks for the Content-Type header and extracts its content.
 * @param msg - the sip message
 * @returns the content-type string, or an empty string if not found
 */
str cscf_get_content_type(struct sip_msg *msg)
{
	str ct={0,0};
	if (!msg) return ct;
	if (parse_headers(msg, HDR_CONTENTTYPE_F, 0) != -1 && msg->content_type)
		ct = msg->content_type->body;		
	return ct;
}

/**
 * Looks for the Content-length header and extracts its content
 * @param msg - the sip message
 * @returns the content length or 0 if not found
 */
int cscf_get_content_len(struct sip_msg *msg)
{
	int cl=0;
	if (!msg) return 0;
	if (parse_headers(msg, HDR_CONTENTLENGTH_F, 0) != -1 && msg->content_length &&
			msg->content_length->parsed)
		cl = get_content_length(msg);		
	return cl;
}

/**
 * Returns the content of the Service-Route header.
 * data vector is pkg_alloced and should be later freed
 * inside values are not duplicated
 * @param msg - the SIP message
 * @param size - size of the returned vector, filled with the result
 * @returns - the str vector of uris
 */
str* cscf_get_service_route(struct sip_msg *msg,int *size)
{
	struct hdr_field *h;
	rr_t *r,*r2;
	str *x = 0;
	int k;
	if (!size) return 0;
	
	*size=0;
		
	if (!msg) return 0;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_service_route: error parsing headers\n");
		return 0;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==13 &&
			strncasecmp(h->name.s,"Service-Route",13)==0)
		{
			if (parse_rr(h)<0){
				LOG(L_ERR,"ERR:"M_NAME":cscf_get_service_route: Error parsing as Route header\n");
				continue;
			}
			r = (rr_t*)h->parsed;
			h->type = HDR_ROUTE_T;
			r2 = r; k=0;
			while(r2){
				k++;
				r2 = r2->next;
			}
			if (!k){
				LOG(L_ERR,"ERR:"M_NAME":cscf_get_service_route: No items in this Service-Route\n");
				continue;
			}
			x = pkg_realloc(x,(*size+k)*sizeof(str));
			if (!x){
				LOG(L_ERR,"ERR:"M_NAME":cscf_get_service_route: Error reallocating to %d bytes\n",
					(*size+k)*sizeof(str));
				return 0;
			}
			r2 = r; 
			while(r2){
				x[*size] = r2->nameaddr.uri;
				(*size) = (*size)+1;
				r2 = r2->next;
			}			
		}
		h = h->next;
	}
	
	return x;
}


/**
 * Returns the originating contact.
 * @param msg - the SIP message to look into
 * @param host - the host string to be filled with the result
 * @param port - the port number to be filled with the result 
 * @param transport - the transport type to be filled with the result
 * @returns 1 on success
 */
int cscf_get_originating_contact(struct sip_msg *msg,str *host,int *port,int *transport)
{
	struct via_body *vb;
	
	vb = cscf_get_ue_via(msg);

	*host = vb->host;
	*port = vb->port;
	*transport = vb->proto;
	LOG(L_INFO,"DBG:"M_NAME":cscf_get_originating_contact: %d://%.*s:%d \n",*transport,host->len,host->s,*port);	
	return 1;
}


/**
 * Returns the terminating contact.
 * @param msg sip message
 * @param msg - the SIP message to look into
 * @param host - the host string to be filled with the result
 * @param port - the port number to be filled with the result 
 * @param transport - the transport type to be filled with the result
 * @returns 1 on success
 */
int cscf_get_terminating_contact(struct sip_msg *msg,str *host,int *port,int *transport)
{
	struct sip_msg *req;	
	req = msg;	
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_terminating_contact: NULL message!!!\n");
		return 0;
	}
 	if (req->first_line.type!=SIP_REQUEST){
 		req = cscf_get_request_from_reply(req);
 	}
	
	if (!req->parsed_orig_ruri_ok)
		if (parse_orig_ruri(req) < 0) 
			return 0;
	*host = req->parsed_orig_ruri.host;	
	*port = req->parsed_orig_ruri.port_no;
	*transport = req->parsed_orig_ruri.proto;
	LOG(L_INFO,"DBG:"M_NAME":cscf_get_terminating_contact: %d://%.*s:%d \n",*transport,host->len,host->s,*port);	
	return 1;
}

/**
 * Returns the terminating contact.
 * @param msg sip message
 * @param msg - the SIP message to look into
 * @param host - the host string to be filled with the result
 * @param port - the port number to be filled with the result 
 * @param transport - the transport type to be filled with the result
 * @returns 1 on success
 */
int cscf_get_terminating_identity(struct sip_msg *msg,str *uri)
{
	struct sip_msg *req;	
	int i;
	req = msg;	
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_terminating_identity: NULL message!!!\n");
		return 0;
	}
 	if (req->first_line.type!=SIP_REQUEST){
 		req = cscf_get_request_from_reply(req);
 	}
	
	if (msg->new_uri.s) *uri = msg->new_uri;
	else *uri = msg->first_line.u.request.uri;
		
	for(i=0;i<uri->len;i++)
		if (uri->s[i]==';' || uri->s[i]=='?') {
			uri->len = i;
			break;
		}
	
	LOG(L_INFO,"DBG:"M_NAME":cscf_get_terminating_identity: <%.*s> \n",uri->len,uri->s);	
	return 1;
}


char* cscf_icid_value_prefix="abcd";		/**< hexadecimal prefix for the icid-value - must be unique on each node */
unsigned int* cscf_icid_value_count=0;		/**< to keep the number of generated icid-values 	*/
gen_lock_t* cscf_icid_value_count_lock=0;	/**< to lock acces on the above counter				*/
char* cscf_icid_gen_addr="127.0.0.1";		/**< address of the generator of the icid-value 	*/
char* cscf_orig_ioi="open-ims.test";		/**< name of the Originating network 				*/
char* cscf_term_ioi="open-ims.test";		/**< name of the Terminating network 				*/

str cscf_icid_value_prefix_str;				/**< fixed hexadecimal prefix for the icid-value - must be unique on each node */
str cscf_icid_gen_addr_str;					/**< fixed address of the generator of the icid-value */
str cscf_orig_ioi_str;						/**< fixed name of the Originating network 			*/
str cscf_term_ioi_str;						/**< fixed name of the Terminating network 			*/

static str p_charging_vector_s={"P-Charging-Vector: icid-value=\"",31};
static str p_charging_vector_1={"\"; icid-generated-at=\"",22};
static str p_charging_vector_2={"\"; orig-ioi=\"",13};
static str p_charging_vector_e={"\"\r\n",3};
static char hex_chars[16]="0123456789abcdef";
/**
 * Inserts the P-Charging-Vector header
 * P-Charging-Vector:
 * @param msg - the SIP message to add to
 * @returns #CSCF_RETURN_TRUE if ok or #CSCF_RETURN_FALSE on error
 */
int cscf_add_p_charging_vector(struct sip_msg *msg)
{
	int r = CSCF_RETURN_FALSE;
	str x={0,0};
	int i;
	time_t t;
	unsigned int cnt;
	
	x.len = p_charging_vector_s.len+
		cscf_icid_value_prefix_str.len+
		sizeof(time_t)*2+
		sizeof(unsigned int)*2+
		p_charging_vector_1.len+
		cscf_icid_gen_addr_str.len+
		p_charging_vector_2.len+		
		cscf_orig_ioi_str.len+
		p_charging_vector_e.len;
	x.s = pkg_malloc(x.len);
	if (!x.s){
		LOG(L_ERR, "ERR"M_NAME":cscf_add_p_charging_vector: Error allocating %d bytes\n",
			x.len);
		x.len=0;
		goto error;		
	}
	x.len=0;
	STR_APPEND(x,p_charging_vector_s);
	STR_APPEND(x,cscf_icid_value_prefix_str);
	
	time(&t);
	LOG(L_DBG,"DBG:"M_NAME":cscf_add_p_charging_vector: time is %ud\n",(unsigned int)t);
	for(i=sizeof(time_t)*2-1;i>=0;i--){
		x.s[x.len+i]= hex_chars[t & 0x0F];
		t >>= 4;
	}
	x.len+=sizeof(time_t)*2;

	lock_get(cscf_icid_value_count_lock);
	 cnt = *cscf_icid_value_count;
	 *cscf_icid_value_count = cnt+1;
	lock_release(cscf_icid_value_count_lock);
	LOG(L_DBG,"DBG:"M_NAME":cscf_add_p_charging_vector: count is %ud\n",cnt);
	for(i=sizeof(unsigned int)*2-1;i>=0;i--){
		x.s[x.len+i]= hex_chars[cnt & 0x0F];
		cnt >>= 4;
	}
	x.len+=sizeof(unsigned int)*2;
	
	STR_APPEND(x,p_charging_vector_1);
	STR_APPEND(x,cscf_icid_gen_addr_str);
	STR_APPEND(x,p_charging_vector_2);
	STR_APPEND(x,cscf_orig_ioi_str);
	STR_APPEND(x,p_charging_vector_e);
	
	if (cscf_add_header(msg,&x,HDR_OTHER_T)) r = CSCF_RETURN_TRUE;
	else goto error;

	return r;
error:
	r = CSCF_RETURN_ERROR;
	if (x.s) pkg_free(x.s);
	return r;
}


/**
 * Get the sent-by parameter of the last Via header in the message.
 * @param msg - the SIP message to loog into
 * @returns the sent-by value string or an empty string if not found
 */
str cscf_get_last_via_sent_by(struct sip_msg *msg)
{
	struct via_body *via;
	
	via = cscf_get_last_via(msg);
	if (!via){
		str zero={0,0};
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_last_via_sent_by(): Message has no via header!\n");		
		return zero;
	}			
	return via->host;
}


/**
 * Get the received parameter of the last Via header in the message.
 * @param msg - the SIP message to loog into
 * @returns the sent-by value string or an empty string if not found
 */
str cscf_get_last_via_received(struct sip_msg *msg)
{
	struct via_body *via;	
	str received={0,0};

	via = cscf_get_last_via(msg);
	if (!via){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_last_via_received(): Message has no via header!\n");
		return received;
	}			
	if (via->received) return via->received->value;
	return received;			
}


/**
 * Get the from tag
 * @param msg - the SIP message to look into
 * @param tag - the pointer to the tag to write to
 * @returns 0 on error or 1 on success
 */
int cscf_get_from_tag(struct sip_msg* msg, str* tag)
{
	struct to_body* from;
	
	if (!msg || parse_from_header(msg)<0||!msg->from||!msg->from->parsed){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_from_tag: error parsing From header\n");
		if (tag) {tag->s = 0;tag->len = 0;}
		return 0;
	}
	from = msg->from->parsed;	
	if (tag) *tag = from->tag_value;	
	return 1;	
}

/**
 * Get the to tag
 * @param msg  - the SIP Message to look into
 * @param tag - the pointer to the tag to write to
 * @returns 0 on error or 1 on success
 */
int cscf_get_to_tag(struct sip_msg* msg, str* tag)
{	
	if (!msg || !msg->to) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_get_to_tag(): To header field missing\n");
		if (tag) {tag->s = 0;tag->len = 0;}
		return 0;
	}

	if (tag) *tag = get_to(msg)->tag_value;		
	return 1;
 }
 
 
/**
 * Get the local uri from the From header.
 * @param msg - the message to look into
 * @param local_uri - ptr to fill with the value
 * @returns 1 on success or 0 on error
 */  
int cscf_get_from_uri(struct sip_msg* msg,str *local_uri)
{	
	struct to_body* from;

	if (!msg || parse_from_header(msg)<0 || !msg->from || !msg->from->parsed){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_from_uri: error parsing From header\n");
		if (local_uri) {local_uri->s = 0;local_uri->len = 0;}
		return 0;
	}
	from = msg->from->parsed;		
	if (local_uri) *local_uri = from->uri;
	return 1;
	
}



/**
 * Get the local uri from the To header.
 * @param msg - the message to look into
 * @param local_uri - ptr to fill with the value
 * @returns 1 on success or 0 on error
 */  
int cscf_get_to_uri(struct sip_msg* msg,str *local_uri)
{	
	struct to_body* to=	NULL;

	if (!msg || !msg->to || !msg->to->parsed || parse_headers(msg,HDR_TO_F,0)==-1 ){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_to_uri: error parsing TO header\n");
		if (local_uri) {local_uri->s = 0;local_uri->len = 0;}
		return 0;
	}
	to = msg->to->parsed;		
	if (local_uri) *local_uri = to->uri;
	return 1;
	
}



/*
 *******************************************************************************
 * Following functions used for message offline charging 
 *******************************************************************************
 */

/**
 * Gets SIP method from a SIP request.
 *
 * @param msg - the SIP message
 * @returns method if msg is a request.
 */
str cscf_get_sip_method(struct sip_msg *msg)
{
	str method={0,0};
	
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_ERR,"INF:"M_NAME":cscf_get_sip_method: This is not a request.\n");
	} else {
		method = msg->first_line.u.request.method;
	}
	
	return method;
}



static str p_charging_vector={"P-Charging-Vector",17};
static str p_charging_vector_icid={"icid-value=\"",12};
static str p_charging_vector_orig_ioi={"orig-ioi=\"",10};
static str p_charging_vector_term_ioi={"term-ioi=\"",10};

/* Get parameter from P-Charging-Vector header
 * msg - SIP message
 * param - param name 
 * return parameter found
 */
str cscf_get_p_charging_vector_param(struct sip_msg* msg, str* param)
{
	str value = {0,0};
	struct hdr_field* h = NULL;
	
	h = cscf_get_header(msg, p_charging_vector);
	if(!h) return value;
	
	int i,k;
	
	k = h->body.len - param->len;
	for(i=0;i<k;i++)
	 if (strncasecmp(h->body.s+i,param->s,
	 		param->len)==0){
		value.s = h->body.s+ i + param->len;
		i+=param->len;
		while(i<h->body.len && h->body.s[i]!='\"'){
			i++;
			value.len++;
		}
		break;
	 }
	
	if (!value.len){
		LOG(L_DBG, "ERR:"M_NAME":cscf_get_p_charging_vector: \
			parameter %.*s not found.\n", param->len, param->s);
		return value;
	}
   	LOG(L_DBG, "DBG:"M_NAME":cscf_get_p_charging_vector: parameter: %.*s.\n",value.len,value.s);
	return value;	
}


str cscf_get_p_charging_vector_icid(struct sip_msg* msg)
{
	return cscf_get_p_charging_vector_param(msg, &p_charging_vector_icid);
}



str cscf_get_p_charging_vector_orig_ioi(struct sip_msg* msg)
{
	return cscf_get_p_charging_vector_param(msg, &p_charging_vector_orig_ioi);
}



str cscf_get_p_charging_vector_term_ioi(struct sip_msg* msg)
{
	return cscf_get_p_charging_vector_param(msg, &p_charging_vector_term_ioi);
}



static str p_access_network_info={"P-Access-Network-Info",21};



/* Get P-Access-Network-Info header
 * msg - SIP message
 * param - param name 
 * return parameter found
 */
str cscf_get_p_access_network_info(struct sip_msg* msg)
{
	str value = {0,0};
	struct hdr_field* h = NULL;
	
	h = cscf_get_header(msg, p_access_network_info);
	if(!h) 
		return value;
	else 
		return h->body;
}
