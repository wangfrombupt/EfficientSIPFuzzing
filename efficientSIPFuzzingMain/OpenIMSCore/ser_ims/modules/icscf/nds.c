/**
 * $Id: nds.c 35 2006-11-29 18:52:20Z vingarzan $
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
 * Interrogating-CSCF - Network Domain Security Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#include "nds.h"

#include "../../parser/hf.h"
#include "../../parser/msg_parser.h"
#include "../../parser/parse_via.h"
#include "../../mem/shm_mem.h"

#include "mod.h"
#include "sip.h"
#include "db.h"


extern int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 
										/**< link to the stateless reply function in sl module */


/** Defines the untrusted headers */
str untrusted_headers[]={
	{"P-Asserted-Identity",19},
	{"P-Access-Network-Info",21},
	{"P-Charging-Vector",17},
	{"P-Charging-Function-Addresses",29},
	{0,0}	
}; 

/** The cached list of trusted domains */
static str *trusted_domains=0;



/**
 * Checks if a request comes from a trusted domain.
 * If not calls function to respond with 403 to REGISTER or clean the message of
 * untrusted headers
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if trusted, #CSCF_RETURN_FALSE if not , #CSCF_RETURN_ERROR on REGISTER or error 
 */
int I_NDS_check_trusted(struct sip_msg* msg, char* str1, char* str2)
{
	int result;
	LOG(L_ERR,"DBG:"M_NAME":I_NDS_check_trusted: Starting ...\n");
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_ERR,"ERR:"M_NAME":I_NDS_check_trusted: The message is not a request\n");
		result = CSCF_RETURN_TRUE;	
		goto done;
	}
	if (I_NDS_is_trusted(msg,str1,str2)){
		LOG(L_INFO,"INF:"M_NAME":I_NDS_check_trusted: Message comes from a trusted domain\n");
		result = CSCF_RETURN_TRUE;	
		goto done;
	} else {
		LOG(L_INFO,"INF:"M_NAME":I_NDS_check_trusted: Message comes from an untrusted domain\n");
		result = CSCF_RETURN_FALSE;					
		if (msg->first_line.u.request.method.len==8 &&
			memcmp(msg->first_line.u.request.method.s,"REGISTER",8)==0){
			sl_reply(msg,(char*)403,MSG_403);
			LOG(L_INFO,"INF:"M_NAME":I_NDS_check_trusted: REGISTER request terminated.\n");
		} else {
			if (!I_NDS_strip_headers(msg,str1,str2)){
				result = CSCF_RETURN_ERROR;
				sl_reply(msg,(char*)500,MSG_500);
				LOG(L_INFO,"INF:"M_NAME":I_NDS_check_trusted: Stripping untrusted headers failed, Responding with 500.\n");				
			}
		}					
	}
	
done:	
	LOG(L_ERR,"DBG:"M_NAME":I_NDS_check_trusted: ... Done\n");
	return result;
}

/**
 * Decides if a message comes from a trusted domain.
 * \todo - SOLVE THE LOCKING PROBLEM - THIS IS A READER
 * @param msg - the SIP request message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if trusted, #CSCF_RETURN_FALSE 
 */
int I_NDS_is_trusted(struct sip_msg *msg, char* str1, char* str2)
{
	struct via_body *vb;
	str subdomain;
	int i;
	
	vb = msg->via1;
	if (!vb) {
		LOG(L_ERR,"ERR:"M_NAME":I_NDS_is_trusted: Error VIA1 hdr not found\n");
		return 0;
	}
	subdomain=vb->host;
	LOG(L_DBG,"DBG:"M_NAME":I_NDS_is_trusted: Message comes from <%.*s>\n",
		subdomain.len,subdomain.s);
		
	i=0;
	while(trusted_domains[i].len){
		if (trusted_domains[i].len<=subdomain.len){
			if (strncasecmp(subdomain.s+subdomain.len-trusted_domains[i].len,
				trusted_domains[i].s,
				trusted_domains[i].len)==0 &&
					(trusted_domains[i].len==subdomain.len ||
					 subdomain.s[subdomain.len-trusted_domains[i].len-1]=='.'))
			{  					
				LOG(L_DBG,"DBG:"M_NAME":I_NDS_is_trusted: <%.*s> matches <%.*s>\n",
					subdomain.len,subdomain.s,trusted_domains[i].len,trusted_domains[i].s);
				return CSCF_RETURN_TRUE;
			} else {
//				LOG(L_DBG,"DBG:"M_NAME":I_NDS_is_trusted: <%.*s> !matches <%.*s>\n",
//					subdomain.len,subdomain.s,trusted_domains[i].len,trusted_domains[i].s);
			}					
		}
		i++;
	}
	return CSCF_RETURN_FALSE;
}



/**
 * Strips untrusty headers from a SIP request.
 * Searched headers are declared in untrusted_headers 
 * @param msg - the SIP request message
 * @param str1 - not used
 * @param str2 - not used
 * @returns the number of headers stripped
 */
int I_NDS_strip_headers(struct sip_msg *msg, char* str1, char* str2)
{
	struct hdr_field *hdr;
	int i,cnt=0;
	if (parse_headers(msg,HDR_EOH_F,0)<0) return 0;
	for (hdr = msg->headers;hdr;hdr = hdr->next)
		for (i=0;untrusted_headers[i].len;i++)
			if (hdr->name.len == untrusted_headers[i].len &&
				strncasecmp(hdr->name.s,untrusted_headers[i].s,hdr->name.len)==0){				
				if (!cscf_del_header(msg,hdr)) return 0;
				cnt++;
			}
	LOG(L_DBG,"DBG:"M_NAME":I_NDS_strip_headers: Deleted %d headers\n",cnt);			
	return cnt;
}



/**
 * Refreshes the trusted domain list reading them from the db.
 * Drops the old cache and queries the db
 * \todo - IMPLEMENT A WAY TO PUSH AN EXTERNAL EVENT FOR THIS
 * \todo - SOLVE THE LOCKING PROBLEM - THIS IS A WRITER
 * @returns 1 on success, 0 on failure
 */
int I_NDS_get_trusted_domains()
{
	int i;
	/* free the old cache */
	if (trusted_domains!=0){
		i=0;
		while(trusted_domains[i].s){
			shm_free(trusted_domains[i].s);
			i++;
		}
		shm_free(trusted_domains);
	}
	return icscf_db_get_nds(&trusted_domains);
}

