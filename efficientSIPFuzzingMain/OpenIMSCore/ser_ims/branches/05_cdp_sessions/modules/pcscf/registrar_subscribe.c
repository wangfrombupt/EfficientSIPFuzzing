/*
 * $Id: registrar_subscribe.c 439 2007-08-28 10:26:33Z albertoberlios $
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
 * Proxy-CSCF - Registrar Refreshment Through SUBSCRIBE to reg event at the S-CSCF
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "registrar_subscribe.h"

#include <libxml/xmlschemas.h>
#include <libxml/parser.h>
 

#include "registrar_storage.h"
#include "mod.h" 
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../parser/parse_uri.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../dialog/dlg_mod.h"
#include "sip.h"

extern struct tm_binds tmb;   		/**< Structure with pointers to tm funcs 		*/
extern dlg_func_t dialogb;			/**< Structure with pointers to dialog funcs			*/

extern str pcscf_name_str;			/**< fixed SIP URI of this P-CSCF 				*/
extern str pcscf_path_str;			/**< fixed Path URI  							*/

extern time_t time_now;				/**< current time 								*/

extern r_hash_slot *registrar;		/**< the actual registrar						*/
extern int r_hash_size;				/**< number of hash slots in the registrar		*/

extern int pcscf_subscribe_retries;	/**< times to retry subscribe to reg on failure */

r_subscription_hash_slot *subscriptions=0;/**< list of subscriptions					*/
extern int subscriptions_hash_size;	/**< the size of the hash table for subscriptions	*/


/**
 * Computes the hash for a contact.
 * @param aor - the string of the contact
 * @param port - the port of the contact
 * @param transport - transport for the contact - ignored for now
 * @param hash_size - size of the hash, to % with
 * @returns the hash for the contact
 */
inline unsigned int get_subscription_hash(str uri)
{
#define h_inc h+=v^(v>>3)
   char* p;
   register unsigned v;
   register unsigned h;
   h=0;
   for (p=uri.s; p<=(uri.s+uri.len-4); p+=4){
       v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
       h_inc;
   }
   v=0;
   for (;p<(uri.s+uri.len); p++) {
       v<<=8;
       v+=*p;
   }
   h_inc;
   h=((h)+(h>>11))+((h>>13)+(h>>23));
   return (h)%subscriptions_hash_size;
#undef h_inc 
}

/**
 * Lock a subscription hash slot.
 * @param hash - index to lock
 */
inline void subs_lock(unsigned int hash)
{
//	LOG(L_CRIT,"GET %d\n",hash);
	lock_get(subscriptions[hash].lock);
//	LOG(L_CRIT,"GOT %d\n",hash);
}
/**
 * UnLock a subscriptions hash slot.
 * @param hash - index to unlock
 */
inline void subs_unlock(unsigned int hash)
{
//	LOG(L_CRIT,"REL %d\n",hash);	
	lock_release(subscriptions[hash].lock);
}
/**
 * Initialize the subscription list.
 * @returns 1 if ok, 0 on error
 */
int r_subscription_init()
{
	int i;
	subscriptions = shm_malloc(sizeof(r_subscription_hash_slot)*subscriptions_hash_size);
	if (!subscriptions) return 0;
	memset(subscriptions,0,sizeof(r_subscription_hash_slot)*subscriptions_hash_size);
	for(i=0;i<subscriptions_hash_size;i++){
		subscriptions[i].lock = lock_alloc();
		if (!subscriptions[i].lock) return 0;
		subscriptions[i].lock = lock_init(subscriptions[i].lock);
	}
	return 1;
}

/**
 * Destroys the subscription list
 */
void r_subscription_destroy()
{
	int i;
	r_subscription *s,*ns;
	for(i=0;i<subscriptions_hash_size;i++){
		subs_lock(i);
		s = subscriptions[i].head;
		while(s){
			ns = s->next;
			//TODO send out unSUBSCRIBE
			free_r_subscription(s);
			s = ns;
		}
		lock_destroy(subscriptions[i].lock);
		lock_dealloc(subscriptions[i].lock);
	}	
	shm_free(subscriptions);
}


