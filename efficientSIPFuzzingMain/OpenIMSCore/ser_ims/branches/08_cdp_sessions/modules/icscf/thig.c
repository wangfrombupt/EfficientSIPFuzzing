/**
 *$Id: thig.c 430 2007-08-01 13:18:42Z vingarzan $ Author Florin Dinu
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
 *
 * I-CSCF Module - THIG Operations 
 * 
 * Copyright (C) 2006 FhG Fokus
 * 		
 */

#include "sip.h"
#include "thig.h"

#include "thig_ims_enc.h"

#include "../../mem/mem.h"
#include "../../parser/parse_hname2.h"
#include "../../parser/parse_rr.h"
#include "../../parser/parse_uri.h"

extern str icscf_thig_path_str;
extern str icscf_thig_rr_str;
extern str icscf_thig_name_str;
extern str icscf_thig_host_str;
extern int icscf_thig_port;
extern str icscf_thig_port_str;
extern str icscf_thig_param_str;
									


void print_string(char*text,char* str , int len){
    int i;
    LOG(L_ALERT,"%s",text);
    for( i=0;i<len;i++)
       printf("%c",*(str++));
    printf("\n");
}

void print_str(char* text , str str_text){
	print_string(text,str_text.s,str_text.len);	
}


/** 
 * Adds the address of the ICSCF(THIG) to the Path header
 * 
 * "Upon receiving an incoming REGISTER request for which THIG has to be 
 * applied and which includes a Path header , the ICSCF shall add the
 * routeable SIP URI of an ICSCF(THIG) to the top of the path header"
 * 
 * @param msg   - the message to update
 * @param str1  - not used
 * @param str2  - not used
 * 
 * @returns CSCF_RETURN_TRUE if OK, CSCF_RETURN_FALSE if not
 */
int I_THIG_add_Path(struct sip_msg* msg, char* str1, char* str2)
{
	int ret;
	str x;
	LOG(L_ALERT,"DBG:"M_NAME":I_add_THIG_Path: Adding THIG Path header\n");
	STR_PKG_DUP(x,icscf_thig_path_str,"pkg");
	ret=cscf_add_header_first(msg,&x,HDR_OTHER_T);
	if (!ret){
		LOG(L_ALERT,"DBG:"M_NAME":I_add_THIG_Path: Error adding Path for THIG\n");
		return CSCF_RETURN_FALSE;
	}		
	return CSCF_RETURN_TRUE;
out_of_memory:	
	return CSCF_RETURN_ERROR;
}

/**
 * Adds the address of the ICSCF(THIG) to the Record-Route header
 * 
 * "Upon receiving an incoming initial request ... and which includes a Record-Route header
 * the ICSCF(THIG)shall add its own routeable SIP URI to the top of the Record-Route header"
 * 
 * @param msg   - the message to update
 * @param str1  - not used
 * @param str2  - not used
 * 
 * @returns CSCF_RETURN_TRUE if OK, CSCF_RETURN_FALSE if not
 */
int I_THIG_add_RR(struct sip_msg* msg, char* str1, char* str2)
{	
	int ret;
	str x;
	LOG(L_ALERT,"DBG:"M_NAME":I_add_THIG_RR: Adding THIG Record-Route header\n");
	STR_PKG_DUP(x,icscf_thig_rr_str,"pkg");
	ret=cscf_add_header_first(msg,&x,HDR_RECORDROUTE_T);
	if (!ret){
		LOG(L_ALERT,"DBG:"M_NAME":I_add_THIG_RR: Error adding Record-Route for THIG\n");
		return CSCF_RETURN_FALSE;
	}		
	return CSCF_RETURN_TRUE;
out_of_memory:	
	return CSCF_RETURN_ERROR;
}


str s_hdr_src={": \r\n",4};
str s_new_hdr_1={"<sip:",5};
str s_new_hdr_2={";lr>",4};
str s_new_hdr_3={"SIP/2.0/UDP ",12};

/**
 * Encrypts a particular header.Use this if you already know what header should be encrypted
 * 
 * @param str1 - name of header to be encrypted
 * @returns CSCF_RETURN_TRUE if OK, CSCF_RETURN_ERROR on THIG error, 
 * 			CSCF_RETURN_FALSE if THIG cannot continue due to another error.
 * 
 **/
