/*
 * $Id: security.c 476 2007-11-02 18:41:52Z vingarzan $
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
 * Proxy-CSCF - Gm Security Operations
 * 
 * IPSec only is now supported and it is transparent in the sense that if the client
 * does not employ it, the functionality is disabled.
 * 
 * The ipsec_P_*.sh scripts are used to control the IPSec and are give as an 
 * example for usage with the linux-2.6 kernel. ipsec_tools is required for this (setkey).
 * 
 * Added TLS support
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include <signal.h> 
#include <stdlib.h>

#include "security.h"


#include "mod.h"
#include "sip.h"
#include "registration.h"
#include "registrar.h"
#include "registrar_subscribe.h"
#include "../../ip_addr.h"
#include "../../data_lump.h"

extern int   pcscf_use_ipsec;			/**< whether to use or not ipsec */
extern char* pcscf_ipsec_host;			/**< IP for protected server */
extern int   pcscf_ipsec_port_c;		/**< PORT for protected client */
extern int   pcscf_ipsec_port_s;		/**< PORT for protected server */

extern char* pcscf_ipsec_P_Inc_Req;		/**< Req E->P */
extern char* pcscf_ipsec_P_Out_Rpl;		/**< Rpl E<-P */
extern char* pcscf_ipsec_P_Out_Req;		/**< Req E<-P */
extern char* pcscf_ipsec_P_Inc_Rpl;		/**< Rpl E->P */
extern char* pcscf_ipsec_P_Drop;		/**< Drop */

extern r_hash_slot *registrar;			/**< the contacts */
extern int r_hash_size;					/**< records tables parameters 	*/

int current_spi=5000;		/**< current SPI value */
extern int pcscf_use_tls;					/**< whether to use or not tls 						*/
extern int pcscf_tls_port;					/**< PORT for TLS server 						*/
extern int tls_disable;

/**
 * Returns the next unused SPI.
 * \todo - make sure that this SPI is not used at the moment
 * @returns the next SPI
 */
int get_next_spi()
{
	return current_spi++;
}



