/*
 * $Id: security.c 223 2007-04-12 10:14:14Z vingarzan $
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
 * \todo Add TLS support
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include <signal.h> 
 
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

/**
 * Saves the IPSec information into the P-CSCF registrar for this contact.
 * @param req - REGISTER request
 * @param rpl - REGISTER respons
 * @param spi_uc - SPI for UE client
 * @param spi_us - SPI for UE server
 * @param spi_pc - SPI for PCSCF client
 * @param spi_ps - SPI for PCSCF server
 * @param port_uc - port for UE client
 * @param port_us - port for UE server
 * @param ealg_setkey - Chypher algorithm
 * @param ck_esp - Cypher Key
 * @param alg_setkey - Integrity algorithm
 * @param ik_esp - Integrity Key
 */
void save_contact_ipsec(struct sip_msg *req,struct sip_msg *rpl,
	int spi_uc,int spi_us,int spi_pc,int spi_ps,int port_uc,int port_us,
	str ealg_setkey,str ck_esp,str alg_setkey,str ik_esp)
{
	contact_t* c=0;
	contact_body_t* b=0;	
	r_contact *rc;
	enum Reg_States reg_state=REG_PENDING;
	int expires,pending_expires=60;
	struct sip_uri puri;
	r_ipsec *ipsec;

	
	if (parse_headers(rpl, HDR_EOH_F, 0) <0) {
		LOG(L_ERR,"ERR:"M_NAME":save_contact_ipsec: error parsing headers\n");
		return ;
	}	
	
	b = cscf_parse_contacts(req);
	
	if (!b||!b->contacts) {
		LOG(L_ERR,"ERR:"M_NAME":save_contact_ipsec: No contacts found\n");
		return;
	}
	
	if (b) c = b->contacts;
			
	r_act_time();
	while(c){
		LOG(L_DBG,"DBG:"M_NAME":save_contact_ipsec: <%.*s>\n",c->uri.len,c->uri.s);
		
		expires = time_now+pending_expires;
		
		if (parse_uri(c->uri.s,c->uri.len,&puri)<0){
			LOG(L_DBG,"DBG:"M_NAME":save_contact_ipsec: Error parsing Contact URI <%.*s>\n",c->uri.len,c->uri.s);
			goto next;			
		}
		if (puri.port_no==0) puri.port_no=5060;
		LOG(L_DBG,"DBG:"M_NAME":save_contact_ipsec: %d %.*s : %d\n",
			puri.proto, puri.host.len,puri.host.s,puri.port_no);

		ipsec = new_r_ipsec(spi_uc,spi_us,spi_pc,spi_ps,port_uc,port_us,
			ealg_setkey,ck_esp,alg_setkey,ik_esp);
			
		rc = update_r_contact_sec(puri.host,ipsec->port_us,puri.proto,
			&(c->uri),&reg_state,&expires,
			ipsec);

next:		
		c = c->next;
	}
	
	//print_r(L_CRIT);
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
 * Process the 401 response for REGISTER and creates the first Security-Associations.
 * Only the SA for P_Inc_Req - Incoming Requests is set now as the next REGISTER
 * could come over that one. 
 * @param rpl - the 401 response
 * @param str1 - not used
 * @param str2 - not used
 * @returns 1 if ok, 0 if not
 */
int P_IPSec_401(struct sip_msg *rpl,char *str1, char *str2)
{
	struct sip_msg *req;
	str auth;
	struct hdr_field *hdr;	
	str sec_cli,sec_srv={0,0};
	str ck,ik;
	char ck_c[64],ik_c[64];
	str ck_esp={ck_c,0},ik_esp={ik_c,0};
	str alg,ealg;
	str alg_setkey,ealg_setkey;
	str tmp;
	unsigned int spi_uc,spi_us;
	unsigned int spi_pc,spi_ps;
	int port_uc,port_us;
	char cmd[256];
	str ue;
	

	contact_body_t *contact;
	struct sip_uri uri;
	
	req = cscf_get_request_from_reply(rpl);
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":P_ipsec_401: No transactional request found.\n");
		goto error;
	}
	auth = cscf_get_authenticate(rpl,&hdr);
	if (!auth.len){
		LOG(L_ERR,"ERR:"M_NAME":P_ipsec_401: No authorization header found.\n");
		goto error; 
	}
	sec_cli = cscf_get_security_client(req,&hdr);
	if (!sec_cli.len){
		LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: No Security-Client header found.\n");
		goto error; 
	}
	
	/* first we look for CK, IK */
	get_qparam(auth,s_ck,ck);
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: CK: <%.*s>\n",
		ck.len,ck.s);
	get_qparam(auth,s_ik,ik);
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: IK: <%.*s>\n",
		ik.len,ik.s);
	
	/* then for algorithms */
	get_param(sec_cli,s_ealg,ealg);
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: Enc Algorithm: <%.*s>\n",
		ealg.len,ealg.s);
	get_param(sec_cli,s_alg,alg);
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: Int Algorithm: <%.*s>\n",
		alg.len,alg.s);
	/* and for spis */
	get_param(sec_cli,s_spi_c,tmp);
	strtoint(tmp,spi_uc);
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: SPI-C: %d\n",
		spi_uc);
	get_param(sec_cli,s_spi_s,tmp);
	strtoint(tmp,spi_us);
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: SPI-S: %d\n",
		spi_us);
	/* and for ports */
	get_param(sec_cli,s_port_c,tmp);
	strtoint(tmp,port_uc);
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: Port-C: %d\n",
		port_uc);
	get_param(sec_cli,s_port_s,tmp);
	strtoint(tmp,port_us);
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: Port-S: %d\n",
		port_us);
	
	
	contact = cscf_parse_contacts(req);
	if (!contact) {		
		LOG(L_ERR,"ERR:"M_NAME":P_ipsec_401: Message contains no Contact!\n");
		goto error;
	}
	if (contact->contacts){
		ue = contact->contacts->uri;
		if (parse_uri(ue.s,ue.len,&uri)){
			LOG(L_ERR,"ERR:"M_NAME":P_ipsec_401: Error parsing uri <%.*s>\n",ue.len,ue.s);
			goto error;
		}
		ue = uri.host;
		if (uri.port_no==0) port_uc=5060;
	}else
		ue = req->via1->host;
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_401: UE IP: <%.*s> \n",ue.len,ue.s);
	
	spi_pc=get_next_spi();	
	spi_ps=get_next_spi();	

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
		LOG(L_ERR,"ERR:"M_NAME":P_ipsec_401: Unknown Integrity algorithm <%.*s>\n",alg.len,alg.s);
		goto error;
	}

	/* try to add the Security-Server header */
	sprintf(cmd,"Security-Server: ipsec-3gpp; ealg=%.*s; alg=%.*s; spi-c=%d; spi-s=%d; port-c=%d; port-s=%d; q=0.1\r\n",
		ealg.len,ealg.s,
		alg.len,alg.s,
		spi_pc,spi_ps,
		pcscf_ipsec_port_c,pcscf_ipsec_port_s);
		
	sec_srv.len = strlen(cmd);
	sec_srv.s = pkg_malloc(sec_srv.len);
	if (!sec_srv.s){
		LOG(L_ERR,"ERR:"M_NAME":P_ipsec_401: Error allocating %d pkg bytes \n",sec_srv.len);
		goto error;
	}
	memcpy(sec_srv.s,cmd,sec_srv.len);
	if (!cscf_add_header(rpl,&sec_srv,HDR_OTHER_T)) {
		LOG(L_ERR,"ERR:"M_NAME":P_ipsec_401: Error adding header <%.*s> \n",sec_srv.len,sec_srv.s);
		pkg_free(sec_srv.s);
		goto error;
	}

	/* run the IPSec script */	
	/* P_Inc_Req */
	sprintf(cmd,"%s %.*s %d %s %d %d %.*s %.*s %.*s %.*s",
		pcscf_ipsec_P_Inc_Req,
		ue.len,ue.s,
		port_uc,
		pcscf_ipsec_host,
		pcscf_ipsec_port_s,
		spi_ps,
		ealg_setkey.len,ealg_setkey.s,
		ck_esp.len,ck_esp.s,
		alg_setkey.len,alg_setkey.s,
		ik_esp.len,ik_esp.s);
	execute_cmd(cmd);
			
	/* save data into registrar */
	save_contact_ipsec(req,rpl,
		spi_uc,spi_us,spi_pc,spi_ps,port_uc,port_us,
		ealg_setkey,ck_esp,alg_setkey,ik_esp);
	
	
	return 1;	
