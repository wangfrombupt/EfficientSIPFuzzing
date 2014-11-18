/*
 * $Id: registration.c 430 2007-08-01 13:18:42Z vingarzan $
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
 * Proxy-CSCF -Registration Related Operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include <time.h>
 
#include "registration.h"

#include "../../data_lump.h"
#include "../../mem/mem.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "mod.h"
#include "sip.h"
#include "registrar.h"
#include "registrar_subscribe.h"
#include "ims_pm_pcscf.h"

extern struct tm_binds tmb;            				/**< Structure with pointers to tm funcs 			*/

extern str pcscf_name_str;							/**< fixed SIP URI of this P-CSCF 					*/
extern str pcscf_path_hdr_str;						/**< fixed Path header 								*/
extern str pcscf_path_str;							/**< fixed Path URI  								*/
extern str pcscf_icid_value_prefix_str;				/**< fixed hexadecimal prefix for the icid-value - must be unique on each node */
extern str pcscf_icid_gen_addr_str;					/**< fixed address of the generator of the icid-value */
extern str pcscf_orig_ioi_str;						/**< fixed name of the Originating network 			*/
extern str pcscf_term_ioi_str;						/**< fixed name of the Terminating network 			*/
extern unsigned int* pcscf_icid_value_count;		/**< to keep the number of generated icid-values 	*/
extern gen_lock_t* pcscf_icid_value_count_lock;		/**< to lock acces on the above counter				*/

extern r_hash_slot *registrar;						/**< the contacts 									*/
extern int r_hash_size;								/**< records tables parameters 						*/


extern int pcscf_nat_enable;	 					/**< whether to enable NAT							*/
extern int pcscf_use_ipsec;							/**< whether to use or not ipsec 					*/
extern int pcscf_use_tls;							/**< whether to use or not TLS 					*/
extern int pcscf_tls_port;						/**< PORT for TLS server 						*/

/**
 * Inserts the Path header.
 * Path: <sip:term@pcscf.name.com;lr>
 * @param msg - the SIP message to add to
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok or #CSCF_RETURN_FALSE on error
 */
int P_add_path(struct sip_msg *msg,char *str1,char*str2)
{
	str x={0,0};

	STR_PKG_DUP(x,pcscf_path_hdr_str,"pkg");
	if (!x.s) return CSCF_RETURN_ERROR;
	if (cscf_add_header(msg,&x,HDR_OTHER_T)) return CSCF_RETURN_TRUE;
	else {
		pkg_free(x.s);
		return CSCF_RETURN_ERROR;
	}
out_of_memory:
	return CSCF_RETURN_ERROR;	
}


static str require_hdr={"Require: path\r\n",15};
/**
 * Inserts the Require header.
 * Require: path 
 * @param msg - the SIP message to add to
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok or #CSCF_RETURN_FALSE on error
 */
int P_add_require(struct sip_msg *msg,char *str1,char*str2)
{
	str x={0,0};

	STR_PKG_DUP(x,require_hdr,"pkg");
	if (!x.s) return CSCF_RETURN_ERROR;
	if (cscf_add_header(msg,&x,HDR_OTHER_T)) return CSCF_RETURN_TRUE;
	else {
		pkg_free(x.s);
		return CSCF_RETURN_ERROR;
	}
out_of_memory:
	return CSCF_RETURN_ERROR;	
}


/**
 * Inserts the P-Charging-Vector header
 * P-Charging-Vector:
 * @param msg - the SIP message to add to
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok or #CSCF_RETURN_FALSE on error
 */
int P_add_p_charging_vector(struct sip_msg *msg,char *str1,char*str2)
{
	return cscf_add_p_charging_vector(msg);
}


/**
 * Finds out if the message was received over a protected IPSec channel.
 * @param msg - the SIP to check
 * @param str1 - the realm to look into
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if no auth header found or #CSCF_RETURN_FALSE on error
 */