/**
 * Subscribe to the reg event to the S-CSCF.
 * @param rpl - 200 OK response to REGISTER containing contacts and Service-Route header
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if subscribed, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_ERROR on error
 */
int P_subscribe(struct sip_msg *rpl, char* str1, char* str2)
{
	int expires_hdr=0,r,max_expires;
	str public_id={0,0};
	contact_t* c=0;
	contact_body_t* b=0;	
	
	cscf_get_first_p_associated_uri(rpl,&public_id);
	
	expires_hdr = cscf_get_expires_hdr(rpl);
	
	if (parse_headers(rpl, HDR_EOH_F, 0) <0) {
		LOG(L_ERR,"ERR:"M_NAME":P_subscribe: error parsing headers\n");
		return CSCF_RETURN_ERROR;
	}	
	
	b = cscf_parse_contacts(rpl);
	
	if (!b||!b->contacts) {
		LOG(L_DBG,"DBG:"M_NAME":P_subscribe: No contacts found\n");
		return CSCF_RETURN_FALSE;
	}
	
	if (b) c = b->contacts;
	
	max_expires = expires_hdr;
	while(c){
		r = expires_hdr;
		if (str2int(&(c->expires->body), (unsigned int*)&r) < 0) {
			r = 0;
		}
		if (r>max_expires) max_expires = r;
		c = c->next;
	}
	if (max_expires<=0){
		LOG(L_INFO,"DBG:"M_NAME":P_subscribe: skipped because de-register\n");		
		return CSCF_RETURN_FALSE;		
	}
	
	if (public_id.s){
//		if (max_expires==0)
//			r_subscribe(public_id,0);
//		else
		r_subscribe(public_id,max_expires+30);
		return CSCF_RETURN_TRUE;
	}else{
		return CSCF_RETURN_FALSE;
	}
}

/**
 * Creates a subcription and starts the timer resubscription for the given contact.
 * @param uri - the contact to subscribe to (actually to its default public id)
 * @param duration - SUBCRIBE expires
 * @returns 1 on success, 0 on failure
 */
int r_subscribe(str uri,int duration)
{
	r_subscription *s;
	/* first we try to update. if not found, add it */
	s = get_r_subscription(uri);	
	if (s){
		s->duration = duration;
		s->attempts_left=pcscf_subscribe_retries;
		subs_unlock(s->hash);
	}else{			
		s = new_r_subscription(uri,duration);
		if (!s){
			LOG(L_ERR,"ERR:"M_NAME":r_subscribe: Error creating new subscription\n");
			return 0;
		}
		add_r_subscription(s);
		s->attempts_left=pcscf_subscribe_retries;
	}
		
	return 1;
}


static str method={"SUBSCRIBE",9};
static str event_hdr={"Event: reg\r\n",12};
static str accept_hdr={"Accept: application/reginfo+xml\r\n",33};
static str content_len_hdr={"Content-Length: 0\r\n",19};
static str max_fwds_hdr={"Max-Forwards: 10\r\n",18};
static str expires_s={"Expires: ",9};
static str expires_e={"\r\n",2};
static str contact_s={"Contact: <",10};
static str contact_e={">\r\n",3};
static str p_asserted_identity_s={"P-Asserted-Identity: <",22};
static str p_asserted_identity_e={">\r\n",3};
/**
 * Send a subscription
 * @param s - the subsription to send for
 * @param duration - expires time
 * @returns true if OK, false if not, error on failure
 * \todo store the dialog and reSubscribe on the same dialog
 */