int I_THIG_encrypt_header(struct sip_msg* msg, char* str1, char* str2)
{	
	str hdr_name={0,0};
	char hdr_str[64];
	struct hdr_field *hdr=0,hdr_search;
	str orig={0,0},enc={0,0},repl={0,0};
	
	hdr_name.s = str1;
	hdr_name.len = strlen(str1);
	
	memcpy(hdr_str,hdr_name.s,hdr_name.len);
	memcpy(hdr_str+hdr_name.len,s_hdr_src.s,s_hdr_src.len);
	if (!parse_hname2(hdr_str,hdr_str+hdr_name.len+s_hdr_src.len,&hdr_search)){
		LOG(L_ERR,"ERR:"M_NAME":I_THIG_encrypt_header: Error parsing header name <%.*s> !!!\n",hdr_name.len,hdr_name.s);
		return CSCF_RETURN_ERROR;
	}
	
	LOG(L_ALERT,"DBG:"M_NAME":I_THIG_encrypt_header: Encrypting header %.*s\n",hdr_name.len,hdr_name.s);   
	for(hdr = (hdr_search.type==HDR_OTHER_T) ? cscf_get_next_header(msg,hdr_name,hdr) : cscf_get_next_header_type(msg,hdr_search.type,hdr);
		hdr;
		hdr = (hdr_search.type==HDR_OTHER_T) ? cscf_get_next_header(msg,hdr_name,hdr) : cscf_get_next_header_type(msg,hdr_search.type,hdr))
	{
		orig.s = hdr->body.s;
		orig.len = hdr->body.len ;
		
		LOG(L_ALERT,"DBG:"M_NAME":I_THIG_encrypt_header: Orig: <%.*s>\n",orig.len,orig.s);   
		enc = thig_encrypt(orig);
		if (!enc.len) {
			LOG(L_ERR,"ERR:"M_NAME":I_THIG_encrypt_header: error encrypting <%.*s>. THIG skipeed!!!\n",orig.len,orig.s);
			return CSCF_RETURN_FALSE;
		}
		
		repl.len = hdr_name.len + icscf_thig_host_str.len + 1 + icscf_thig_port_str.len + 1 + icscf_thig_param_str.len + 1 + enc.len;
		
		if (hdr->type==HDR_VIA_T) repl.len += s_new_hdr_3.len;
		else repl.len += s_new_hdr_1.len + s_new_hdr_2.len;
		
		repl.s = pkg_malloc(repl.len);
		if (!repl.s){
			LOG(L_ERR,"ERR:"M_NAME":I_THIG_encrypt_header: error allocating %d bytes. THIG skipeed!!!\n",repl.len);
			pkg_free(enc.s);
			return CSCF_RETURN_FALSE;
		}	
		repl.len = 0;
		if (hdr->type==HDR_VIA_T) {
			STR_APPEND(repl,s_new_hdr_3);
			STR_APPEND(repl,icscf_thig_host_str);
			repl.s[repl.len++]=':';
			STR_APPEND(repl,icscf_thig_port_str);
			repl.s[repl.len++]=';';
			STR_APPEND(repl,icscf_thig_param_str);
			repl.s[repl.len++]='=';
			STR_APPEND(repl,enc);
		}else{
			STR_APPEND(repl,s_new_hdr_1);
			STR_APPEND(repl,icscf_thig_host_str);
			repl.s[repl.len++]=':';
			STR_APPEND(repl,icscf_thig_port_str);
			repl.s[repl.len++]=';';
			STR_APPEND(repl,icscf_thig_param_str);
			repl.s[repl.len++]='=';
			STR_APPEND(repl,enc);
			STR_APPEND(repl,s_new_hdr_2);
		}
		pkg_free(enc.s);
		LOG(L_ALERT,"DBG:"M_NAME":I_THIG_encrypt_header: Enc : <%.*s>\n",repl.len,repl.s);   
		cscf_replace_string(msg,orig,repl);
	}	
	return CSCF_RETURN_TRUE;
}