error:
	return 0;	
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
int P_IPSec_200(struct sip_msg *rpl,char *str1, char *str2)
{
	struct sip_msg *req;
	struct hdr_field *hdr;	
	str sec_cli;
	struct hdr_field *h;
	struct via_body *vb;
	r_contact *c;
	r_ipsec *i;
	int expires;
	char out_rpl[256],out_req[256],inc_rpl[256];

	req = cscf_get_request_from_reply(rpl);
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":P_ipsec_200: No transactional request found.\n");
		goto error;
	}	
	/* just to jump out if no IPSec is employed - the info is already saved */
	sec_cli = cscf_get_security_client(req,&hdr);
	if (!sec_cli.len){
		LOG(L_DBG,"DBG:"M_NAME":P_ipsec_200: No Security-Client header found.\n");
		goto error; 
	}

	/* find the expires (reg or dereg?) */
	expires = cscf_get_max_expires(req);
	
	/* get the IPSec info from the registrar */
	
	vb = cscf_get_first_via(req,&h);
//	if (!h||!h->parsed){
//		LOG(L_ERR,"ERR:"M_NAME":P_ipsec_200: Error extracting sender's id.\n");
//		goto error;
//	}		
//	vb = (struct via_body*) h->parsed;
	
	LOG(L_DBG,"DBG:"M_NAME":P_ipsec_200: Looking for <%d://%.*s:%d> \n",
		vb->proto,vb->host.len,vb->host.s,vb->port);

	c = get_r_contact(vb->host,vb->port,vb->proto);
		
	r_act_time();
	if (!c){
		LOG(L_ERR,"ERR:"M_NAME":P_ipsec_200: Contact not found\n");		
		goto error;
	}
	if (!r_valid_contact(c)||!c->ipsec){
		LOG(L_DBG,"DBG:"M_NAME":P_ipsec_200: Contact expired or no IPSec info\n");
		r_unlock(c->hash);
		goto error;
	}
	
	i = c->ipsec;
		
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
	return 1;	