int r_send_subscribe(r_subscription *s,int duration)
{
	str h={0,0};

	LOG(L_DBG,"DBG:"M_NAME":r_send_subscribe: SUBSCRIBE to <%.*s>\n",
		s->req_uri.len,s->req_uri.s);
	
	h.len = event_hdr.len+accept_hdr.len+content_len_hdr.len+max_fwds_hdr.len;
	h.len += expires_s.len + 12 + expires_e.len;

	h.len += contact_s.len + pcscf_name_str.len + contact_e.len;
	if (pcscf_path_str.len) h.len += p_asserted_identity_s.len + 
		p_asserted_identity_e.len + pcscf_path_str.len;

	h.s = pkg_malloc(h.len);
	if (!h.s){
		LOG(L_ERR,"ERR:"M_NAME":r_send_subscribe: Error allocating %d bytes\n",h.len);
		h.len = 0;
		return 0;
	}

	h.len = 0;
	STR_APPEND(h,event_hdr);
	STR_APPEND(h,accept_hdr);
	STR_APPEND(h,content_len_hdr);
	STR_APPEND(h,max_fwds_hdr);

	STR_APPEND(h,expires_s);
	sprintf(h.s+h.len,"%d",duration);
	h.len += strlen(h.s+h.len);
	STR_APPEND(h,expires_e);

	STR_APPEND(h,contact_s);
	STR_APPEND(h,pcscf_name_str);
	STR_APPEND(h,contact_e);
	
	if (pcscf_path_str.len) {
		STR_APPEND(h,p_asserted_identity_s);
		STR_APPEND(h,pcscf_path_str);
		STR_APPEND(h,p_asserted_identity_e);
	}
	
	if (!s->dialog){
		/* this is the first request in the dialog */
		if (tmb.new_dlg_uac(0,0,1,&pcscf_name_str,&s->req_uri,&s->dialog)<0){
			LOG(L_ERR,"ERR:"M_NAME":r_send_subscribe: Error creating a dialog for SUBSCRIBE\n");
			goto error;
		}
		if (dialogb.request_outside(&method, &h, 0, s->dialog, r_subscribe_response,  &(s->req_uri)) < 0){
			LOG(L_ERR,"ERR:"M_NAME":r_send_subscribe: Error sending initial request in a SUBSCRIBE dialog\n");
			goto error;
		}		
	}else{
		/* this is a subsequent subscribe */
		if (dialogb.request_inside(&method, &h, 0, s->dialog, r_subscribe_response,  &(s->req_uri)) < 0){
			LOG(L_ERR,"ERR:"M_NAME":r_send_subscribe: Error sending subsequent request in a SUBSCRIBE dialog\n");
			goto error;
		}				
	}

	if (h.s) pkg_free(h.s);
	return 1;

error:
	if (h.s) pkg_free(h.s);
	return 0;
}

/**
 * Response callback for subscribe
 */
void r_subscribe_response(struct cell *t,int type,struct tmcb_params *ps)
{
	str req_uri;
	int expires;
	r_subscription *s=0;
	LOG(L_DBG,"DBG:"M_NAME":r_subscribe_response: code %d\n",ps->code);
	if (!ps->rpl) {
		LOG(L_ERR,"INF:"M_NAME":r_subscribe_response: No reply\n");
		return;	
	}
	req_uri = *((str*) *(ps->param));		
	s = get_r_subscription(req_uri);
	if (!s){
		LOG(L_ERR,"INF:"M_NAME":r_subscribe_response: received a SUBSCRIBE response but no subscription for <%.*s>\n",
			req_uri.len,req_uri.s);
		return;
	}
	if (ps->code>=200 && ps->code<300){
		expires = cscf_get_expires_hdr(ps->rpl);
		update_r_subscription(s,expires);
		tmb.dlg_response_uac(s->dialog, ps->rpl, IS_TARGET_REFRESH);
	}else
		if (ps->code==404){
			update_r_subscription(s,0);			
			//tmb.dlg_response_uac(s->dialog, ps->rpl, IS_TARGET_REFRESH);
		}else{
			LOG(L_INFO,"INF:"M_NAME":r_subscribe_response: SUBSCRIRE response code %d ignored\n",ps->code);				
		}	
	if (s) subs_unlock(s->hash);		
}