/**
 * Decrypts a particular header.Use this if you already know what header should be encrypted
 * 
 * @param str1 - name of header to be decrypted
 * @returns CSCF_RETURN_TRUE if OK, CSCF_RETURN_ERROR on THIG error, 
 * 			CSCF_RETURN_FALSE if THIG cannot continue due to another error.
 * 
 * */
int I_THIG_decrypt_header(struct sip_msg* msg, char* str1, char* str2)
{
	str hdr_name={0,0};
	char hdr_str[64];
	struct hdr_field *hdr=0,hdr_search;
	str orig={0,0},dec={0,0};
	struct via_body *vb;
	struct via_param *vp;
	rr_t *rr;
	struct sip_uri uri;
	int i,k;
	
	hdr_name.s = str1;
	hdr_name.len = strlen(str1);
	
	memcpy(hdr_str,hdr_name.s,hdr_name.len);
	memcpy(hdr_str+hdr_name.len,s_hdr_src.s,s_hdr_src.len);
	if (!parse_hname2(hdr_str,hdr_str+hdr_name.len+s_hdr_src.len,&hdr_search)){
		LOG(L_ERR,"ERR:"M_NAME":I_THIG_decrypt_header: Error parsing header name <%.*s> !!!\n",hdr_name.len,hdr_name.s);
		return CSCF_RETURN_ERROR;
	}
	
	LOG(L_ALERT,"DBG:"M_NAME":I_THIG_decrypt_header: Decrypting header %.*s\n",hdr_name.len,hdr_name.s);   
	for(hdr = (hdr_search.type==HDR_OTHER_T) ? cscf_get_next_header(msg,hdr_name,hdr) : cscf_get_next_header_type(msg,hdr_search.type,hdr);
		hdr;
		hdr = (hdr_search.type==HDR_OTHER_T) ? cscf_get_next_header(msg,hdr_name,hdr) : cscf_get_next_header_type(msg,hdr_search.type,hdr))
	{
		if (hdr->type==HDR_VIA_T){
			if (!hdr->parsed){
				vb = pkg_malloc(sizeof(struct via_body));
				if (!vb){
					LOG(L_ERR,"ERR:"M_NAME":I_THIG_decrypt_header: Error allocating %d bytes\n",sizeof(struct via_body));
					return CSCF_RETURN_ERROR;
				}
				parse_via(hdr->body.s,hdr->body.s+hdr->body.len,vb);
				hdr->parsed = vb;
			}
			
			for(vb = hdr->parsed;vb;vb = vb->next)
			{
				if (vb->port != icscf_thig_port ||
					vb->host.len != icscf_thig_host_str.len ||
					strncasecmp(vb->host.s,icscf_thig_host_str.s,icscf_thig_host_str.len)!=0){
						continue;
					}
				k=0;
				for(vp=vb->param_lst;vp;vp=vp->next)
					if (vp->name.len == icscf_thig_param_str.len &&
						strncasecmp(vp->name.s,icscf_thig_param_str.s,icscf_thig_param_str.len)==0){
							k=1;
							orig = vp->value;
							break;
						}
				if (!k) continue;
				LOG(L_ALERT,"DBG:"M_NAME":I_THIG_decrypt_header: Orig: <%.*s>\n",orig.len,orig.s);   
				dec = thig_decrypt(orig);
				if (!dec.len) {
					LOG(L_ERR,"ERR:"M_NAME":I_THIG_decrypt_header: error decrypting <%.*s>. THIG skipeed!!!\n",orig.len,orig.s);
					return CSCF_RETURN_FALSE;
				}
				orig.s = vb->name.s;
				orig.len = vb->last_param->value.s - orig.s + vb->last_param->value.len;
				if (!cscf_replace_string(msg,orig,dec)){
					LOG(L_ERR,"ERR:"M_NAME":I_THIG_decrypt_header: error replacing string!!!\n");
					return CSCF_RETURN_FALSE;					
				}
			}
		}else{
			if (!hdr->parsed){
				if (parse_rr(hdr)<0){
					LOG(L_ERR,"ERR:"M_NAME":I_THIG_decrypt_header: Error parsing as *Route header <%.*s>\n",hdr->body.len,hdr->body.s);
					continue;
				}
			}
			rr = hdr->parsed;
			if (!rr) {
				LOG(L_ERR,"ERR:"M_NAME":I_THIG_decrypt_header: no rr in this header <%.*s>\n",hdr->body.len,hdr->body.s);
				continue;
			}
			while(rr){
				if (parse_uri(rr->nameaddr.uri.s,rr->nameaddr.uri.len,&uri)!=0){
					LOG(L_ERR,"ERR:"M_NAME":I_THIG_decrypt_header: error parsing uri <%.*s>\n",rr->nameaddr.uri.len,rr->nameaddr.uri.s);
					continue;	
				}
				if (uri.port_no!=icscf_thig_port ||
					uri.host.len!=icscf_thig_host_str.len ||
					strncasecmp(uri.host.s,icscf_thig_host_str.s,icscf_thig_host_str.len)!=0){
						continue;
					}
				k=0;
				for(i=0;i<uri.params.len-icscf_thig_param_str.len;i++)
					if (strncasecmp(uri.params.s,icscf_thig_param_str.s,icscf_thig_param_str.len)==0){
						k = 1;
						i+=icscf_thig_param_str.len+1;
						orig.s = uri.params.s+i;
						orig.len = 0;
						while(i<uri.params.len && uri.params.s[i]!=';' && uri.params.s[i]!='&'){
							i++;
							orig.len++;
						}
					}
				if (!k) continue;
				LOG(L_ALERT,"DBG:"M_NAME":I_THIG_decrypt_header: Orig: <%.*s>\n",orig.len,orig.s);   
				dec = thig_decrypt(orig);
				if (!dec.len) {
					LOG(L_ERR,"ERR:"M_NAME":I_THIG_decrypt_header: error decrypting <%.*s>. THIG skipeed!!!\n",orig.len,orig.s);
					return CSCF_RETURN_FALSE;
				}
				orig.s = rr->nameaddr.name.s;
				orig.len = rr->len;
				if (!cscf_replace_string(msg,orig,dec)){
					LOG(L_ERR,"ERR:"M_NAME":I_THIG_decrypt_header: error replacing string!!!\n");
					return CSCF_RETURN_FALSE;					
				}
				rr = rr->next;
			}
		}			
	}
	return CSCF_RETURN_TRUE;
}