int P_is_integrity_protected(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	struct via_body *vb;
	unsigned long s_hash = 0;


	LOG(L_DBG,"DBG:"M_NAME":P_is_integrity_protected: Looking if registered\n");
//	print_r(L_INFO);
	
	vb = cscf_get_ue_via(msg);
	
	LOG(L_DBG,"DBG:"M_NAME":P_is_integrity_protected: Looking for <%d://%.*s:%d,%d>\n",
		vb->proto,vb->host.len,vb->host.s,vb->port,msg->rcv.src_port);
	if (pcscf_use_tls && msg->rcv.dst_port == pcscf_tls_port){
		s_hash = get_tls_session_hash(msg);
		if (!s_hash){
			LOG(L_ERR,"ERR:"M_NAME":P_is_integrity_protected: Session Hash could not be obtained !\n");
			return CSCF_RETURN_FALSE;
		}
	}
	if (r_is_integrity_protected(vb->host,vb->port,msg->rcv.src_port,vb->proto, s_hash)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;	
	
	return ret;
}


static str authorization_s={"Authorization: ",15}; 
static str authorization_e={"\r\n",2};
static str integrity_protected_s={", integrity-protected=\"",23};
static str integrity_protected_e={"\"",1};
/**
 * Inserts the integrity protected parameter into the Authorization header.
 * @param msg - the SIP message to add to
 * @param str1 - the value to insert into integrity protected: ["yes","no"]
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if no auth header found or #CSCF_RETURN_FALSE on error
 */
int P_add_integrity_protected(struct sip_msg *msg,char *str1,char*str2)
{
	int r = CSCF_RETURN_FALSE;
	str x={0,0};
	str v={0,0};
	str auth;
	int i;
	int s=0,e=0;
	struct hdr_field *hdr;	
	
	v.s = str1;
	v.len = strlen(str1);
	
	auth = cscf_get_authorization(msg,&hdr);
	if (!auth.len){
		LOG(L_INFO,"INF:"M_NAME":P_add_integrity_protected: No authorization header found.\n");
		goto done; 
	}
	
	s=auth.len;e=auth.len;
	/* first we look for it */
	for(i=0;i<auth.len-integrity_protected_s.len;i++)
		if (strncasecmp(auth.s+i,integrity_protected_s.s,integrity_protected_s.len)==0){
			s = i;
			e = i+integrity_protected_s.len;
			while(e<auth.len && auth.s[e]!='\"') 
				e++;
			if (e<auth.len) e++;
			break;			
		}
	x.len = authorization_s.len+
			auth.len - (e-s)+
			integrity_protected_s.len+v.len+integrity_protected_e.len+
			authorization_e.len;
	x.s = pkg_malloc(x.len);
	if (!x.s){
		LOG(L_ERR, "ERR"M_NAME":P_add_integrity_protected: Error allocating %d bytes\n",
			x.len);
		x.len=0;
		goto error;		
	}
	x.len=0;
	
	STR_APPEND(x,authorization_s);
	memcpy(x.s+x.len,auth.s,s);
	x.len += s;
	STR_APPEND(x,integrity_protected_s);
	STR_APPEND(x,v);
	STR_APPEND(x,integrity_protected_e);
	memcpy(x.s+x.len,auth.s+e,auth.len-e);
	x.len += auth.len-e;
	STR_APPEND(x,authorization_e);

	if (cscf_add_header(msg,&x,HDR_OTHER_T)) r = CSCF_RETURN_TRUE;
	else goto error;


	if (!cscf_del_header(msg,hdr)){
		LOG(L_INFO,"INF:"M_NAME":P_add_integrity_protected: Error dropping old authorization header.\n");		
		goto error; 
	}

done:
	return r;
error:
	r = CSCF_RETURN_ERROR;
	if (x.s) pkg_free(x.s);
	return r;
}



static str p_visited_network_id_s={"P-Visited-Network-ID: ",22};
static str p_visited_network_id_1={", ",2};
static str p_visited_network_id_e={"\r\n",2};
/**
 * Inserts the P-Visited-Network-ID header or a field inside if it exists.
 * @param msg - the SIP message to add to
 * @param str1 - the value to insert - !!! quoted if needed
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if okor #CSCF_RETURN_FALSE on error
 */
int P_add_p_visited_network_id(struct sip_msg *msg,char *str1,char*str2)
{
	int r = CSCF_RETURN_FALSE;
	str x={0,0};
	str v={0,0};
	str old={0,0};
	struct hdr_field *hdr;
	
	v.s = str1;
	v.len = strlen(str1);
	
	old = cscf_get_visited_network_id(msg,&hdr);
	
	x.len = p_visited_network_id_s.len + old.len + v.len + p_visited_network_id_e.len;
	if (old.len) x.len+=p_visited_network_id_1.len;
	
	x.s = pkg_malloc(x.len);
	if (!x.s){
		LOG(L_ERR, "ERR"M_NAME":P_add_p_visited_network_id: Error allocating %d bytes\n",
			require_hdr.len);
		x.len=0;
		goto error;		
	}
	x.len=0;
	memcpy(x.s+x.len,p_visited_network_id_s.s,p_visited_network_id_s.len);
	x.len += p_visited_network_id_s.len;
	
	if (old.len) {
		memcpy(x.s+x.len,old.s,old.len);
		x.len += old.len;
		
		memcpy(x.s+x.len,p_visited_network_id_1.s,p_visited_network_id_1.len);
		x.len += p_visited_network_id_1.len;
	}
	
	memcpy(x.s+x.len,v.s,v.len);
	x.len += v.len;

	memcpy(x.s+x.len,p_visited_network_id_e.s,p_visited_network_id_e.len);
	x.len += p_visited_network_id_e.len;

		
	if (cscf_add_header(msg,&x,HDR_OTHER_T)) r = CSCF_RETURN_TRUE;
	else goto error;

	if (!cscf_del_header(msg,hdr)){
		LOG(L_INFO,"INF:"M_NAME":P_add_p_visited_network_id: Error dropping old header.\n");		
		goto error; 
	}
	
	return r;
error:
	r = CSCF_RETURN_ERROR;
	if (x.s) pkg_free(x.s);
	return r;
}

static str ck={"ck=\"",4};
static str ik={"ik=\"",4};
static str authenticate_s={"WWW-Authenticate: ",18};
static str authenticate_e={"\r\n",2};

/**
 * Removes the CK and IK keys from the WWW-Authenticate header 
 * @param msg - the SIP reply message to remove from
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if no auth header found or #CSCF_RETURN_FALSE on error
 */
int P_remove_ck_ik(struct sip_msg *msg,char *str1,char*str2)
{
	int r = CSCF_RETURN_FALSE;
	str auth,x={0,0};
	struct hdr_field *hdr;	
	int inString = 0;
	int mlen,i;
	auth = cscf_get_authenticate(msg,&hdr);
	if (!auth.len){
		LOG(L_INFO,"INF:"M_NAME":P_remove_ck_ik: No WWW-Authenticate header found.\n");
		goto done; 
	}
	
	LOG(L_DBG,"DBG::"M_NAME":P_remove_ck_ik: Original: <%.*s>\n",
		auth.len,auth.s);
		
	x.s = pkg_malloc(authenticate_s.len+auth.len+authenticate_e.len);
	if (!x.s){
		LOG(L_ERR, "ERR"M_NAME":P_remove_ck_ik: Error allocating %d bytes\n",
			x.len);
		x.len=0;
		goto error;		
	}
	x.len=0;

	if (ck.len>ik.len) mlen = auth.len - ck.len;
	else mlen = auth.len - ik.len;
	
	STR_APPEND(x,authenticate_s);
	i=0;
	while(i<auth.len){
		if ((auth.s[i] == '\"') &&(i==0||auth.s[i-1]!='\\'))
			inString = !inString;

		if (!inString && i<mlen && strncasecmp(auth.s+i,ck.s,ck.len)==0){
			while(x.len>0 && (x.s[x.len-1]==' '||x.s[x.len-1]==','||x.s[x.len-1]=='\t')){
				x.len--;
			}
			i+=ck.len;
			while(i<auth.len && auth.s[i]!='\"')
			 i++;
			i++;
			while(i<auth.len &&(auth.s[i]==' '||auth.s[i]==','||auth.s[i]=='\t'))
			 i++;
			continue;  
		}	
		if (!inString && i<mlen && strncasecmp(auth.s+i,ik.s,ik.len)==0){
			while(x.len>0 && (x.s[x.len-1]==' '||x.s[x.len-1]==','||x.s[x.len-1]=='\t')){
				x.len--;
			}
			i+=ik.len;
			while(i<auth.len && auth.s[i]!='\"')
			 i++;
			i++;
			while(i<auth.len &&(auth.s[i]==' '||auth.s[i]==','||auth.s[i]=='\t'))
			 i++;
			continue;  
		}	
		x.s[x.len++]=auth.s[i++];	
	}		
	STR_APPEND(x,authenticate_e);		

	LOG(L_DBG,"DBG:"M_NAME":P_remove_ck_ik: Changed: <%.*s>\n",
		x.len,x.s);

	if (!cscf_del_header(msg,hdr)){
		LOG(L_INFO,"INF:"M_NAME":P_remove_ck_ik: Error dropping old authorization header.\n");		
		goto error; 
	}
	
	if (cscf_add_header(msg,&x,HDR_OTHER_T)) r = CSCF_RETURN_TRUE;
	else goto error;

	LOG(L_DBG,"DBG::"M_NAME":P_remove_ck_ik: Final   : <%.*s>\n",
		x.len,x.s);

done:
	return r;
error:
	r = CSCF_RETURN_ERROR;
	if (x.s) pkg_free(x.s);
	return r;
}




/**
 * Finds if the message comes from a registered UE at this P-CSCF
 * @param msg - the SIP message
 * @param str1 - the realm to look into
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if registered, #CSCF_RETURN_FALSE if not 
 */
int P_is_registered(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	struct via_body *vb;

	LOG(L_INFO,"DBG:"M_NAME":P_is_registered: Looking if registered\n");
//	print_r(L_INFO);

	vb = cscf_get_ue_via(msg);

	
	if (vb->port==0) vb->port=5060;
	LOG(L_INFO,"DBG:"M_NAME":P_is_registered: Looking for <%d://%.*s:%d>\n",
		vb->proto,vb->host.len,vb->host.s,vb->port);
	
	if (r_is_registered(vb->host,vb->port,vb->proto)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;	
	
	return ret;
}


static str p_asserted_identity_s={"P-Asserted-Identity: ",21};
static str p_asserted_identity_m={"<",1};
static str p_asserted_identity_e={">\r\n",3};
/**
 * Asserts the P-Preferred-Identity if registered and inserts the P-Asserted-Identity.
 * @param msg - the SIP message
 * @param str1 - the realm to look into
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if asseted, #CSCF_RETURN_FALSE if not, #CSCF_RETURN_ERROR on error 
 */
int P_assert_identity(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	struct via_body *vb;
	struct hdr_field *h=0;
	name_addr_t preferred,asserted;
	str x={0,0};

	LOG(L_INFO,"INF:"M_NAME":P_assert_identity: Asserting Identity\n");
//	print_r(L_INFO);
	
	vb = cscf_get_ue_via(msg);
	
	preferred = cscf_get_preferred_identity(msg,&h);
	
	LOG(L_INFO,"DBG:"M_NAME":P_assert_identity: Looking for <%d://%.*s:%d> Pref: %.*s\n",
		vb->proto,vb->host.len,vb->host.s,vb->port,
		preferred.len,preferred.name.s);

	asserted = r_assert_identity(vb->host,vb->port,vb->proto,preferred);
	if (!asserted.uri.len){
		ret = CSCF_RETURN_FALSE;	
	}else{
		cscf_del_header(msg,h);
		x.len = p_asserted_identity_s.len+asserted.name.len+p_asserted_identity_m.len + 
			asserted.uri.len+p_asserted_identity_e.len;
		x.s = pkg_malloc(x.len);
		if (!x.s){
			LOG(L_ERR, "ERR"M_NAME":P_assert_identity: Error allocating %d bytes\n",
				require_hdr.len);
			x.len=0;
			goto error;		
		}
		x.len=0;
		STR_APPEND(x,p_asserted_identity_s);
		STR_APPEND(x,asserted.name);
		STR_APPEND(x,p_asserted_identity_m);
		STR_APPEND(x,asserted.uri);
		STR_APPEND(x,p_asserted_identity_e);
		
		if (cscf_add_header(msg,&x,HDR_OTHER_T))
			ret = CSCF_RETURN_TRUE;
		else
			ret = CSCF_RETURN_FALSE;	
	}
	
	
	return ret;
error:
	ret=CSCF_RETURN_ERROR;
	return ret;	
}

/**
 * Asserts the P-Called-Identity
 * @param rpl - the SIP response message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if asseted, #CSCF_RETURN_FALSE if not, #CSCF_RETURN_ERROR on error 
 */
int P_assert_called_identity(struct sip_msg *rpl,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	struct hdr_field *h=0;
	str called={0,0},x={0,0};
	struct sip_msg *req = cscf_get_request_from_reply(rpl);

	LOG(L_INFO,"INF:"M_NAME":P_assert_called_identity: Asserting Identity\n");

	cscf_get_preferred_identity(rpl,&h);
	cscf_del_header(rpl,h);

	if (!req){
		LOG(L_INFO,"INF:"M_NAME":P_assert_called_identity: Error finding correspondent request.\n");
		goto error;		
	}

	called = cscf_get_called_party_id(req,&h);
	if (!called.len){
		ret = CSCF_RETURN_FALSE;	
	}else{
		x.len = p_asserted_identity_s.len+p_asserted_identity_m.len+called.len+p_asserted_identity_e.len;
		x.s = pkg_malloc(x.len);
		if (!x.s){
			LOG(L_ERR, "ERR"M_NAME":P_assert_called_identity: Error allocating %d bytes\n",
				require_hdr.len);
			x.len=0;
			goto error;		
		}
		x.len=0;
		STR_APPEND(x,p_asserted_identity_s);
		STR_APPEND(x,p_asserted_identity_m);
		STR_APPEND(x,called);
		STR_APPEND(x,p_asserted_identity_e);
		
		if (cscf_add_header(rpl,&x,HDR_OTHER_T))
			ret = CSCF_RETURN_TRUE;
		else
			ret = CSCF_RETURN_FALSE;	
	}
	
	return ret;
error:
	ret=CSCF_RETURN_FALSE;
	return ret;	
}


static str reginfo={"application/reginfo+xml",23};
/**
 * Process an incoming NOTIFY for the reg event.
 * To be Called when a NOTIFY comes in - checks if for itself.
 * if the NOTIFY is destined for this node, generate an event and respond with
 * 200 OK, and return TRUE. else return FALSE
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not 
 */
int P_process_notification(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str content_type,body;
	r_notification *n=0;
	int expires;
	LOG(L_DBG,"DBG:"M_NAME":P_NOTIFY: Checking NOTIFY\n");
	
//	print_r(L_INFO);

	/* check if we received what we should */
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_ERR,"ERR:"M_NAME":P_NOTIFY: The message is not a request\n");
		goto error;
	}
	if (msg->first_line.u.request.method.len!=6||
		memcmp(msg->first_line.u.request.method.s,"NOTIFY",6)!=0)
	{
		LOG(L_ERR,"ERR:"M_NAME":P_NOTIFY: The method is not a NOTIFY\n");
		goto error;		
	}

	/* update the subscription state */
	expires = cscf_get_subscription_state(msg);
	/* treat event */	
	content_type = cscf_get_content_type(msg);
	if (content_type.len==reginfo.len &&
		strncasecmp(content_type.s,reginfo.s,reginfo.len)==0)
	{
		body.s = get_body(msg);
		if (!body.s){
			LOG(L_ERR,"ERR:"M_NAME":P_NOTIFY: No body extracted\n");									
			goto error;
		}else{
			body.len = cscf_get_content_len(msg);
			LOG(L_DBG,"DBG:"M_NAME":P_NOTIFY: Found body: %.*s\n",
				body.len,body.s);
			n = r_notification_parse(body);
			if (!n){
				LOG(L_DBG,"DBG:"M_NAME":P_NOTIFY: Error parsing XML\n");
			}else {				
				#ifdef WITH_IMS_PM
					ims_pm_notify_reg(n,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0));
				#endif				
				if (r_notification_process(n,expires))
					ret = CSCF_RETURN_TRUE;							
				r_notification_free(n);
			}
		}
	}else{
		LOG(L_ERR,"ERR:"M_NAME":P_NOTIFY: The content should be %.*s but it is %.*s\n",
			reginfo.len,reginfo.s,content_type.len,content_type.s);
		goto error;		
	}
			
			
	return ret;