#define get_qparam(src,name,dst) \
{\
	int i,j;\
	(dst).s=0;(dst).len=0;\
	for(i=0;i<(src).len-(name).len;i++)\
		if (strncasecmp((src).s+i,(name).s,(name).len)==0){\
			j=i+(name).len;\
			(dst).s = (src).s+j;\
			(dst).len = 0;\
			while(j<(src).len && (src).s[j]!='\"') \
				j++;			\
			(dst).len = j-i-(name).len;\
			break;\
		}		\
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

#define strtoint(src,dest) \
{\
	int i;\
	(dest)=0;\
	for(i=0;i<(src).len;i++)\
		if ((src).s[i]>='0' && (src).s[i]<='9')\
			(dest) = (dest)*10 + (src).s[i] -'0';\
}

str s_security_client={"Security-Client",15};
str s_security_verify={"Security-Verify",15};

str s_security_server_s={"Security-Server: ",17};
str s_security_server_e={"\r\n",2};

static str s_ck={"ck=\"",4};
static str s_ik={"ik=\"",4};
static str s_ealg={"ealg=",5};
static str s_alg={"alg=",4};
static str s_spi_c={"spi-c=",6};
static str s_spi_s={"spi-s=",6};
static str s_port_c={"port-c=",7};
static str s_port_s={"port-s=",7};

static str s_des_in={"des-ede3-cbc",12};
static str s_des_out={"3des-cbc",8};
static str s_aes_in={"aes-cbc",7};
static str s_aes_out={"rijndael-cbc",12};
//static str s_null_in={"null",4};
static str s_null_out={"null",4};

static str s_md5_in={"hmac-md5-96",11};
static str s_md5_out={"hmac-md5",8};
static str s_sha_in={"hmac-sha-1-96",13};
static str s_sha_out={"hmac-sha1",9};

extern time_t time_now;


static str s_tls={"tls", 3};
static str s_ipsec={"ipsec-3gpp", 10};
static str s_q={"q=", 2};

/**
 * Looks for  the tls session hash 
 * @param req - sip msg request 
 * @returns  tls session hash on success or 0  
 */
unsigned long tls_get_session_hash(struct sip_msg *req)
{
	unsigned long s_hash = 0;
	if (!pcscf_use_tls) return 0;
	s_hash = get_tls_session_hash(req);
	if (!s_hash){
		LOG(L_ERR,"ERR:"M_NAME":tls_get_session_hash: Session Hash could not be obtained !\n");
		return 0;
	}
	return s_hash;
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
 * Looks for the security type in the Security header .
 * @param security_header_body - input security header
 * @returns the type of security
 */
static r_security_type cscf_get_security_type(str security_header_body)
{
	str sec_type_s;
	int i;
	
	sec_type_s.s = security_header_body.s;
	sec_type_s.len = security_header_body.len;
	for (i = 0; i< security_header_body.len; i++)
		if (security_header_body.s[i] == ';') {
			sec_type_s.len = i;
			break;
		}	
	str_trim(&sec_type_s);
	if (sec_type_s.len==s_tls.len &&
		!strncasecmp(sec_type_s.s, s_tls.s , s_tls.len))
		return SEC_TLS;
	else if (sec_type_s.len==s_ipsec.len &&
			!strncasecmp(sec_type_s.s, s_ipsec.s , s_ipsec.len))
			return SEC_IPSEC;
	return SEC_NONE;
}

/**
 * Looks for the prefered Security header .
 * @param req - the SIP message
 * @param header_name - SIP header name 
 * @param type - output value of the security type
 * @param q - output preferate value
 * @returns the security body for the provided SIP message
 */
str cscf_get_pref_security_header(struct sip_msg *req, str header_name,r_security_type *type, float *q)
{
	str q_sec={0,0},t_sec;
	r_security_type q_type=SEC_NONE,t_type;
	float q_q=-1,t_q;
	struct hdr_field *hdr=0;	
	str tmp={0,0};
	char c;	

		
	/* first find the highest q */
	for(hdr = cscf_get_next_header(req,header_name,(void*)0);hdr;hdr = cscf_get_next_header(req,header_name,hdr)){
		t_sec = hdr->body;
		/* if unknown type, skip */
		t_type = cscf_get_security_type(t_sec);
		if (t_type==SEC_NONE ||
			(!pcscf_use_ipsec&&t_type==SEC_IPSEC)||
			(!pcscf_use_tls&&t_type==SEC_TLS)) continue;
		/* check if q is maximum */
		get_param(t_sec,s_q,tmp);
		if (tmp.len) {
			c = tmp.s[tmp.len];
			tmp.s[tmp.len]=0;			
			t_q = atof(tmp.s);
			tmp.s[tmp.len]=c;
		}	
		else t_q = -1;
		if (t_q > q_q || q_sec.len==0)	{			
			q_sec = t_sec;
			q_q = t_q;
			q_type = t_type;
		}		
	}

	if (!q_sec.len) {
		LOG(L_INFO,"DBG:"M_NAME":cscf_get_pref_security_header: No known Security header found.\n");
		return q_sec;
	}	
	
	if (type) *type = q_type;
	if (q) *q = q_q;
	return q_sec;
}


/**
 * Saves the Contact Security information into the P-CSCF registrar for this contact.
 * @param req - REGISTER request
 * @param auth - WWW-Authenticate header
 * @param sec_hdr - Security header
 * @param type - Security Type
 * @param q - Preference Value
 */
r_contact* save_contact_security(struct sip_msg *req, str auth, str sec_hdr,r_security_type type,float q)
{
	contact_t* c=0;
	contact_body_t* b=0;	
	r_contact *rc;
	enum Reg_States reg_state=REG_PENDING;
	int expires,pending_expires=60;
	struct sip_uri puri;
	r_security *s=0;
	
	b = cscf_parse_contacts(req);
	
	if (!b||!b->contacts) {
		LOG(L_ERR,"ERR:"M_NAME":save_contact_security: No contacts found\n");
		goto error; 
	}
	
	if (b) c = b->contacts;
			
	r_act_time();
	/* the Security works for just 1 contact/registration! */
	if(c){
		LOG(L_DBG,"DBG:"M_NAME":save_contact_security: <%.*s>\n",c->uri.len,c->uri.s);
		
		expires = time_now+pending_expires;
		
		if (parse_uri(c->uri.s,c->uri.len,&puri)<0){
			LOG(L_DBG,"DBG:"M_NAME":save_contact_security: Error parsing Contact URI <%.*s>\n",c->uri.len,c->uri.s);
			goto error;			
		}
		if (puri.port_no==0) puri.port_no=5060;
		LOG(L_DBG,"DBG:"M_NAME":save_contact_security: %d %.*s : %d\n",
			puri.proto, puri.host.len,puri.host.s,puri.port_no);

		if (type == SEC_TLS) 
			puri.proto = PROTO_TLS;

		/* create the r_security structure */
		s = new_r_security(sec_hdr,type,q);
		if (!s) goto error;	

		switch(type) {
			case SEC_NONE:
				break;
			case SEC_TLS:
				// r_tls creation happens on 200
				break;
			case SEC_IPSEC:
			{
				/* then parse the parameters */
				r_ipsec *ipsec;	
				str ck,ik,ealg,alg,tmp;
				str alg_setkey,ealg_setkey;
				unsigned int spi_uc,spi_us;
        		unsigned int spi_pc,spi_ps;
				int port_uc,port_us;
				char ck_c[64],ik_c[64];
				str ck_esp={ck_c,0},ik_esp={ik_c,0};
									
				get_qparam(auth,s_ck,ck);
				LOG(L_DBG,"DBG:"M_NAME":save_contact_security: CK: <%.*s>\n",
					ck.len,ck.s);
				get_qparam(auth,s_ik,ik);
				LOG(L_DBG,"DBG:"M_NAME":save_contact_security: IK: <%.*s>\n",
					ik.len,ik.s);		
				get_param(sec_hdr,s_ealg,ealg);
				LOG(L_DBG,"DBG:"M_NAME":save_contact_security: Enc Algorithm: <%.*s>\n",
					ealg.len,ealg.s);
				get_param(sec_hdr,s_alg,alg);
				LOG(L_DBG,"DBG:"M_NAME":save_contact_security: Int Algorithm: <%.*s>\n",
					alg.len,alg.s);
				/* and for spis */
				get_param(sec_hdr,s_spi_c,tmp);
				strtoint(tmp,spi_uc);
				LOG(L_DBG,"DBG:"M_NAME":save_contact_security: SPI-C: %d\n",
					spi_uc);
				get_param(sec_hdr,s_spi_s,tmp);
				strtoint(tmp,spi_us);
				LOG(L_DBG,"DBG:"M_NAME":save_contact_security: SPI-S: %d\n",
					spi_us);
				/* and for ports */
				get_param(sec_hdr,s_port_c,tmp);
				strtoint(tmp,port_uc);
				LOG(L_DBG,"DBG:"M_NAME":save_contact_security: Port-C: %d\n",
					port_uc);
				get_param(sec_hdr,s_port_s,tmp);
				strtoint(tmp,port_us);
				LOG(L_DBG,"DBG:"M_NAME":save_contact_security: Port-S: %d\n",
					port_us);
		
				ck_esp.s[ck_esp.len++]='0';
				ck_esp.s[ck_esp.len++]='x';	
				if (ealg.len == s_des_in.len && strncasecmp(ealg.s,s_des_in.s,ealg.len)==0) {
					memcpy(ck_esp.s+ck_esp.len,ck.s,32);ck_esp.len+=32;
					memcpy(ck_esp.s+ck_esp.len,ck.s,16);ck_esp.len+=16;
					ealg_setkey = s_des_out;
				}
				else
				if (ealg.len == s_aes_in.len && strncasecmp(ealg.s,s_aes_in.s,ealg.len)==0) {
					memcpy(ck_esp.s+ck_esp.len,ck.s,ck.len);ck_esp.len+=ck.len;
					ealg_setkey = s_aes_out;
				}else {
					memcpy(ck_esp.s+ck_esp.len,ck.s,ck.len);ck_esp.len+=ck.len;
					ealg_setkey = s_null_out;
					ealg = s_null_out;
				}
			
				ik_esp.s[ik_esp.len++]='0';
				ik_esp.s[ik_esp.len++]='x';		
				if (alg.len == s_md5_in.len && strncasecmp(alg.s,s_md5_in.s,alg.len)==0) {
					memcpy(ik_esp.s+ik_esp.len,ik.s,ik.len);ik_esp.len+=ik.len;
					alg_setkey = s_md5_out;
				}
				else
				if (alg.len == s_sha_in.len && strncasecmp(alg.s,s_sha_in.s,alg.len)==0) {		
					memcpy(ik_esp.s+ik_esp.len,ik.s,ik.len);ik_esp.len+=ik.len;
					memcpy(ik_esp.s+ik_esp.len,"00000000",8);ik_esp.len+=8;
					alg_setkey = s_sha_out;
				}else{
					LOG(L_ERR,"ERR:"M_NAME":save_contact_security: Unknown Integrity algorithm <%.*s>\n",alg.len,alg.s);
					goto error;
				}
				
				spi_pc=get_next_spi();	
				spi_ps=get_next_spi();	

				ipsec = new_r_ipsec(spi_uc,spi_us,spi_pc,spi_ps,port_uc,port_us,
					ealg_setkey,ealg, ck_esp,alg_setkey,alg, ik_esp);
				if (!ipsec) goto error;
				s->data.ipsec = ipsec;
				
				puri.port_no = ipsec->port_us;
				/*
				 * this should actually be port_uc... then the cscf_get_ue_via should be 
				 * changed to give rport and not the port in the via. but this would
				 * break NATed clients...
				 */
			}
				break;
		}
	}
	
	rc = update_r_contact_sec(puri.host,puri.port_no,puri.proto,
			&(c->uri),&reg_state,&expires,s);						

	return rc;	
error:
	if (s) free_r_security(s);
	return 0;	
}

/**
 * Executes an external command.
 * Used to call the IPSec scripts/commands. 
 * \note Temporarily disabled the signal for SIGCHLD.
 * Logs the output of the command
 * @param cmd - full command to execute, with parameters
 * @returns 1 if ok, 0 on error
 */
inline int execute_cmd(char *cmd)
{
	FILE *p;
	void *prev;
	char out[256];
	LOG(L_INFO,"INF:"M_NAME":execute_cmd: [%s]\n",cmd);		
	/* because SER forked dies when a chield dies, SIGCHLD is masked temporary */
	prev = signal(SIGCHLD,SIG_DFL);
	p = popen(cmd,"r");
	if (!p) {		
		LOG(L_ERR,"ERR:"M_NAME":execute_cmd: Error executing cmd> %s\n",cmd);
		signal(SIGCHLD,prev);	
		return 0;
	}
	while(fgets(out,256,p))
		LOG(L_DBG,"DBG:"M_NAME":execute_cmd: > %s",out);
	pclose(p);
	signal(SIGCHLD,prev);	
	return 1;
}

/**
 * Process the REGISTER and verify Client-Security.
 * @param req - Register request
 * @param str1 - not used
 * @param str2 - not used
 * @returns 1 if ok, 0 if not
 */
int P_verify_security(struct sip_msg *req,char *str1, char *str2)
{
	str sec_hdr;
	struct hdr_field *h;
	struct via_body *vb;
	r_contact *c;
	r_security *s;
	r_security_type sec_type;
	float sec_q;
	
	str ealg,alg,tmp;
	unsigned int spi_pc,spi_ps;;
	int port_pc,port_ps;

	vb = cscf_get_first_via(req,&h);

	LOG(L_INFO,"DBG:"M_NAME":P_verify_security: Looking for <%d://%.*s:%d> \n",	vb->proto,vb->host.len,vb->host.s,vb->port);

	c = get_r_contact(vb->host,vb->port,vb->proto);

	r_act_time();
	if (!c){
		//first register
		LOG(L_DBG,"DBG:"M_NAME":P_verify_security: No Contact found ! \n");
		return CSCF_RETURN_TRUE;
	}

	if (!r_valid_contact(c) || !c->security_temp){
		LOG(L_DBG,"DBG:"M_NAME":P_verify_security: No security temp !.\n");
		r_unlock(c->hash);
		return CSCF_RETURN_TRUE;
	}

	sec_hdr = cscf_get_pref_security_header(req,s_security_verify, &sec_type,&sec_q);
	if (!sec_hdr.len)
	{	
		LOG(L_DBG,"DBG:"M_NAME":P_verify_security: No Security-Verify header found.\n");
		r_unlock(c->hash);
		return CSCF_RETURN_TRUE;
	}
	
	s = c->security_temp;
	
	switch (s->type)
	{
	case SEC_NONE:
		break;
	case SEC_TLS:
		if (sec_type != SEC_TLS || req->rcv.dst_port != pcscf_tls_port)
					goto error;
		break;
	case SEC_IPSEC:
		if (sec_type != SEC_IPSEC || req->rcv.dst_port != pcscf_ipsec_port_s)
		{
			LOG(L_INFO,"DBG:"M_NAME":P_verify_security: Not IPSEC tunnel!.\n");
			r_unlock(c->hash);
			goto error;
		}
		get_param(sec_hdr,s_ealg,ealg);
		get_param(sec_hdr,s_alg,alg);
		/* and for spis */
		get_param(sec_hdr,s_spi_c,tmp);
		strtoint(tmp,spi_pc);
		get_param(sec_hdr,s_spi_s,tmp);
		strtoint(tmp,spi_ps);
		/* and for ports */
		get_param(sec_hdr,s_port_c,tmp);
		strtoint(tmp,port_pc);
		get_param(sec_hdr,s_port_s,tmp);
		strtoint(tmp,port_ps);
		if ((s->data.ipsec->r_ealg.len != ealg.len || strncasecmp(s->data.ipsec->r_ealg.s, ealg.s, ealg.len)) ||
				(s->data.ipsec->r_alg.len != alg.len || strncasecmp(s->data.ipsec->r_alg.s, alg.s, alg.len)) || 
				(s->data.ipsec->spi_pc != spi_pc) ||
				(s->data.ipsec->spi_ps != spi_ps) ||
				(pcscf_ipsec_port_c != port_pc) ||
				(pcscf_ipsec_port_s != port_ps))
		{		
			LOG(L_INFO,"DBG:"M_NAME":P_verify_security: No valid Security-Verify header!.\n");
			r_unlock(c->hash);
			goto error;
		}
		break;
	}
	r_unlock(c->hash);
	
	return CSCF_RETURN_TRUE;
error:	
	return CSCF_RETURN_FALSE;
}

/**
 * Process the 401 response for REGISTER and creates the first Security-Associations.
 * IPSEc: Only the SA for P_Inc_Req - Incoming Requests is set now as the next REGISTER
 * could come over that one. 
 * @param rpl - the 401 response
 * @param str1 - not used
 * @param str2 - not used
 * @returns 1 if ok, 0 if not
 */
int P_security_401(struct sip_msg *rpl,char *str1, char *str2)
{
	struct sip_msg *req;
	struct hdr_field *hdr;	
	str sec_hdr,sec_srv={0,0};
	r_security_type sec_type;
	char cmd[256];
	r_contact *c;
	r_ipsec *ipsec;
	float sec_q=-1;
	str auth;

	if (!pcscf_use_ipsec &&!pcscf_use_tls) goto	ret_false;
	
	req = cscf_get_request_from_reply(rpl);
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":P_security_401: No transactional request found.\n");
		goto error;
	}
	auth = cscf_get_authenticate(rpl,&hdr);
	if (!auth.len){
		LOG(L_ERR,"ERR:"M_NAME":P_security_401: No WWW-Authenticate header found.\n");
		goto ret_false; 
	}
	
	sec_hdr = cscf_get_pref_security_header(req,s_security_client, &sec_type,&sec_q);
	if (!sec_hdr.len) {	
		LOG(L_DBG,"DBG:"M_NAME":P_security_401: No Security-Client header found.\n");
		goto ret_false;
	}
	LOG(L_INFO,"DBG:"M_NAME":P_security_401: Security-Client header found : <%.*s>.\n", sec_hdr.len, sec_hdr.s);	


	/* save data into registrar */
	c = save_contact_security(req, auth, sec_hdr, sec_type, sec_q);	
	if (!c) goto error;
	switch(sec_type){
		case SEC_NONE:
			break;
		case SEC_TLS:			
			/* try to add the Security-Server header */		
			sec_srv.len = s_security_server_s.len+sec_hdr.len+s_security_server_e.len;
			sec_srv.s = pkg_malloc(sec_srv.len);
			if (!sec_srv.s){
				LOG(L_ERR,"ERR:"M_NAME":P_security_401: Error allocating %d pkg bytes \n",sec_srv.len);
				goto error;
			}
			sec_srv.len=0;
			STR_APPEND(sec_srv,s_security_server_s);
			STR_APPEND(sec_srv,sec_hdr);
			STR_APPEND(sec_srv,s_security_server_e);

			if (!cscf_add_header(rpl,&sec_srv,HDR_OTHER_T)) {
				LOG(L_ERR,"ERR:"M_NAME":P_security_401: Error adding header <%.*s> \n",sec_srv.len,sec_srv.s);
				pkg_free(sec_srv.s);
				goto error;
			}
			break;
		case SEC_IPSEC:
			ipsec = c->security_temp->data.ipsec;
			/* try to add the Security-Server header */
			sprintf(cmd,"Security-Server: ipsec-3gpp; ealg=%.*s; alg=%.*s; spi-c=%d; spi-s=%d; port-c=%d; port-s=%d; q=0.1\r\n",
				ipsec->r_ealg.len,ipsec->r_ealg.s,
				ipsec->r_alg.len,ipsec->r_alg.s,
				ipsec->spi_pc,ipsec->spi_ps,
				pcscf_ipsec_port_c,pcscf_ipsec_port_s);
			
			sec_srv.len = strlen(cmd);
			sec_srv.s = pkg_malloc(sec_srv.len);
			if (!sec_srv.s){
				LOG(L_ERR,"ERR:"M_NAME":P_security_401: Error allocating %d pkg bytes \n",sec_srv.len);
				goto error;
			}
			memcpy(sec_srv.s,cmd,sec_srv.len);
			if (!cscf_add_header(rpl,&sec_srv,HDR_OTHER_T)) {
				LOG(L_ERR,"ERR:"M_NAME":P_security_401: Error adding header <%.*s> \n",sec_srv.len,sec_srv.s);
				pkg_free(sec_srv.s);
				goto error;
			}
	
			/* run the IPSec script */	
			/* P_Inc_Req */
			sprintf(cmd,"%s %.*s %d %s %d %d %.*s %.*s %.*s %.*s",
				pcscf_ipsec_P_Inc_Req,
				c->host.len,c->host.s,
				ipsec->port_uc,
				pcscf_ipsec_host,
				pcscf_ipsec_port_s,
				ipsec->spi_ps,
				ipsec->ealg.len,ipsec->ealg.s,
				ipsec->ck.len,ipsec->ck.s,
				ipsec->alg.len,ipsec->alg.s,
				ipsec->ik.len,ipsec->ik.s);

			r_unlock(c->hash);
				
			execute_cmd(cmd);
			break;						
	}
	
	return CSCF_RETURN_TRUE;
ret_false:
	return CSCF_RETURN_FALSE;
error:
	return CSCF_RETURN_ERROR;
}