error:
	return 0;	
}


/**
 * Drops the SAs for the given contact.
 * @param c - the contact to drop the SAs for
 * \note Should be called some time after the deregistration, to allow the 200 OK for 
 * De-REGISTER and other messages to still reach the client and responses to be relayed 
 * to the network.
 */
void P_drop_ipsec(r_contact *c)
{
	char drop[256];
	if (!c) return;
	r_ipsec *i = c->ipsec;
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
	LOG(L_INFO,"DBG:"M_NAME":P_check_sent_by(): Received from <%s>\n",src_ip_ch);			 

	/* find the sent-by Via parameter */
	sent_by = cscf_get_last_via_sent_by(msg);
	LOG(L_INFO,"DBG:"M_NAME":P_check_sent_by(): Via sent-by=<%.*s>\n",sent_by.len,sent_by.s);
			
	/* if not found, exit now */	
	if (sent_by.len == 0) {
		LOG(L_INFO,"DBG:"M_NAME":P_check_sent_by(): Via does not contain a sent-by parameter\n");
		return ret;
	}		
	
	/* if found, check if it is matching */
	if (sent_by.len==strlen(src_ip_ch) &&
		strncasecmp(sent_by.s,src_ip_ch,sent_by.len)==0){
			ret = CSCF_RETURN_TRUE;
			LOG(L_INFO,"DBG:"M_NAME":P_check_sent_by(): sent-by matches the actual IP received from\n");
	}else{
		ret = CSCF_RETURN_FALSE;
		LOG(L_INFO,"DBG:"M_NAME":P_check_sent_by(): sent-by does not match the actual IP received from\n");
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