/**
 * The Subscription timer looks for almost expired subscriptions and subscribes again.
 * @param ticks - the current time
 * @param param - the generic parameter
 */
void subscription_timer(unsigned int ticks, void* param)
{
	r_subscription *s,*ns;
	int i;
	for(i=0;i<subscriptions_hash_size;i++){
		subs_lock(i);
		s = subscriptions[i].head;
		r_act_time();
		while(s){
			ns = s->next;			
			if (s->attempts_left > 0 ){
				/* attempt to send a subscribe */
				if (!r_send_subscribe(s,s->duration)){
					LOG(L_ERR,"ERR:"M_NAME":subscription_timer: Error on SUBSCRIBE (%d times)... droping\n",
						pcscf_subscribe_retries);
					del_r_subscription_nolock(s);
				}else{
					s->attempts_left--;
				}
			}else if (s->attempts_left==0) {
				/* we failed to many times, drop the subscription */
				LOG(L_ERR,"ERR:"M_NAME":subscription_timer: Error on SUBSCRIBE for %d times... aborting\n",pcscf_subscribe_retries);
				del_r_subscription_nolock(s);										
			}else{
				/* we are subscribed already */
				/* if expired, drop it */
				if (s->expires<time_now) 
					del_r_subscription_nolock(s);
				/* if not expired, check for renewal */
//		Commented as the S-CSCF should adjust the subscription time accordingly				
//				if ((s->duration<1200 && s->expires-time_now<s->duration/2)||
//					(s->duration>=1200 && s->expires-time_now<600))
//				{
//					/* if we need a resubscribe, we mark it as such and try to subscribe again */					
//					s->attempts_left = pcscf_subscribe_retries;
//					ns = s;
//				}
			}
			s = ns;
		}	
		subs_unlock(i);
	}
	print_subs(L_INFO);
}





/**
 * Creates a subscription based on the given parameters.
 * @param req_uri - the AOR of the user to subcribe to
 * @param from - the From header
 * @param duration - expires time in seconds
 * @param asserted_identity - P-Asserted-Identity-Header to use
 * @returns the r_notification or NULL on error
 */
r_subscription* new_r_subscription(str req_uri,int duration)
{
	r_subscription *s=0;
	
	s = shm_malloc(sizeof(r_subscription));
	if (!s){
		LOG(L_ERR,"ERR:"M_NAME":new_r_subscription: Error allocating %d bytes\n",
			sizeof(r_subscription));
		goto error;
	}
	memset(s,0,sizeof(r_subscription));
	
	STR_SHM_DUP(s->req_uri,req_uri,"new_r_subscription");
	if (!s->req_uri.s) goto error;
		
	s->duration = duration;
	s->expires = 0;
	
	return s;
error:
	if (s->req_uri.s) shm_free(s->req_uri.s);
	if (s) shm_free(s);	
	return 0;
}

/**
 * Adds a subscription to the list of subscriptions at the end (FIFO).
 * @param s - the subscription to be added
 */
void add_r_subscription(r_subscription *s)
{
	if (!s) return;
	s->hash = get_subscription_hash(s->req_uri);
	subs_lock(s->hash);
		s->next = 0;
		s->prev = subscriptions[s->hash].tail;
		if (subscriptions[s->hash].tail) subscriptions[s->hash].tail->next = s;			
		subscriptions[s->hash].tail = s;
		if (!subscriptions[s->hash].head) subscriptions[s->hash].head = s;		
	subs_unlock(s->hash);
}

/**
 * Updates the expiration time of a subscription.
 * \todo Maybe we should use a hash here to index it as this is called for every notification
 * @param aor - aor to look for
 * @param expires - new expiration time
 * @returns 1 if found, 0 if not
 */
int update_r_subscription(r_subscription *s,int expires)
{
	LOG(L_DBG,"DBG:"M_NAME":update_r_subscription: refreshing subscription for <%.*s> [%d]\n",
		s->req_uri.len,s->req_uri.s,expires);
	s->attempts_left = -1;
	if (expires == 0) del_r_subscription_nolock(s);
	else s->expires = expires+time_now;;
	subs_unlock(s->hash);	
	return 1;
}