/**
 * Process the 200 response for REGISTER and creates the first Security-Associations.
 * The rest of the SA are not set.
 * could come over that one. 
 * @param rpl - the 200 response
 * @param str1 - not used
 * @param str2 - not used
 * @returns 1 if ok, 0 if not
 */
int P_security_200(struct sip_msg *rpl,char *str1, char *str2)
{
	struct sip_msg *req;
	str sec_hdr;
	r_security_type sec_type;
	float sec_q;
	struct hdr_field *h;
	struct via_body *vb;
	r_contact *c;
	r_ipsec *i;
	int expires;
	unsigned long s_hash;
	char out_rpl[256],out_req[256],inc_rpl[256];

	if (!pcscf_use_ipsec &&!pcscf_use_tls) goto	ret_false;

	req = cscf_get_request_from_reply(rpl);
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":P_security_200: No transactional request found.\n");
		goto error;
	}	

	sec_hdr = cscf_get_pref_security_header(req,s_security_client, &sec_type,&sec_q);	
	if (!sec_hdr.len) {	
		LOG(L_DBG,"DBG:"M_NAME":P_security_200: No Security-Verify header found.\n");
		goto error;
	}
	
	/* find the expires (reg or dereg?) */
	expires = cscf_get_max_expires(req);
	
	/* get the IPSec info from the registrar */
	
	vb = cscf_get_first_via(req,&h);	
	LOG(L_DBG,"DBG:"M_NAME":P_security_200: Looking for <%d://%.*s:%d> \n",
		vb->proto,vb->host.len,vb->host.s,vb->port);

	c = get_r_contact(vb->host,vb->port,vb->proto);
		
	r_act_time();
	if (!c){
		LOG(L_ERR,"ERR:"M_NAME":P_security_200: Contact not found\n");		
		goto error;
	}
	
	if (c->security_temp){
		if (c->security && c->security->type == SEC_TLS && 
				(c->security->data.tls && c->security->data.tls->port_tls==req->rcv.src_port&& 
				 c->security->data.tls->session_hash!=0 && c->security->data.tls->session_hash == tls_get_session_hash(req))){
			/* don't replace security when doing an integrity protected REGISTER with
			 *  possible attack-garbage from security_temp */
			P_security_drop(c,c->security_temp);
			free_r_security(c->security_temp);
			c->security_temp = 0;
		}
		else
		{	
			if (c->security) {
				P_security_drop(c,c->security);
				free_r_security(c->security);
			}
			
			c->security = c->security_temp;
			c->security_temp = 0;
		}
	}	
	
	switch(sec_type){
		case SEC_NONE:
			break;
		case SEC_TLS:
			if (c->security && pcscf_use_tls) {
				r_tls *tls;
				int port_tls = req->rcv.src_port;
				s_hash = get_tls_session_hash(req);
				if (!s_hash){
					LOG(L_ERR,"ERR:"M_NAME":P_security_200: Session Hash could not be obtained !\n");
					r_unlock(c->hash);
					goto error;
				}	
				tls = new_r_tls(port_tls, s_hash);
				if (!tls) goto error;
				c->security->data.tls = tls;
			} 		
			r_unlock(c->hash);
			break;
		case SEC_IPSEC:
			if (!r_valid_contact(c)||!c->security||!c->security->data.ipsec ){
				LOG(L_DBG,"DBG:"M_NAME":P_security_200: Contact expired or no IPSec info\n");
				r_unlock(c->hash);
				goto error;
			}
			i = c->security->data.ipsec;
			
			/* P_Out_Rpl */
			sprintf(out_rpl,"%s %.*s %d %s %d %d %.*s %.*s %.*s %.*s",
				pcscf_ipsec_P_Out_Rpl,
				c->host.len,c->host.s,
				i->port_uc,
				pcscf_ipsec_host,
				pcscf_ipsec_port_s,
				i->spi_uc,
				i->ealg.len,i->ealg.s,
				i->ck.len,i->ck.s,
				i->alg.len,i->alg.s,
				i->ik.len,i->ik.s	);					
	
			/* P_Out_Req */
			sprintf(out_req,"%s %.*s %d %s %d %d %.*s %.*s %.*s %.*s",
				pcscf_ipsec_P_Out_Req,
				c->host.len,c->host.s,
				i->port_us,
				pcscf_ipsec_host,
				pcscf_ipsec_port_c,
				i->spi_us,
				i->ealg.len,i->ealg.s,
				i->ck.len,i->ck.s,
				i->alg.len,i->alg.s,
				i->ik.len,i->ik.s	);								
			/* P_Out_Inc_Rpl */
			sprintf(inc_rpl,"%s %.*s %d %s %d %d %.*s %.*s %.*s %.*s",
				pcscf_ipsec_P_Inc_Rpl,
				c->host.len,c->host.s,
				i->port_us,
				pcscf_ipsec_host,
				pcscf_ipsec_port_c,
				i->spi_pc,
				i->ealg.len,i->ealg.s,
				i->ck.len,i->ck.s,
				i->alg.len,i->alg.s,
				i->ik.len,i->ik.s	);								
				
			if (expires<=0) {
				/* Deregister */
				c->reg_state = DEREGISTERED;
				r_act_time();
				c->expires = time_now + 60;
			}			
			r_unlock(c->hash);
		
			//print_r(L_CRIT);
			
			/* run the IPSec scripts */	
			/* Registration */
			execute_cmd(out_rpl);		
			execute_cmd(out_req);		
			execute_cmd(inc_rpl);
			break;
	}
	
	return CSCF_RETURN_TRUE;