error:
	ret=CSCF_RETURN_FALSE;
	return ret;	
}


/**
 * Determines if this is the Mobile Terminating case.
 * uses the indication in the first Route header
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if terminating case, #CSCF_RETURN_FALSE if not
 */
int P_mobile_terminating(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str route={0,0};
	int i;
	
	route = cscf_get_first_route(msg,0);
	if (!route.len){
		LOG(L_DBG,"DBG:"M_NAME":P_mobile_terminating: No Route header.\n");
		goto done;
	}
	i=0;
	while(i<route.len && (route.s[i]==' ' ||route.s[i]=='\t'||route.s[i]=='<'))
		i++;
	route.s += i;
	route.len -= i;
	i=0;
	while(route.s[i]!=';' && route.s[i]!='>' && i<route.len)
		i++;
	route.len = i;
	
	if (route.len == pcscf_path_str.len &&
		strncasecmp(route.s,pcscf_path_str.s,pcscf_path_str.len)==0)
	{
		LOG(L_DBG,"DBG:"M_NAME":P_mobile_terminating: Term indication found.\n");
		ret = CSCF_RETURN_TRUE;
		goto done;
	}else{
		LOG(L_DBG,"DBG:"M_NAME":P_mobile_terminating: Term indication not found in <%.*s> as <%.*s>.\n",
			route.len,route.s,pcscf_path_str.len,pcscf_path_str.s);
	}
	
	
done:
	return ret;
}