/**
 * Returns a subscription if it exists
 * \note - this returns with a lock on the subscriptions[s->hash] if found. Don't forget to unlock when done!!!
 * @param aor - AOR to look for
 * @returns 1 if found, 0 if not
 */
r_subscription* get_r_subscription(str aor)
{
	r_subscription *s;
	unsigned int hash = get_subscription_hash(aor);
	subs_lock(hash);
		s = subscriptions[hash].head;
		while(s){
			if (s->req_uri.len == aor.len &&
				strncasecmp(s->req_uri.s,aor.s,aor.len)==0)
			{
				return s;
			}
			s = s->next;
		}
	subs_unlock(hash);	
	return 0;
}

/**
 * Finds out if a subscription exists
 * @param aor - AOR to look for
 * @returns 1 if found, 0 if not
 */
int is_r_subscription(str aor)
{
	r_subscription *s;
	unsigned int hash = get_subscription_hash(aor);
	subs_lock(hash);
		s = subscriptions[hash].head;
		while(s){
			if (s->req_uri.len == aor.len &&
				strncasecmp(s->req_uri.s,aor.s,aor.len)==0)
			{
				subs_unlock(hash);	
				return 1;
			}
			s = s->next;
		}
	subs_unlock(hash);	
	return 0;
}

/**
 * Deletes a subscription from the list of subscriptions 
 * @param s - the subscription to be deleted
 */
void del_r_subscription(r_subscription *s)
{
	if (!s) return;
	subs_lock(s->hash);
		if (subscriptions[s->hash].head == s) subscriptions[s->hash].head = s->next;
		else s->prev->next = s->next;
		if (subscriptions[s->hash].tail == s) subscriptions[s->hash].tail = s->prev;
		else s->next->prev = s->prev;
	subs_unlock(s->hash);
	free_r_subscription(s);
}

/**
 * Deletes a subscription from the list of subscriptions.
 * \note Must have the lock to do this
 * @param s - the subscription to be deleted
 */
void del_r_subscription_nolock(r_subscription *s)
{
	if (!s) return;
	if (subscriptions[s->hash].head == s) subscriptions[s->hash].head = s->next;
	else s->prev->next = s->next;
	if (subscriptions[s->hash].tail == s) subscriptions[s->hash].tail = s->prev;
	else s->next->prev = s->prev;
	free_r_subscription(s);
}

/**
 * Frees up space taken by a subscription
 * @param s - the subscription to free
 */
void free_r_subscription(r_subscription *s)
{
	if (s){
		if (s->req_uri.s) shm_free(s->req_uri.s);
		if (s->dialog) tmb.free_dlg(s->dialog);
		shm_free(s);
	}
}

void print_subs(int log_level)
{
	r_subscription *s;
	int i;
	LOG(log_level,ANSI_GREEN"INF:"M_NAME":----------  Subscription list begin ---------\n");
	for(i=0;i<subscriptions_hash_size;i++){
		subs_lock(i);
		s = subscriptions[i].head;
		r_act_time();
		while(s){
			LOG(log_level,ANSI_GREEN"INF:"M_NAME":[%4u]\tP: <"ANSI_BLUE"%.*s"ANSI_GREEN"> D:["ANSI_CYAN"%5d"ANSI_GREEN"] E:["ANSI_MAGENTA"%5d"ANSI_GREEN"] Att:[%2d]\n",
				s->hash,s->req_uri.len,s->req_uri.s,s->duration,(int)(s->expires-time_now),s->attempts_left);
			s = s->next;			
		}
		subs_unlock(i);
	}
	LOG(log_level,ANSI_GREEN"INF:"M_NAME":----------  Subscription list end -----------\n");	
}








char *pcscf_reginfo_dtd;/**< DTD to check the reginfo/xml in the NOTIFY to reg */