ret_false:
	return CSCF_RETURN_FALSE;
error:
	return CSCF_RETURN_ERROR;
}


/**
 * Drops the SAs for the given contact.
 * @param c - the contact to drop the SAs for
 * \note Should be called some time after the deregistration, to allow the 200 OK for 
 * De-REGISTER and other messages to still reach the client and responses to be relayed 
 * to the network.
 */
void P_security_drop(r_contact *c,r_security *s)
{
	char drop[256];
	r_ipsec *i;
	if (!s||!c) return;
	switch (s->type){
		case SEC_NONE:
			break;
		case SEC_TLS:
			//TODO!!!
			break;
		case SEC_IPSEC:
			i = s->data.ipsec;
			if (!i) return;
			sprintf(drop,"%s %.*s %d %d %s %d %d %d %d %d %d",
				pcscf_ipsec_P_Drop,
				c->host.len,c->host.s,
				i->port_uc,
				i->port_us,
				pcscf_ipsec_host,
				pcscf_ipsec_port_c,
				pcscf_ipsec_port_s,
				i->spi_uc,
				i->spi_us,
				i->spi_pc,
				i->spi_ps);		
			execute_cmd(drop);
			break;
	}
}


/**
 * Checks if the sent-by parameter in the first Via header equals the source IP address of the message
 * @param req - the SIP request
 * @param str1 - not used
 * @param str2 - not used
 * @returns true if ok, false if not or error
 */