/**
 * \deprecated
 * Removes header pointing to itself.
 * @param msg - the SIP message
 * @param str1 - if not empty, will look and remove this value instead
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not 
 */
int P_remove_route(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	str x;
	
	if (str1 && strlen(str1)){
		x.s = str1;
		x.len =strlen(str1);
		if (cscf_remove_first_route(msg,x)) 
			ret = CSCF_RETURN_TRUE;
		else 
			ret = CSCF_RETURN_FALSE;	
	}else{
		if (cscf_remove_first_route(msg,pcscf_path_str)+cscf_remove_first_route(msg,pcscf_name_str)) 
			ret = CSCF_RETURN_TRUE;
		else 
			ret = CSCF_RETURN_FALSE;	
	}
	
	return ret;
}



/**
 * Checks if the message follows Service-Route imposed at registration.
 * @param msg - the SIP message
 * @param str1 - the realm to look into for the user
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not 
 */
int P_follows_service_routes(struct sip_msg *msg,char *str1,char *str2)
{
	int i;
	struct hdr_field *hdr=0;
	rr_t *r;
	struct via_body *vb;
	r_contact *c;
	struct sip_uri uri;
	
	vb = cscf_get_ue_via(msg);
	
	if (vb->port==0) vb->port=5060;
	c = get_r_contact(vb->host,vb->port,vb->proto);
	if (!c) return CSCF_RETURN_FALSE;

	hdr = cscf_get_next_route(msg,0);
	r = 0;
	if (!hdr){
		if (c->service_route_cnt==0) 
			goto ok;
		else 
			goto nok;
	}
	r = (rr_t*) hdr->parsed;	
	/* first let's skip route pointing to myself */
	if (r&&parse_uri(r->nameaddr.uri.s, r->nameaddr.uri.len, &uri)==0 &&
		check_self(&uri.host,uri.port_no?uri.port_no:SIP_PORT,0))
	{
		r = r->next;		
		if (!r) {
			hdr = cscf_get_next_route(msg,hdr);
			if (!hdr){ 
				if (c->service_route_cnt==0) 
					goto ok;
				else 
					goto nok;
			}
			r = (rr_t*) hdr->parsed;	
		}
	}
	/* then check the others */
	
	for(i=0;i<c->service_route_cnt;i++){
		LOG(L_DBG,"DBG:"M_NAME":P_follows_service_route: mst %.*s\n",
			c->service_route[i].len,c->service_route[i].s);		
		if (!r) {
			hdr = cscf_get_next_route(msg,hdr);
			if (!hdr) 
				goto nok;
			r = (rr_t*) hdr->parsed;	
		}
		LOG(L_DBG,"DBG:"M_NAME":P_follows_service_route: src %.*s\n",
			r->nameaddr.uri.len,r->nameaddr.uri.s);		
		if (r->nameaddr.uri.len==c->service_route[i].len &&
				strncasecmp(r->nameaddr.uri.s,
					c->service_route[i].s,c->service_route[i].len)==0)
		{
			LOG(L_DBG,"DBG:"M_NAME":P_follows_service_route: src match\n");		
		} 
		else goto nok; 
			
		r = r->next;
	}
	if (r) goto nok;
	else
		if (cscf_get_next_route(msg,hdr)) goto nok;

ok:
	if (c) r_unlock(c->hash);
	return CSCF_RETURN_TRUE;	
nok:
	if (c) r_unlock(c->hash);
	return CSCF_RETURN_FALSE;	
}