static xmlDtdPtr	dtd=0;	/**< DTD file */
static xmlValidCtxt	cvp;	/**< XML Validating context */

/**
 * Initializes the libxml parser.
 * @param dtd_filename - path to the DTD file
 * @returns 1 if OK, 0 on error
 */
int parser_init(char *dtd_filename)
{
	dtd = xmlParseDTD(NULL,(unsigned char*)dtd_filename);
	if (!dtd){
		LOG(L_ERR,"ERR:"M_NAME":parser_init: unsuccesful DTD parsing from file <%s>\n",
			dtd_filename);
		return 0;
	}
	cvp.userData = (void*)stderr;
	cvp.error = (xmlValidityErrorFunc) fprintf;
	cvp.warning = (xmlValidityWarningFunc) fprintf;
	return 1;
}

/**
 * Destroys the parser. 
 */
void parser_destroy()
{
	xmlCleanupParser();
}



/**
 * Trims spaces and duplicate content into pkg.
 * @param dest - destination
 * @param src - source
 */
static inline void space_trim_dup(str *dest, char *src)
{
	int i;
	dest->s=0;
	dest->len=0;
	if (!src) return;
	dest->len = strlen(src);
	i = dest->len-1;
	while((src[i]==' '||src[i]=='\t') && i>0) 
		i--;
	i=0;
	while((src[i]==' '||src[i]=='\t') && i<dest->len)
		i++;
	dest->len -= i;
	dest->s = pkg_malloc(dest->len);
	if (!dest->s) {
		LOG(L_ERR,"ERR:"M_NAME":space_trim_dup: Out of memory allocating %d bytes\n",dest->len);
		dest->len=0;
		return;
	}
	memcpy(dest->s,src+i,dest->len);
}

/**
 * Parses a notification and creates the r_notification object
 * @param xml - the XML data
 * @returns the new r_notification* or NULL on error
 */