int P_check_via_sent_by(struct sip_msg *msg,char *str1, char *str2)
{
	int ret = CSCF_RETURN_FALSE;
	struct ip_addr *src_ip;
	char *src_ip_ch;
	str sent_by={0,0};


	/* get the real receive IP address */
	src_ip = &(msg->rcv.src_ip);
	src_ip_ch = ip_addr2a(src_ip);
	LOG(L_DBG,"DBG:"M_NAME":P_check_sent_by(): Received from <%s>\n",src_ip_ch);			 

	/* find the sent-by Via parameter */
	sent_by = cscf_get_last_via_sent_by(msg);
	LOG(L_DBG,"DBG:"M_NAME":P_check_sent_by(): Via sent-by=<%.*s>\n",sent_by.len,sent_by.s);
			
	/* if not found, exit now */	
	if (sent_by.len == 0) {
		LOG(L_DBG,"DBG:"M_NAME":P_check_sent_by(): Via does not contain a sent-by value\n");
		return ret;
	}		
	
	/* if found, check if it is matching */
	if (sent_by.len==strlen(src_ip_ch) &&
		strncasecmp(sent_by.s,src_ip_ch,sent_by.len)==0){
			ret = CSCF_RETURN_TRUE;
			LOG(L_DBG,"DBG:"M_NAME":P_check_sent_by(): sent-by matches the actual IP received from\n");
	}else{
		ret = CSCF_RETURN_FALSE;
		LOG(L_DBG,"DBG:"M_NAME":P_check_sent_by(): sent-by does not match the actual IP received from\n");
	}	
	return ret;
}