/**
 * Tries to encrypt all the headers that need THIG to be applied to.
 * 
 * @param msg  - the message to update
 * @param str1 - not used
 * @param str2 - not used
 * 
 * @returns TRUE or FALSE
 */
int I_THIG_encrypt_all_headers(struct sip_msg* msg, char* str1, char* str2)
{
	int ret;
	ret = I_THIG_encrypt_header(msg,"Via",0);
	if (ret!=CSCF_RETURN_TRUE) return ret;
	ret = I_THIG_encrypt_header(msg,"Route",0);
	if (ret!=CSCF_RETURN_TRUE) return ret;
	ret = I_THIG_encrypt_header(msg,"Record-Route",0);
	if (ret!=CSCF_RETURN_TRUE) return ret;
	ret = I_THIG_encrypt_header(msg,"Service-Route",0);
	if (ret!=CSCF_RETURN_TRUE) return ret;	

	return 1;
}

/**
 * Tries to decrypt all the headers that need THIG to be applied to.
 * 
 * @param msg  - the message to update
 * @param str1 - not used
 * @param str2 - not used
 * 
 * @returns TRUE or FALSE
 */
int I_THIG_decrypt_all_headers(struct sip_msg* msg, char* str1, char* str2)
{
	int ret;
	ret = I_THIG_decrypt_header(msg,"Via",0);
	if (ret!=CSCF_RETURN_TRUE) return ret;
	ret = I_THIG_decrypt_header(msg,"Route",0);
	if (ret!=CSCF_RETURN_TRUE) return ret;
	ret = I_THIG_decrypt_header(msg,"Record-Route",0);
	if (ret!=CSCF_RETURN_TRUE) return ret;
//	ret = I_THIG_decrypt_header(msg,"Service-Route",0);
//	if (ret!=CSCF_RETURN_TRUE) return ret;	

	return 1;
}