static str route_s={"Route: <",8};
static str route_1={">, <",4};
static str route_e={">\r\n",3};
/**
 * Inserts the Route header containing the Service-Route to be enforced
 * @param msg - the SIP message to add to
 * @param str1 - the value to insert - !!! quoted if needed
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok or #CSCF_RETURN_ERROR on error
 */
int P_enforce_service_routes(struct sip_msg *msg,char *str1,char*str2)
{
	int i;
	str newuri={0,0};
	struct via_body *vb;
	r_contact *c;
	str x;
		
	vb = cscf_get_ue_via(msg);
	
	if (vb->port==0) vb->port=5060;
	
	c = get_r_contact(vb->host,vb->port,vb->proto);
	if (!c) return CSCF_RETURN_FALSE;
	if (!c->service_route_cnt){
		r_unlock(c->hash);
		return CSCF_RETURN_TRUE;
	}

	x.len = route_s.len + route_e.len + (c->service_route_cnt-1)*route_1.len;
	for(i=0;i<c->service_route_cnt;i++)
		x.len+=c->service_route[i].len;
			
	x.s = pkg_malloc(x.len);
	if (!x.s){
		LOG(L_ERR, "ERR:"M_NAME":P_enforce_service_route: Error allocating %d bytes\n",
			x.len);
		x.len=0;
		r_unlock(c->hash);
		return CSCF_RETURN_ERROR;
	}
	x.len=0;
	STR_APPEND(x,route_s);
	for(i=0;i<c->service_route_cnt;i++){
		if (i) STR_APPEND(x,route_1);
		STR_APPEND(x,c->service_route[i]);
	}	
	STR_APPEND(x,route_e);
	
	newuri.s = pkg_malloc(c->service_route[0].len);
	if (!newuri.s){
		LOG(L_ERR, "ERR:"M_NAME":P_enforce_service_route: Error allocating %d bytes\n",
			c->service_route[0].len);
		r_unlock(c->hash);
		return CSCF_RETURN_ERROR;
	}
	newuri.len = c->service_route[0].len;
	memcpy(newuri.s,c->service_route[0].s,newuri.len);
	msg->dst_uri = newuri;
	
	//LOG(L_ERR,"%.*s",x.len,x.s);
	r_unlock(c->hash);
	if (cscf_add_header_first(msg,&x,HDR_ROUTE_T)) {
		if (cscf_del_all_headers(msg,HDR_ROUTE_T))
			return CSCF_RETURN_TRUE;
		else {
			LOG(L_ERR,"ERR:"M_NAME":P_enforce_service_route: new Route headers added, but failed to drop old ones.\n");
			return CSCF_RETURN_ERROR;
		}
	}
	else {
		if (x.s) pkg_free(x.s);
		return CSCF_RETURN_ERROR;
	}
	
}