/**
 * Checks if a response coming from a UE contains the same Via headers sent in the corresponding request
 * @param rpl - the SIP reply
 * @param str1 - not used
 * @param str2 - not used
 * @returns true if ok, false if not or error
 */
int P_follows_via_list(struct sip_msg *rpl,char *str1, char *str2)
{
	int h_req_pos, h_rpl_pos,indx_rpl, indx_req;
	struct hdr_field *h_req=NULL, *h_out_req, *h_rpl=NULL, *h_out_rpl;
	str via_req={0,0}, via_rpl={0,0};

	struct sip_msg *req = cscf_get_request_from_reply(rpl);	
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":P_follows_via_list: No transactional request found.\n");
		return CSCF_RETURN_ERROR;
	}	

	indx_rpl = indx_req = 0;
	//get via headers
	via_rpl = cscf_get_next_via_str(rpl, 0, 0, &h_out_rpl, &h_rpl_pos);
	while (via_rpl.len)
	{
		if (indx_rpl > 0) //first header from reply shouldn't be checked
		{
			if (indx_req == 0)
			{
				via_req = cscf_get_next_via_str(req, 0, 0, &h_out_req, &h_req_pos);
				if (!via_req.len || !cscf_str_via_matching(&via_req, &via_rpl))
				{
					LOG(L_INFO,"DBG:"M_NAME":P_follows_via_list: first via not matching <%.*s>!=<%.*s>\n",
						via_req.len,via_req.s,via_rpl.len,via_rpl.s);
					return CSCF_RETURN_FALSE; 
				}
			}
			else
			{
				if (!via_req.len || (via_req.len!=via_rpl.len) || (strncasecmp(via_req.s, via_rpl.s, via_req.len)))
				{
					LOG(L_INFO,"DBG:"M_NAME":P_follows_via_list: not matching <%.*s>!=<%.*s>\n",
						via_req.len,via_req.s,via_rpl.len,via_rpl.s);
					return CSCF_RETURN_FALSE;
				}
			}
			indx_req++;
			h_req = h_out_req;
			if (!h_req)
			{
				break;
			}
			via_req = cscf_get_next_via_str(req, h_req, h_req_pos , &h_out_req, &h_req_pos);
		}
		indx_rpl++;
		h_rpl = h_out_rpl;
		if (!h_rpl) break;
		via_rpl = cscf_get_next_via_str(rpl, h_rpl, h_rpl_pos , &h_out_rpl, &h_rpl_pos);
	}

	if (h_out_req || h_out_rpl) //remaining headers ...
	{ 
		LOG(L_INFO,"DBG:"M_NAME":P_follows_via_list: header count not matching \n");
		return CSCF_RETURN_FALSE;
	}
	return CSCF_RETURN_TRUE;
}