r_notification* r_notification_parse(str xml)
{
	r_notification *n;
	r_registration *r;
	r_regcontact *rc;
	xmlDocPtr doc=0;
	xmlNodePtr root=0,child=0,nephew=0,node=0;
	xmlChar *reginfo_state=0,*x;
	char c=0;
	
	n = pkg_malloc(sizeof(r_notification));
	if (!n) {
		LOG(L_ERR,"ERR:"M_NAME":r_notification_parse: Error allocating %d bytes\n",
			sizeof(r_notification));
		goto error;
	}
	memset(n,0,sizeof(r_notification));

	if (!dtd) parser_init(pcscf_reginfo_dtd);
	doc=0;
	c = xml.s[xml.len];
	xml.s[xml.len]=0;
	doc = xmlParseDoc((unsigned char *)xml.s);
	if (!doc){
		LOG(L_ERR,"ERR:"M_NAME":r_notification_parse:  This is not a valid XML <%.*s>\n",
			xml.len,xml.s);
		goto error;
	}
	if (xmlValidateDtd(&cvp,doc,dtd)!=1){
		LOG(L_ERR,"ERR:"M_NAME":r_notification_parse:  Verification of XML against DTD failed <%.*s>\n",
			xml.len,xml.s);
		goto error;
	}
	root = xmlDocGetRootElement(doc);
	if (!root){
		LOG(L_ERR,"ERR:"M_NAME":r_notification_parse:  Empty XML <%.*s>\n",
			xml.len,xml.s);
		goto error;
	}

	reginfo_state = xmlGetProp(root,(xmlChar*)"state");
	LOG(L_DBG,"DBG:"M_NAME":r_notification_parse: reginfo_state <%s>\n",
			reginfo_state);
	if (reginfo_state[0]=='f'||reginfo_state[0]=='F')
		n->state = IMS_REGINFO_FULL;
	else 
		n->state = IMS_REGINFO_PARTIAL;
	
	for(child = root->children; child; child = child->next)
		if (child->type == XML_ELEMENT_NODE)
	{
		r = pkg_malloc(sizeof(r_registration));
		if (!r){
			LOG(L_ERR,"ERR:"M_NAME":r_notification_parse: Error allocating %d bytes\n",
				sizeof(r_registration));
			goto error;
		}
		memset(r,0,sizeof(r_registration));
		
		x = xmlGetProp(child,(xmlChar*)"id");
		space_trim_dup(&(r->id),(char*)x);
		xmlFree(x);

		x = xmlGetProp(child,(xmlChar*)"aor");
		space_trim_dup(&(r->aor),(char*)x);
		xmlFree(x);
		
		x = xmlGetProp(child,(xmlChar*)"state");
		
		if (x[0]=='a'||x[0]=='A') 
			r->state = IMS_REGINFO_ACTIVE;
		else 
			r->state = IMS_REGINFO_TERMINATED;
		xmlFree(x);

		for(nephew = child->children; nephew; nephew = nephew->next)
				if (nephew->type == XML_ELEMENT_NODE)
		{
			rc = pkg_malloc(sizeof(r_regcontact));
			if (!rc){
				LOG(L_ERR,"ERR:"M_NAME":r_notification_parse: Error allocating %d bytes\n",
					sizeof(r_regcontact));
				goto error;
			}
			memset(rc,0,sizeof(r_regcontact));
			
			x = xmlGetProp(nephew,(xmlChar*)"id");
			space_trim_dup(&(rc->id),(char*)x);
			xmlFree(x);
				
			x = xmlGetProp(nephew,(xmlChar*)"state");
			if (x[0]=='a'||x[0]=='A') 
				rc->state = IMS_REGINFO_ACTIVE;
			else 
				rc->state = IMS_REGINFO_TERMINATED;
			xmlFree(x);
			
			x = xmlGetProp(nephew,(xmlChar*)"event");
			switch(x[0]){
				case 'r':case 'R':
					switch (x[2]){
						case 'g': case 'G':
							rc->event = IMS_REGINFO_CONTACT_REGISTERED;
							break;
						case 'f': case 'F':
							rc->event = IMS_REGINFO_CONTACT_REFRESHED;
							break;						
						case 'j': case 'J':
							rc->event = IMS_REGINFO_CONTACT_REJECTED;
							break;						
						default:
							rc->event = IMS_REGINFO_NONE;
					}
					break;
				case 'c':case 'C':
					rc->event = IMS_REGINFO_CONTACT_CREATED;	
					break;
				case 's':case 'S':
					rc->event = IMS_REGINFO_CONTACT_SHORTENED;	
					break;
				case 'e':case 'E':
					rc->event = IMS_REGINFO_CONTACT_EXPIRED;	
					break;
				case 'd':case 'D':
					rc->event = IMS_REGINFO_CONTACT_DEACTIVATED;	
					break;
				case 'p':case 'P':
					rc->event = IMS_REGINFO_CONTACT_PROBATION;	
					break;
				case 'u':case 'U':
					rc->event = IMS_REGINFO_CONTACT_UNREGISTERED;	
					break;
				default:
					rc->event = IMS_REGINFO_NONE;	
			}
			xmlFree(x);

			x = xmlGetProp(nephew,(xmlChar*)"expires");			
			if (x) {
				rc->expires = atoi((char*)x);
				xmlFree(x);
			}
			
			node = nephew->children;
			while(node && node->type!=XML_ELEMENT_NODE)
				node =node->next;
			if (node) {
				x = xmlNodeGetContent(node);
				space_trim_dup(&(rc->uri),(char*)x);
				xmlFree(x);
			}
			
			rc->next = r->contact;
			r->contact = rc;
		}
		
		r->next = n->registration;
		n->registration = r;
					
	}
			
	if (reginfo_state) xmlFree(reginfo_state);		
	
	if (doc) xmlFreeDoc(doc);
	xml.s[xml.len]=c;
	return n;
error:	
	if (reginfo_state) xmlFree(reginfo_state);		

	if (doc) xmlFreeDoc(doc);
	xml.s[xml.len]=c;
	if (n) r_notification_free(n);
	return 0;
}