/**
 * Forward a message through the NAT pinhole
 * @param msg - the SIP message to forward
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if no NAT
 */
int P_NAT_relay(struct sip_msg * msg, char * str1, char * str2) 
{
	str dst;
	int len, i;
	struct ip_addr ip;
	unsigned short int port;
	r_contact *c=0;
	struct dest_info dst_info;
	struct cell *t=0;	

	if (!pcscf_nat_enable) return CSCF_RETURN_FALSE;
	
	if(msg -> first_line.type == SIP_REQUEST) {		
		/* on request get the destination from the Request-URI */
		struct sip_uri uri;
		str req_uri = msg -> first_line.u.request.uri;
		
		parse_uri(req_uri.s, req_uri.len, &uri);
		if(uri.port_no == 0)
			uri.port_no=5060;
		
		c = get_r_contact(uri.host, uri.port_no, uri.proto);
		
		if(c==NULL || c->pinhole == NULL) {
			LOG(L_DBG, "ERR:"M_NAME":P_NAT_relay: we cannot find the pinhole for contact %.*s. Sorry\n", req_uri.len, req_uri.s);
			if (c) r_unlock(c->hash);
			return CSCF_RETURN_FALSE;
		}	
		ip = c->pinhole->nat_addr;
		port = c->pinhole->nat_port;
	} else {
		/* on response get the destination from the received addr for the corresponding request */
		struct sip_msg *req;
		req = cscf_get_request_from_reply(msg);
		if(req == NULL) {
			LOG(L_ERR, "ERR:"M_NAME":P_NAT_relay: Cannot get request for the transaction\n");
			if (c) r_unlock(c->hash);
			return CSCF_RETURN_FALSE;
		}		
		ip = req->rcv.src_ip;
		port = req->rcv.src_port;	
	}

	len = 4 /* sip: */ + 4 * ip.len /* ip address */ + 1 /* : */ + 6 /* port */;
	dst.s = pkg_malloc(len);
	if (!dst.s){
		LOG(L_ERR, "ERR:"M_NAME":P_NAT_relay: Error allocating %d bytes\n", len);					
		if (c) r_unlock(c->hash);
		return CSCF_RETURN_FALSE;
	}
	strcpy(dst.s, "sip:");
	dst.len = 4;		
	dst.len += sprintf(dst.s + 4, "%d", ip.u.addr[0]);		
	for(i = 1; i < ip.len; i++)
		dst.len += sprintf(dst.s + dst.len, ".%d", ip.u.addr[i]);			
	dst.len += sprintf(dst.s + dst.len, ":%d", port);

	if (c) r_unlock(c->hash);
	
	if (msg->dst_uri.s) pkg_free(msg->dst_uri.s);
	msg -> dst_uri = dst;

	if (msg -> first_line.type == SIP_REPLY) {
		/* on reply we have to modify the t->uas->response->dst.to.sin.sin_port/sin_addr */
		t = tmb.t_gett();
		if (!t){
			LOG(L_INFO, "INFO:"M_NAME":P_NAT_relay: Can't relay non-transactional responses\n");		
			return CSCF_RETURN_FALSE;	/* error */
		}
#ifdef USE_DNS_FAILOVER
		if (!uri2dst(0,&dst_info, msg, &msg->dst_uri, PROTO_NONE)) {
#else
		if (!uri2dst(&dst_info, msg, &msg->dst_uri, PROTO_NONE)) {
#endif
			LOG(L_INFO, "INFO:"M_NAME":P_NAT_relay: Error setting uri as dst <%.*s>\n", msg -> dst_uri.len, msg -> dst_uri.s);		
			return CSCF_RETURN_FALSE;	/* error */
		}
		t->uas.response.dst = dst_info;
	}	
		
	LOG(L_INFO, "INFO:"M_NAME":P_NAT_relay: <%.*s>\n", msg -> dst_uri.len, msg -> dst_uri.s);
	return CSCF_RETURN_TRUE;
}



static str sip_s={"sip:",4};
/**
 * Forward a response through the IPSec SA
 * @param msg - the SIP message to forward
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if no NAT
 */
int P_security_relay(struct sip_msg * msg, char * str1, char * str2) 
{
	str dst={0,0};
	int len;
	r_contact *c=0;
	int proto, port;
	str host;
	struct dest_info dst_info;
	struct cell *t=0;
	struct sip_msg *req = 0;

	if (!pcscf_use_ipsec && !pcscf_use_tls) return CSCF_RETURN_FALSE;

	if(msg -> first_line.type == SIP_REQUEST) {
		/* on request, get the destination from the Request-URI */
		struct sip_uri uri;
		str req_uri = msg->first_line.u.request.uri;
		if (parse_uri(req_uri.s,req_uri.len,&uri)<0){
			LOG(L_ERR, "ERR:"M_NAME":P_security_relay: Error parsing Request-URI: <%.*s>\n",req_uri.len,req_uri.s);
			return CSCF_RETURN_FALSE;
		}
		if (uri.port_no==0) uri.port_no =5060;
		proto = uri.proto;
		host = uri.host;		
		port = uri.port_no;
	} else {
		/* On response get the source from the first via in the corresponding request */
		struct via_body *vb;
		req = cscf_get_request_from_reply(msg);
		if(req == NULL) {
			LOG(L_ERR, "ERR:"M_NAME":P_security_relay: Cannot get request for the transaction\n");
			return CSCF_RETURN_FALSE;
		}
		vb = cscf_get_ue_via(req);	
		if (vb->port==0) vb->port=5060;
		proto = vb->proto;
		host = vb->host;		
		port = vb->port;
	}
	
	len = sip_s.len + host.len + 1 /* : */ + 6 /* port */+ 14 /*;transport=tls"*/;
	dst.s = pkg_malloc(len);
	if (!dst.s){
		LOG(L_ERR, "ERR:"M_NAME":P_security_relay: Error allocating %d bytes\n", len);
		dst.len=0;
		r_unlock(c->hash);
		return CSCF_RETURN_FALSE;
	}	
	STR_APPEND(dst,sip_s);
	STR_APPEND(dst,host);

	if (msg -> first_line.type != SIP_REQUEST && req->rcv.dst_port == pcscf_tls_port  && pcscf_use_tls)
	{
		dst.len += sprintf(dst.s + dst.len, ":%d;transport=tls", req->rcv.src_port);
	}	
	else {
		c = get_r_contact(host,port,proto);
		if (!c || !c->security || c->security->type==SEC_NONE) {
			LOG(L_DBG, "ERR:"M_NAME":P_security_relay: we cannot find the contact or its IPSec/TLS SAs for <%d://%.*s:%d>.\n", 
				proto,host.len,host.s,port);
			if (c) r_unlock(c->hash);
			if (dst.s)
				pkg_free(dst.s);
			return CSCF_RETURN_FALSE;
		}					
	
		switch (c->security->type){
			case SEC_NONE:
				break;
			case SEC_IPSEC:
				if (pcscf_use_ipsec && c->security->data.ipsec) 
					dst.len += sprintf(dst.s + dst.len, ":%d", c->security->data.ipsec->port_us);
				else dst.len += sprintf(dst.s + dst.len, ":%d", port);
				break;
			case SEC_TLS:
				if (!pcscf_use_tls){
					r_unlock(c->hash);
					if (dst.s)
						pkg_free(dst.s);
				}
				if(msg -> first_line.type != SIP_REQUEST) {
					// request received on other port
					if (req->rcv.dst_port != pcscf_tls_port )  
					{
						r_unlock(c->hash);
						if (dst.s)
							pkg_free(dst.s);
						return CSCF_RETURN_FALSE;
					}
				}
				dst.len += sprintf(dst.s + dst.len, ":%d;transport=tls", c->security->data.tls->port_tls);
				break;
		}	
		r_unlock(c->hash);
	}
	
	if (msg->dst_uri.s) pkg_free(msg->dst_uri.s);
	msg -> dst_uri = dst;
	
	if (msg -> first_line.type == SIP_REPLY) {
		/* on reply we have to modify the t->uas->response->dst.to.sin.sin_port/sin_addr */
		t = tmb.t_gett();
		if (!t){
			LOG(L_INFO, "INFO:"M_NAME":P_security_relay: Can't relay non-transactional responses\n");		
			return CSCF_RETURN_FALSE;	/* error */
		}
#ifdef USE_DNS_FAILOVER
		if (!uri2dst(0,&dst_info, msg, &msg->dst_uri, PROTO_NONE)) {
#else
		if (!uri2dst(&dst_info, msg, &msg->dst_uri, PROTO_NONE)) {
#endif
			LOG(L_INFO, "INFO:"M_NAME":P_security_relay: Error setting uri as dst <%.*s>\n", msg -> dst_uri.len, msg -> dst_uri.s);		
			return CSCF_RETURN_FALSE;	/* error */
		}
		t->uas.response.dst = dst_info;
	}	
	LOG(L_INFO, "INFO:"M_NAME":P_security_relay: <%.*s>\n", msg -> dst_uri.len, msg -> dst_uri.s);
	return CSCF_RETURN_TRUE;
}