static str via_hdr_s={"Via: ",5};
static str via_hdr_e={"\r\n",2};
/**
 * enforce a response coming from a UE to contain the same Via headers sent in the corresponding request
 * @param rpl - the SIP reply
 * @param str1 - not used
 * @param str2 - not used
 * @returns true if ok, false if not or error
 */
int P_enforce_via_list(struct sip_msg *rpl,char *str1, char *str2)
{
	static struct hdr_field * h = NULL;
	str hdr;

	cscf_del_all_headers(rpl, HDR_VIA_T);

	struct sip_msg *req = cscf_get_request_from_reply(rpl);
	
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":P_enforce_via_list: No transactional request found.\n");
		return CSCF_RETURN_FALSE;
	
	}

	h = cscf_get_next_via_hdr(req,0);
	while (h)
	{
		hdr.len = h->body.len + via_hdr_s.len + via_hdr_e.len;
		hdr.s = pkg_malloc(hdr.len);
		if (!hdr.s)
		{
			LOG(L_ERR, "ERR:"M_NAME":P_enforce_via_list: cannot alloc bytes : %d", hdr.len);
		}
		hdr.len=0;
		STR_APPEND(hdr, via_hdr_s);
		STR_APPEND(hdr, h->body);
		STR_APPEND(hdr, via_hdr_e);
		cscf_add_header_first(rpl, &hdr, HDR_VIA_T);
		h = cscf_get_next_via_hdr(req,h);
	}
	
	return CSCF_RETURN_TRUE;
}

static str s_received={";received=",10};
static str s_received2={"received",8};
/**
 * Adds a received parameter to the first via with the source IP of the message.
 * @param req - the SIP request
 * @param str1 - not used
 * @param str2 - not used
 * @returns true if ok, false if not or error
 */
int P_add_via_received(struct sip_msg *msg,char *str1, char *str2)
{
	struct via_body *via;
	struct via_param *vp;
	str received={0,0};
	struct ip_addr *src_ip;
	char *src_ip_ch;
	int len;
	struct lump* anchor,*l;
	char *x;
	
	/* first add a received parameter */
	via = msg->via1;
	
	/* if we already have a received header, SER will take care of it and put the right value */	
	if (via->received) goto delete_others;
	
	x = via->port_str.s+via->port_str.len;
	if (!x) x= via->host.s+via->host.len;
	anchor = anchor_lump(msg, x-msg->buf, 0 , 0 );
	if (anchor == NULL) {
		LOG(L_ERR, "ERR:"M_NAME":P_add_via_received(): anchor_lump failed\n");
		return 0;
	}
	
	src_ip = &(msg->rcv.src_ip);
	src_ip_ch = ip_addr2a(src_ip);
	len = strlen(src_ip_ch);
	received.len = s_received.len+len;	
	received.s = pkg_malloc(received.len);
	
	if (!received.s){
		LOG(L_ERR, "ERR:"M_NAME":P_add_via_received(): allocating %d bytes\n",received.len);
		return CSCF_RETURN_ERROR;
	}
	memcpy(received.s,s_received.s,s_received.len);
	memcpy(received.s+s_received.len,src_ip_ch,len);
	
	if (!(l=insert_new_lump_before(anchor, received.s,received.len,0))){
		LOG(L_ERR, "ERR:"M_NAME":P_add_via_received(): error creating lump for received parameter\n" );
		return CSCF_RETURN_ERROR;
	}	
	
	/* then remove the old received params*/
delete_others:	
	for(vp = via->param_lst; vp; vp = vp->next)
		if (vp->name.len == s_received2.len &&
			strncasecmp(vp->name.s,s_received2.s,s_received2.len)==0){
				LOG(L_ERR, "ERR:"M_NAME":P_add_via_received(): Found old received parameter!! This might indicate an attack.\n" );
				if (!del_lump(msg,vp->start-msg->buf-1,vp->size+1,0)){
					LOG(L_ERR,"ERR:"M_NAME":P_add_via_received(): Error deleting old received parameter from first via\n");
					return CSCF_RETURN_ERROR;		
				}		
			}	
	return CSCF_RETURN_TRUE;
}