/**
 * Processes a notification and updates the registrar info.
 * @param n - the notification
 * @param expires - the Subscription-Status expires parameter
 * @returns 1 on success, 0 on error
 */
int r_notification_process(r_notification *n,int expires)
{
	r_registration *r;
	r_regcontact *rc;	
	r_contact *c;
	struct sip_uri puri;
	enum Reg_States reg_state;
	int expires2;
	r_subscription *s=0;
	
	r_notification_print(n);	
	if (!n) return 0;
	
	r_act_time();
	r = n->registration;
	while(r){
		
		rc = r->contact;
		while(rc){
			if (parse_uri(rc->uri.s,rc->uri.len,&puri)<0){
				LOG(L_ERR,"ERR:"M_NAME":r_notification_process: Error parsing Contact URI <%.*s>\n",
					rc->uri.len,rc->uri.s);
				goto next;
			}
//			LOG(L_CRIT,"DBG:"M_NAME":r_notification_process: refreshing contacts <%.*s> [%d]\n",rc->uri.len,rc->uri.s,rc->expires);
			if (rc->state==IMS_REGINFO_TERMINATED){
				reg_state = DEREGISTERED;
				expires2 = time_now+30;
				c = update_r_contact(puri.host,puri.port_no,puri.proto,
					0,&reg_state,&expires2,0,0,0);
				if (c) {
					LOG(L_DBG,"DBG:"M_NAME":r_notification_process: expired contact <%.*s>\n",
						c->uri.len,c->uri.s);
					r_unlock(c->hash);					
				}
			}else{
				reg_state = REGISTERED;
				expires2 = rc->expires+time_now;
				c = update_r_contact(puri.host,puri.port_no,puri.proto,
					0,&reg_state,&expires2,0,0,0);
				if (c) {
					LOG(L_DBG,"DBG:"M_NAME":r_notification_process: refreshing contact <%.*s> [%d]\n",
						c->uri.len,c->uri.s,rc->expires);
					r_unlock(c->hash);					
				}

			}
next:				
			rc = rc->next;	
		}
		s = get_r_subscription(r->aor);
		if (s){
			update_r_subscription(s,expires);
			subs_unlock(s->hash);
		}
		r = r->next;
	}

	return 1;
}

/** 
 * Prints the content of a notification
 * @param n - the notification to print
 */
void r_notification_print(r_notification *n)
{
	r_registration *r;
	r_regcontact *c;
	
	if (!n) return;
	LOG(L_DBG,"DBG:"M_NAME":r_notification_print: State %d\n",n->state);
	r = n->registration;
	while(r){
		LOG(L_DBG,"DBG:"M_NAME":r_notification_print: \tR [%d] ID<%.*s> AOR<%.*s>\n",r->state,
			r->id.len,r->id.s,r->aor.len,r->aor.s);
		c = r->contact;
		while(c){
			LOG(L_DBG,"DBG:"M_NAME":r_notification_print: \t\tC [%d]>[%d] ID<%.*s> URI<%.*s>\n",c->state,
				c->event,c->id.len,c->id.s,c->uri.len,c->uri.s);
			c = c->next;
		}
		r = r->next;
	}	
}

/**
 * Frees up the space taken by a notification
 * @param n - the notification to free
 */
void r_notification_free(r_notification *n)
{
	r_registration *r,*nr;
	r_regcontact *c,*nc;
	r = n->registration;
	while(r){
		nr = r->next;
		if (r->id.s) pkg_free(r->id.s);
		if (r->aor.s) pkg_free(r->aor.s);
		c = r->contact;
		while(c){
			nc = c->next;
			if (c->id.s) pkg_free(c->id.s);			
			if (c->uri.s) pkg_free(c->uri.s);
			pkg_free(c);
			c = c->next;
		}
		pkg_free(r);
		r = nr;
	}
	if (n) pkg_free(n);
}






