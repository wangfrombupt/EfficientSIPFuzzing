/*
 * $Id: sip.h 325 2007-06-19 22:47:13Z vingarzan $
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
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 * Copyright (C) 2005 FhG Fokus
 * 		
 */  
#ifndef PIS_CSCF_SIP_H
#define PIS_CSCF_SIP_H

#include "../../sr_module.h"
#include "../../parser/contact/parse_contact.h"
#include "../../parser/parse_rr.h"

int cscf_add_header_first(struct sip_msg *msg, str *hdr,int type);
int cscf_add_header(struct sip_msg *msg, str *hdr,int type);
int cscf_add_header_rpl(struct sip_msg *msg, str *hdr);
int cscf_add_contact(struct sip_msg *msg,str uri,int expires);

str cscf_get_private_identity(struct sip_msg *msg, str realm);
str cscf_get_public_identity(struct sip_msg *msg);
int cscf_get_expires_hdr(struct sip_msg *msg);
int cscf_get_max_expires(struct sip_msg *msg);
str cscf_get_public_identity_from_requri(struct sip_msg *msg);

struct sip_msg *cscf_get_request(unsigned int hash,unsigned int label);

int cscf_get_integrity_protected(struct sip_msg *msg,str realm);
int cscf_get_transaction(struct sip_msg *msg, unsigned  int *hash, unsigned int *label);
int cscf_reply_transactional(struct sip_msg *msg, int code, char *text);
str cscf_get_auts(struct sip_msg *msg, str realm);
str cscf_get_nonce(struct sip_msg *msg, str realm);
str cscf_get_algorithm(struct sip_msg *msg, str realm);
str cscf_get_digest_uri(struct sip_msg *msg, str realm);
int cscf_get_nonce_response(struct sip_msg *msg, str realm,str *nonce,str *response);
str cscf_get_user_agent(struct sip_msg *msg);
contact_body_t *cscf_parse_contacts(struct sip_msg *msg);
str cscf_get_path(struct sip_msg *msg);
str cscf_get_event(struct sip_msg *msg);
str cscf_get_asserted_identity(struct sip_msg *msg);
str cscf_get_asserted_identity_domain(struct sip_msg *msg);
str cscf_get_contact(struct sip_msg *msg);

str cscf_get_first_route(struct sip_msg *msg,struct hdr_field **hr);
int cscf_remove_first_route(struct sip_msg *msg,str value);

int cscf_remove_own_route(struct sip_msg *msg,struct hdr_field **h);

str cscf_get_record_routes(struct sip_msg *msg);

struct hdr_field* cscf_get_next_record_route(struct sip_msg *msg,struct hdr_field *start);

struct hdr_field* cscf_get_next_via_hdr(struct sip_msg *msg,struct hdr_field *start);
str cscf_get_next_via_str(struct sip_msg *msg, struct hdr_field * h, int pos, struct hdr_field **h_out, int *pos_out);

int cscf_via_matching( struct via_body *req_via, struct via_body *rpl_via );
int cscf_str_via_matching(str *sreq_via, str *srpl_via);

str cscf_get_realm_from_ruri(struct sip_msg *msg);

str cscf_get_identity_from_ruri(struct sip_msg *msg);

str cscf_get_call_id(struct sip_msg *msg,struct hdr_field **hr);
int cscf_get_cseq(struct sip_msg *msg,struct hdr_field **hr);
str cscf_get_cseq_method(struct sip_msg *msg,struct hdr_field **hr);

struct sip_msg* cscf_get_request_from_reply(struct sip_msg *reply);

str cscf_get_called_party_id(struct sip_msg *msg,struct hdr_field **hr);

int cscf_get_subscription_state(struct sip_msg *msg);

// from icscf
int cscf_replace_string(struct sip_msg *msg, str orig,str repl);

struct hdr_field* cscf_get_header(struct sip_msg * msg , str header_name);
struct hdr_field* cscf_get_next_header(struct sip_msg * msg ,
						 str header_name,struct hdr_field* last_header);
struct hdr_field* cscf_get_next_header_type(struct sip_msg * msg ,
						 hdr_types_t type, struct hdr_field* last_header);						 

str cscf_get_headers_content(struct sip_msg * msg , str header_name);


// from pcscf

str cscf_get_visited_network_id(struct sip_msg *msg, struct hdr_field **h);

str cscf_get_authorization(struct sip_msg *msg, struct hdr_field **h);
str cscf_get_authenticate(struct sip_msg *msg,struct hdr_field **h);

str cscf_get_session_expires_body(struct sip_msg *msg,struct hdr_field **h);
time_t cscf_get_session_expires(str expHdr, str *refresher);
str cscf_get_min_se(struct sip_msg *msg,struct hdr_field **h);

int cscf_del_header(struct sip_msg *msg,struct hdr_field *h);
int cscf_del_all_headers(struct sip_msg *msg,int hdr_type);

struct via_body* cscf_get_first_via(struct sip_msg *msg, struct hdr_field **h);
struct via_body* cscf_get_last_via(struct sip_msg *msg);
struct via_body* cscf_get_ue_via(struct sip_msg *msg);

str cscf_get_realm(struct sip_msg *msg);
str cscf_get_realm_from_uri(str uri);

int cscf_get_p_associated_uri(struct sip_msg *msg,str **public_id,int *public_id_cnt);
int cscf_get_first_p_associated_uri(struct sip_msg *msg,str *public_id);

name_addr_t cscf_get_preferred_identity(struct sip_msg *msg,struct hdr_field **h);
str cscf_get_called_party_id(struct sip_msg *msg,struct hdr_field **hr);

struct hdr_field* cscf_get_next_route(struct sip_msg *msg,struct hdr_field *start);

int cscf_is_myself(str uri);

str cscf_get_content_type(struct sip_msg *msg);

int cscf_get_content_len(struct sip_msg *msg);

str* cscf_get_service_route(struct sip_msg *msg,int *size);

int cscf_get_originating_contact(struct sip_msg *msg,str *host,int *port,int *transport);
int cscf_get_terminating_contact(struct sip_msg *msg,str *host,int *port,int *transport);

int cscf_get_terminating_identity(struct sip_msg *msg,str *uri);

int cscf_add_p_charging_vector(struct sip_msg *msg);

str cscf_get_last_via_sent_by(struct sip_msg *msg);
str cscf_get_last_via_received(struct sip_msg *msg);

int cscf_get_from_tag(struct sip_msg* msg, str* tag);
int cscf_get_to_tag(struct sip_msg* msg, str* tag);
int cscf_get_from_uri(struct sip_msg* msg,str *local_uri);
int cscf_get_to_uri(struct sip_msg* msg,str *local_uri);
#endif /* PIS_CSCF_SIP_H */