/**
 * Remove from <str1> headers the <str2> tag
 * \note Does not work if you call it multiple times for the same hdr_name!
 * \note Because of the note above, this needs a fix to accept multiple tags in the str2 parameter!
 * @param msg - SIP Request
 * @param str1 - the header to remove from
 * @param str2 - the tag to remove
 * @returns #CSCF_RETURN_TRUE if removed, #CSCF_RETURN_FALSE if no changes required or #CSCF_RETURN_FALSE on error
 */
int P_remove_header_tag(struct sip_msg *msg,char *str1, char *str2)
{
	str hdr_name,tag,x,y;
	struct hdr_field *hdr;
	char *c;
	int found,i,j,changed=0;
	
	hdr_name.s = str1;
	hdr_name.len = strlen(str1);

	tag.s = str2;
	tag.len = strlen(str2);
	
	hdr = cscf_get_header(msg,hdr_name);
	while (hdr){
		/* get the original body */
		x = hdr->body;
		LOG(L_INFO,"DBG:"M_NAME":P_remove_header_tag(): Original <%.*s> -> <%.*s>\n",
			hdr_name.len,hdr_name.s,x.len,x.s);
		
		/* duplicate the original body */
		x.len++;
		STR_PKG_DUP(y,x,"P_remove_header_tag");
		if (!y.s) goto error;
		y.len--;
		y.s[y.len]=0;
		
		/* look for occurences of tag and overwrite with \0 */
		found=0;
		c = strtok(y.s," \t\r\n,");
		while (c){
			if (strlen(c)==tag.len && strncasecmp(c,tag.s,tag.len)==0){
				found++;
				memset(c,0,tag.len);
			}	
			c = strtok(0," \t\r\n,");
		}
		
		/* if not found just skip to next header */
		if (!found) goto next;
				
		/* compact the remaining tags by removing the \0 */		
		for(i=0,j=0;i<y.len;i++)
			if (y.s[i]!=0){ 
				y.s[j++]=y.s[i];
			} else {
				if (j!=0) y.s[j++]=',';
				while(i+1<y.len && y.s[i+1]==0)
					i++;
			}
		y.len = j;
		if (y.s[y.len-1]==',') y.len--;

		LOG(L_INFO,"DBG:"M_NAME":P_remove_header_tag(): Modified <%.*s> -> <%.*s>\n",
			hdr_name.len,hdr_name.s,y.len,y.s);

		/* write the changes */
		if (y.len){
			/* replace just the content */
			if (!cscf_replace_string(msg,x,y)){
				LOG(L_ERR,"ERR:"M_NAME":P_remove_header_tag(): Error replacing string!\n");
				if (y.s) pkg_free(y.s);
				goto error;
			}
		}else{
			/* remove the whole header */
			if (y.s) pkg_free(y.s);
			if (!cscf_del_header(msg,hdr)){
				LOG(L_ERR,"ERR:"M_NAME":P_remove_header_tag(): Error removing the whole header!\n");
				goto error;
			}
		}
		changed++;
		
next:
		hdr = cscf_get_next_header(msg,hdr_name,hdr);
	}	

	return changed?CSCF_RETURN_TRUE:CSCF_RETURN_FALSE;
error:
out_of_memory:
	return CSCF_RETURN_ERROR;
}


/**
 * Look for Security-Client and delete it if found.
 * @param msg - the SIP message to add to
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not found or #CSCF_RETURN_FALSE on error
 */
int P_remove_security_client(struct sip_msg *msg,char *str1,char*str2)
{
	struct hdr_field *hdr=0;	

	for(hdr=cscf_get_next_header(msg,s_security_client,0);hdr;hdr=cscf_get_next_header(msg,s_security_client,hdr)){
		if (!cscf_del_header(msg,hdr)){
			LOG(L_INFO,"INF:"M_NAME":P_drop_security_client: Error dropping Security-Client header.\n");		
			return CSCF_RETURN_ERROR; 
		}		
	}
	return CSCF_RETURN_TRUE;
}

/**
 * Look for Security-Verify and delete it if found.
 * @param msg - the SIP message to add to
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not found or #CSCF_RETURN_FALSE on error
 */
int P_remove_security_verify(struct sip_msg *msg,char *str1,char*str2)
{
	struct hdr_field *hdr=0;	

	for(hdr=cscf_get_next_header(msg,s_security_verify,0);hdr;hdr=cscf_get_next_header(msg,s_security_verify,hdr)){		
		if (!cscf_del_header(msg,hdr)){
			LOG(L_INFO,"INF:"M_NAME":P_remove_security_verify: Error dropping Security-Client header.\n");		
			return CSCF_RETURN_ERROR; 
		}
	}
	return CSCF_RETURN_TRUE;
}

/**
 * Removes the Security-Client,Security-Verify headers.
 * @param msg - the SIP message to remove from
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not found or #CSCF_RETURN_FALSE on error
 */
int P_remove_security_headers(struct sip_msg *msg,char *str1,char*str2)
{
	int r1,r2; r1 = r2 = CSCF_RETURN_FALSE;

	r1=P_remove_security_client(msg,str1,str2);
	if (r1==CSCF_RETURN_ERROR) return r1;
	
	r2=P_remove_security_verify(msg,str1,str2);
	if (r2==CSCF_RETURN_ERROR) return r2;

	if (r1==CSCF_RETURN_TRUE&&r2==CSCF_RETURN_TRUE) return CSCF_RETURN_TRUE;
	else return CSCF_RETURN_FALSE;
}
