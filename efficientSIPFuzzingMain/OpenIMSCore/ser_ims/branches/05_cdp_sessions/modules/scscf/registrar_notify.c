/*
 * $Id: registrar_notify.c 202 2007-03-26 15:21:29Z vingarzan $
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
 * Serving-CSCF - "reg" Event Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include "registrar_notify.h"

#include "registrar_storage.h"
#include "mod.h" 
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../parser/parse_uri.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "sip.h"

extern struct tm_binds tmb;            		/**< Structure with pointers to tm funcs 		*/

extern str scscf_name_str;							/**< fixed name of the S-CSCF 					*/

extern int subscription_default_expires;	/**< the default value for expires if none found */
extern int subscription_min_expires;		/**< minimum subscription expiration time 		*/
extern int subscription_max_expires;		/**< maximum subscription expiration time 		*/

extern time_t time_now;						/**< Current time of the S-CSCF registrar		*/

extern int r_hash_size;						/**< Size of S-CSCF registrar hash table		*/
extern r_hash_slot *registrar;				/**< The S-CSCF registrar 						*/

r_notification_list *notification_list=0;	/**< List of pending notifications				*/
	
/**
 * Initializes the reg notifications list.
 */
int r_notify_init()
{
	notification_list = shm_malloc(sizeof(r_notification_list));
	if (!notification_list) return 0;
	memset(notification_list,0,sizeof(r_notification_list));
	notification_list->lock = lock_alloc();
	if (!notification_list->lock) return 0;
	notification_list->lock = lock_init(notification_list->lock);
	return 1;
}

/**
 * Destroys the reg notifications list.
 */
void r_notify_destroy()
{
	r_notification *n,*nn;
	lock_get(notification_list->lock);
	n = notification_list->head;
	while(n){
		nn = n->next;
		free_r_notification(n);
		n = nn;
	}
	lock_destroy(notification_list->lock);
	lock_dealloc(notification_list->lock);	
	shm_free(notification_list);
}


static str lookup_sip={"sip:",4};
/**
 * Check if this message quallifies for a subscription.
 * Only 3 entities can subscribe:
 * - the user agent to its own state
 * - the P-CSCF specified in a Path header for that user
 * - AS (not implemented yet
 * \todo Allow also the AS listed in iFC and not belonging to a 3rd party 
 * @param msg - the SIP SUBSCRIBE message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if allowed, #CSCF_RETURN_FALSE if not, #CSCF_RETURN_ERROR on error
 */
int S_can_subscribe(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	struct sip_uri puri;
	str uri={0,0};
	str event;
	str asserted_id;
	r_public *p=0;
	r_contact *c=0;
	ims_public_identity *pi=0;
	int i,j;

	LOG(L_DBG,"DBG:"M_NAME":S_can_subscribe: Checking if allowed to SUBSCRIBE\n");

	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_can_subscribe: This message is not a request\n");
		goto error;
	}		
	if (msg->first_line.u.request.method.len != 9 ||
		memcmp(msg->first_line.u.request.method.s,"SUBSCRIBE",9)!=0)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_can_subscribe: This message is not a SUBSCRIBE\n");
		goto error;
	}

	/* 1. Get the event 	*/
	event = cscf_get_event(msg);
	if (event.len!=3||strncasecmp(event.s,"reg",3)!=0){
		LOG(L_ERR,"ERR:"M_NAME":S_can_subscribe: Accepting only <Event: reg>. Found: <%.*s>\n",
			event.len,event.s);
		ret = CSCF_RETURN_FALSE;	
		goto done;
	} 
	
	/* 2. Get the target of the SUBSCRIBE from RequestURI */
	if (msg->new_uri.s) uri = msg->new_uri;
	else uri = msg->first_line.u.request.uri;
	
	if (parse_uri(uri.s, uri.len, &puri) < 0) {
		LOG(L_ERR,"ERR:"M_NAME":S_can_subscribe: Error parsing uri <%.*s>\n",
			uri.len,uri.s);
		goto error;
	}
	uri.len = lookup_sip.len+puri.user.len+1+puri.host.len;
	uri.s = pkg_malloc(uri.len);
	if (!uri.s){
		LOG(L_ERR,"ERR:"M_NAME":S_can_subscribe: Error allocating %d bytes\n",uri.len);
		goto error;
	}
	uri.len=0;
	memcpy(uri.s,lookup_sip.s,lookup_sip.len);
	uri.len+=lookup_sip.len;
	memcpy(uri.s+uri.len,puri.user.s,puri.user.len);
	uri.len+=puri.user.len;
	uri.s[uri.len++]='@';
	memcpy(uri.s+uri.len,puri.host.s,puri.host.len);
	uri.len+=puri.host.len;

	/* 3. Check if the asserted identity is in the same group with the ReqURI */
	asserted_id = cscf_get_asserted_identity(msg);
	if (!asserted_id.len){
		LOG(L_ERR,"ERR:"M_NAME":S_can_subscribe: P-Asserted-Identity empty.\n");
		ret =  CSCF_RETURN_FALSE;
		goto done;		
	}
	LOG(L_DBG,"DBG:"M_NAME":S_can_subscribe: P-Asserted-Identity <%.*s>.\n",
		asserted_id.len,asserted_id.s);
	
	p = get_r_public(uri);
	if (!p){
		LOG(L_ERR,"ERR:"M_NAME":S_can_subscribe: Identity not found <%.*s>\n",
			uri.len,uri.s);
		ret =  CSCF_RETURN_FALSE;
		goto done;
	}
	if (p->aor.len == asserted_id.len &&
		strncasecmp(p->aor.s,asserted_id.s,asserted_id.len)==0)
	{
		LOG(L_DBG,"DBG:"M_NAME":S_can_subscribe: Identity found as AOR <%.*s>\n",
			uri.len,uri.s);
		ret = CSCF_RETURN_TRUE;
		goto done;
	}
	if (p->s){
		for(i=0;i<p->s->service_profiles_cnt;i++)
			for(j=0;j<p->s->service_profiles[i].public_identities_cnt;j++)
			{
				pi = &(p->s->service_profiles[i].public_identities[j]);
				if (!pi->barring &&
					pi->public_identity.len == asserted_id.len &&
					strncasecmp(pi->public_identity.s,asserted_id.s,asserted_id.len)==0)
				{
					LOG(L_DBG,"DBG:"M_NAME":S_can_subscribe: Identity found in SP[%d][%d]\n",
						i,j);
					ret = CSCF_RETURN_TRUE;
					goto done;
				}	
			}
	}
	
	/* 4. Check if it present in any of the Path headers */
	c=p->head;

	while(c){
		if (c->path.len){
			for(i=0;i<c->path.len-asserted_id.len;i++)
				if (strncasecmp(c->path.s+i,asserted_id.s,asserted_id.len)==0){
					LOG(L_DBG,"DBG:"M_NAME":S_can_subscribe: Identity found in Path <%.*s>\n",
						c->path.len,c->path.s);
					ret = CSCF_RETURN_TRUE;
					goto done;
				}
		}
		c = c->next;
	}
	
	
done:
	if (p) r_unlock(p->hash);
	if (uri.s) pkg_free(uri.s);
	return ret;
error:
	if (p) r_unlock(p->hash);
	if (uri.s) pkg_free(uri.s);
	ret=CSCF_RETURN_ERROR;
	return ret;	
}


/**
 * Save this subscription.
 * @param msg - the SIP SUBSCRIBE message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if allowed, #CSCF_RETURN_FALSE if not, #CSCF_RETURN_ERROR on error
 */
int S_subscribe(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	struct sip_uri puri;
	str uri={0,0};
	str event;
	int event_i=IMS_EVENT_NONE;
	int expires=0,expires_time=0;
	str subscriber;
	r_public *p=0;
	r_subscriber *s=0,*new_s=0;
	dlg_t *dialog=0;

	LOG(L_DBG,"DBG:"M_NAME":S_subscribe: Saving SUBSCRIBE\n");

	/* First check the parameters */
	if (msg->first_line.type!=SIP_REQUEST)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_subscribe: This message is not a request\n");
		goto error;
	}		
	if (msg->first_line.u.request.method.len != 9 ||
		memcmp(msg->first_line.u.request.method.s,"SUBSCRIBE",9)!=0)
	{
		LOG(L_ERR,"ERR:"M_NAME":S_subscribe: This message is not a SUBSCRIBE\n");
		goto error;
	}

	/* 1. Get the event 	*/
	event = cscf_get_event(msg);
	if (event.len!=3||strncasecmp(event.s,"reg",3)!=0){
		LOG(L_ERR,"ERR:"M_NAME":S_subscribe: Accepting only <Event: reg>. Found: <%.*s>\n",
			event.len,event.s);
		ret = CSCF_RETURN_FALSE;	
		goto done;
	} 
	if (event.len==0 && strncasecmp(event.s,"reg",3)==0)
		event_i = IMS_EVENT_REG;
	
	/* 2. Get the target of the SUBSCRIBE from RequestURI */
	if (msg->new_uri.s) uri = msg->new_uri;
	else uri = msg->first_line.u.request.uri;
	
	if (parse_uri(uri.s, uri.len, &puri) < 0) {
		LOG(L_ERR,"ERR:"M_NAME":S_subscribe: Error parsing uri <%.*s>\n",
			uri.len,uri.s);
		goto error;
	}
	uri.len = lookup_sip.len+puri.user.len+1+puri.host.len;
	uri.s = pkg_malloc(uri.len);
	if (!uri.s){
		LOG(L_ERR,"ERR:"M_NAME":S_subscribe: Error allocating %d bytes\n",uri.len);
		goto error;
	}
	uri.len=0;
	memcpy(uri.s,lookup_sip.s,lookup_sip.len);
	uri.len+=lookup_sip.len;
	memcpy(uri.s+uri.len,puri.user.s,puri.user.len);
	uri.len+=puri.user.len;
	uri.s[uri.len++]='@';
	memcpy(uri.s+uri.len,puri.host.s,puri.host.len);
	uri.len+=puri.host.len;

	/* 3. Get the Subscriber's Identity */
	subscriber = cscf_get_contact(msg);
	if (!subscriber.len){
		LOG(L_ERR,"ERR:"M_NAME":S_subscribe: Contact empty.\n");
		ret =  CSCF_RETURN_FALSE;
		goto done;		
	}
	LOG(L_DBG,"DBG:"M_NAME":S_subscribe: Contact <%.*s>.\n",
		subscriber.len,subscriber.s);
	
	p = get_r_public(uri);
	/* Registration is possible even if the user is not registered... so just create one */
	if (!p){
		p = add_r_public(uri,0,0);
	}
	if (!p){		
		LOG(L_ERR,"ERR:"M_NAME":S_subscribe: Identity not found and error on creation <%.*s>\n",
			uri.len,uri.s);
		ret =  CSCF_RETURN_FALSE;
		goto done;
	}
		
	expires = cscf_get_expires_hdr(msg);
	if (expires == -1) expires = subscription_default_expires;
	if (expires > 0) {
		if (expires < subscription_min_expires) expires = subscription_min_expires;
		if (expires > subscription_max_expires) expires = subscription_max_expires;
		r_act_time();
		expires_time = expires + time_now;
		
		/* get the old subscriber - if any */
		s = get_r_subscriber(p,subscriber,event_i);	
		if (!s){
			/* create a new dialog */
			if (tmb.new_dlg_uas(msg, 200, &dialog) < 0) {		
				LOG(L_ERR,"ERR:"M_NAME":S_subscribe:  Error while creating dialog state\n");
				ret = CSCF_RETURN_FALSE;
				goto error;
			}
		}else
			dialog = s->dialog;
		
		/* update the subscriber */
		if ((new_s=update_r_subscriber(p,subscriber,event_i,&expires_time,dialog))!=0){		
			//if (!s)	/* Only on first subscription, not on refreshes, send a notify */
				S_event_reg(p,0,new_s,IMS_REGISTRAR_SUBSCRIBE,0);
			ret = CSCF_RETURN_TRUE;
		}
		else
			ret = CSCF_RETURN_FALSE;	
	}else{
		/* Unsubscribe */
		/* get the old subscriber - if any */
		s = get_r_subscriber(p,subscriber,event_i);	
		if (s) del_r_subscriber(p,s);
		ret = CSCF_RETURN_TRUE;
	}
done:	
	r_unlock(p->hash);
	if (uri.s) pkg_free(uri.s);
	if (expires ==0 )S_SUBSCRIBE_reply(msg,200,MSG_REG_UNSUBSCRIBE_OK,&expires,0);
	else S_SUBSCRIBE_reply(msg,200,MSG_REG_SUBSCRIBE_OK,&expires,0);
	return ret;
error:
	if (p) r_unlock(p->hash);
	if (uri.s) pkg_free(uri.s);
	ret=CSCF_RETURN_FALSE;
	return ret;	
}



str expires_hdr1={"Expires: ",9};
str expires_hdr2={"\r\n",2};
str contact_hdr1={"Contact: <",10};
str contact_hdr2={">\r\n",3};
/**
 * Replies to a SUBSCRIBE and also adds the need headers.
 * Path for example.
 * @param msg - the SIP SUBSCRIBE message
 * @param code - response code to send
 * @param text - response phrase to send
 * @param expires - expiration interval in seconds
 * @param contact - contact to add to reply
 * @returns the tmn.r_reply returned value value
 */
int S_SUBSCRIBE_reply(struct sip_msg *msg, int code,  char *text,int *expires,str *contact)
{
	str hdr={0,0};
	tmb.t_newtran(msg);
	
	if (expires){
		hdr.len = expires_hdr1.len+12+expires_hdr1.len;
		hdr.s = pkg_malloc(hdr.len);
		if (!hdr.s){
			LOG(L_ERR,"ERR:"M_NAME":S_SUBSCRIBE_reply: Error allocating %d bytes.\n",
				hdr.len);			
		}else{
			hdr.len=0;
			STR_APPEND(hdr,expires_hdr1);
			sprintf(hdr.s+hdr.len,"%d",*expires);
			hdr.len += strlen(hdr.s+hdr.len);
			STR_APPEND(hdr,expires_hdr2);
			cscf_add_header_rpl(msg,&hdr);		
			pkg_free(hdr.s);
		}
	}

	if (contact){
		hdr.len = contact_hdr1.len+contact->len+contact_hdr2.len;
		hdr.s = pkg_malloc(hdr.len);
		if (!hdr.s){
			LOG(L_ERR,"ERR:"M_NAME":S_SUBSCRIBE_reply: Error allocating %d bytes.\n",
				hdr.len);			
		}else{
			hdr.len=0;
			STR_APPEND(hdr,contact_hdr1);
			STR_APPEND(hdr,*contact);
			STR_APPEND(hdr,contact_hdr2);
			cscf_add_header_rpl(msg,&hdr);		
			pkg_free(hdr.s);
		}
	}
	
	return tmb.t_reply(msg,code,text);
}




static str ruri_lr={";lr",3};
static str subs_terminated={"terminated",10};
static str subs_active={"active;expires=",15};
/**
 * Creates notifications with the given content for all of the subscribers.
 * @param pv - r_public* to which it refers
 * @param cv - the r_contact* to which it refers or NULL if for all
 * @param ps - the r_subscriber*  to which it refers or NULL if for all
 * @param content - the body content
 * @param expires - the remaining subcription expiration time in seconds
 */
static void r_create_notifications(void *pv,void *cv,void *ps,str content,long expires)
{
	r_notification *n;
	r_public *p=(r_public*)pv;
//	r_contact *c;
	r_subscriber *for_s=(r_subscriber*)ps;
	r_subscriber *s;	
	str uri={0,0},req_uri={0,0},subscription_state={"active;expires=10000000000",26},
		event={0,0},content_type={"application/reginfo+xml",23};
		
	uri = p->aor;
	if (!for_s)	s = p->shead;
	else s = for_s;
	
	while(s){
		req_uri.len = s->subscriber.len+ruri_lr.len;
		req_uri.s = pkg_malloc(req_uri.len);
		if (!req_uri.s) {
			LOG(L_ERR,"ERR:"M_NAME":r_create_notifications: Error allocating %d bytes.\n",req_uri.len);
			return;			
		}
		memcpy(req_uri.s,s->subscriber.s,s->subscriber.len);
		memcpy(req_uri.s+s->subscriber.len,ruri_lr.s,ruri_lr.len);
		//route = s->path;
		if (s->expires>=time_now) {
			subscription_state.s = pkg_malloc(32);
			subscription_state.len=0;
			if (subscription_state.s){
				sprintf(subscription_state.s,"%.*s%ld",subs_active.len,subs_active.s,expires);
				subscription_state.len=strlen(subscription_state.s);
			}					
		}else {					
			STR_PKG_DUP(subscription_state,subs_terminated,"pkg subs state");
		}
		n = new_r_notification(req_uri,uri,subscription_state,event,
			content_type,content,s->dialog,s->version++);						
		if (req_uri.s) pkg_free(req_uri.s);
		if (subscription_state.s) pkg_free(subscription_state.s);	
		if (n) add_r_notification(n);		
		
		if (!for_s) s = s->next;				
		else s = 0;
	}
}

/** Maximum reginfo XML size */
#define MAX_REGINFO_SIZE 16384



/**
 * Look if there are no more public ids or contacts registered and expires the subscription
 * \note must be called with lock on the domain
 * @param p - the r_public to update.
 * @returns the expiration is seconds
 */
static inline long r_update_subscription_status(r_public *p)
{
	r_subscriber *s;
	r_contact *c;
	long expires=0;
	int cnt=0;
	
	r_act_time();
	
	c = p->head;
	while(c){
		if (r_valid_contact(c)){
			if (c->expires-time_now+30>expires)
				expires = c->expires-time_now+30;
			cnt++;
		}
		c = c->next;
	}		
	if (!cnt){
		s = p->shead;
		while(s){
			s->expires = time_now-1;
			s = s->next;
		}	
	}
	return expires;
}

static str xml_start={"<?xml version=\"1.0\"?>\n",22};

static str r_full={"full",4};
static str r_partial={"partial",7};
static str r_reginfo_s={"<reginfo xmlns=\"urn:ietf:params:xml:ns:reginfo\" version=\"%s\" state=\"%.*s\">\n",74};
static str r_reginfo_e={"</reginfo>\n",11};

//static str r_init={"init",4};
static str r_active={"active",6};
static str r_terminated={"terminated",10};
static str registration_s={"\t<registration aor=\"%.*s\" id=\"%p\" state=\"%.*s\">\n",48};
static str registration_e={"\t</registration>\n",17};

static str r_registered={"registered",10};
static str r_created={"created",7};
static str r_refreshed={"refreshed",9};
static str r_shortened={"shortened",9};
static str r_expired={"expired",7};
static str r_deactivated={"deactivated",11};
static str r_probation={"probation",9};
static str r_unregistered={"unregistered",12};
static str r_rejected={"rejected",8};
static str contact_s={"\t\t<contact id=\"%p\" state=\"%.*s\" event=\"%.*s\" expires=\"%d\">\n",59};
static str contact_e={"\t\t</contact>\n",13};

static str uri_s={"\t\t\t<uri>",8};
static str uri_e={"</uri>\n",7};
/**
 * Creates the full reginfo XML.
 * @param pv - the r_public to create for
 * @param event_type - event type
 * @param subsExpires - subscription expiration
 * @returns the str with the XML content
 */
str r_get_reginfo_full(void *pv,int event_type,long *subsExpires)
{		
	str x={0,0};
	str buf,pad;	
	char bufc[MAX_REGINFO_SIZE],padc[MAX_REGINFO_SIZE];

	r_public *p=(r_public*)pv,*p2;
	r_contact *c;
	ims_public_identity *pi;
	int i,j;
	unsigned int hash;
	
	buf.s = bufc;
	buf.len=0;
	pad.s = padc;
	pad.len=0;
	
	*subsExpires = r_update_subscription_status(p);
	
	STR_APPEND(buf,xml_start);
	sprintf(pad.s,r_reginfo_s.s,"%d",r_full.len,r_full.s);
	pad.len = strlen(pad.s);
	STR_APPEND(buf,pad);
	
	
	if (p->s){
		for(i=0;i<p->s->service_profiles_cnt;i++)
			for(j=0;j<p->s->service_profiles[i].public_identities_cnt;j++){
				pi = &(p->s->service_profiles[i].public_identities[j]);
				if (!pi->barring){
					hash = get_aor_hash(pi->public_identity,r_hash_size);
					if (hash == p->hash) /* because we already have the lock on p->hash */
						p2 = get_r_public_nolock(pi->public_identity);
					else 
						p2 = get_r_public(pi->public_identity);
					if (p2){
						if (p2->reg_state==REGISTERED)
							sprintf(pad.s,registration_s.s,p2->aor.len,p2->aor.s,p2,r_active.len,r_active.s);
						else 
							sprintf(pad.s,registration_s.s,p2->aor.len,p2->aor.s,p2,r_terminated.len,r_terminated.s);
						pad.len = strlen(pad.s);
						STR_APPEND(buf,pad);
						c = p2->head;
						while(c){
							sprintf(pad.s,contact_s.s,c,r_active.len,r_active.s,r_registered.len,r_registered.s,c->expires-time_now);
							pad.len = strlen(pad.s);
							STR_APPEND(buf,pad);							
							STR_APPEND(buf,uri_s);
							STR_APPEND(buf,c->uri);
							STR_APPEND(buf,uri_e);
							STR_APPEND(buf,contact_e);
							c = c->next;
						}
						STR_APPEND(buf,registration_e);
						if (p2->hash != p->hash) r_unlock(p2->hash);
					}
				}
			}				
	}

	STR_APPEND(buf,r_reginfo_e);

	
	x.s = pkg_malloc(buf.len+1);
	if (x.s){
		x.len = buf.len;
		memcpy(x.s,buf.s,buf.len);
		x.s[x.len]=0;
	}
	return x;
}


/**
 * Creates the partial reginfo XML.
 * @param pv - the r_public to create for
 * @param pc - the r_contatct to create for
 * @param event_type - event type
 * @param subsExpires - subscription expiration
 * @returns the str with the XML content
 */
str r_get_reginfo_partial( void *pv,void *pc,int event_type,long *subsExpires)
{		
	str x={0,0};
	str buf,pad;	
	char bufc[MAX_REGINFO_SIZE],padc[MAX_REGINFO_SIZE];
	int expires=-1;

	r_public *p=(r_public*)pv;
	r_contact *c=(r_contact*)pc;
	str state,event;
	
	buf.s = bufc;
	buf.len=0;
	pad.s = padc;
	pad.len=0;

	*subsExpires = r_update_subscription_status(p);
	
	STR_APPEND(buf,xml_start);
	sprintf(pad.s,r_reginfo_s.s,"%d",r_partial.len,r_partial.s);
	pad.len = strlen(pad.s);
	STR_APPEND(buf,pad);
	
	
	if (p){
		expires = c->expires-time_now;
		if (p->head == c && p->tail == c && 
			(event_type==IMS_REGISTRAR_CONTACT_EXPIRED ||
			 event_type==IMS_REGISTRAR_CONTACT_DEACTIVATED||
			 event_type==IMS_REGISTRAR_CONTACT_UNREGISTERED||
			 event_type==IMS_REGISTRAR_CONTACT_REJECTED)
		   )
			sprintf(pad.s,registration_s.s,p->aor.len,p->aor.s,p,r_terminated.len,r_terminated.s);
		else 
			sprintf(pad.s,registration_s.s,p->aor.len,p->aor.s,p,r_active.len,r_active.s);
		pad.len = strlen(pad.s);
		STR_APPEND(buf,pad);
		if (c){
			switch(event_type){
				case IMS_REGISTRAR_CONTACT_REGISTERED:
					state = r_active;
					event = r_registered;
					break;
				case IMS_REGISTRAR_CONTACT_CREATED:
					state = r_active;
					event = r_created;
					break;
				case IMS_REGISTRAR_CONTACT_REFRESHED:
					state = r_active;
					event = r_refreshed;
					break;
				case IMS_REGISTRAR_CONTACT_SHORTENED:
					state = r_active;
					event = r_shortened;
					break;
				case IMS_REGISTRAR_CONTACT_EXPIRED:
					state = r_terminated;
					event = r_expired;
					expires = 0;
					break;
				case IMS_REGISTRAR_CONTACT_DEACTIVATED:
					state = r_terminated;
					event = r_deactivated;
					break;
				case IMS_REGISTRAR_CONTACT_PROBATION:
					state = r_terminated;
					event = r_probation;
					break;
				case IMS_REGISTRAR_CONTACT_UNREGISTERED:
					state = r_terminated;
					event = r_unregistered;
					expires = 0;
					break;
				case IMS_REGISTRAR_CONTACT_REJECTED:			
					state = r_terminated;
					event = r_rejected;
					break;				
				default:
					state = r_active;
					event = r_registered;
			}
			sprintf(pad.s,contact_s.s,c,state.len,state.s,event.len,event.s,expires);
			pad.len = strlen(pad.s);
			STR_APPEND(buf,pad);							
			STR_APPEND(buf,uri_s);
			STR_APPEND(buf,c->uri);
			STR_APPEND(buf,uri_e);
			STR_APPEND(buf,contact_e);
			STR_APPEND(buf,registration_e);
		}
	}

	STR_APPEND(buf,r_reginfo_e);

	
	x.s = pkg_malloc(buf.len+1);
	if (x.s){
		x.len = buf.len;
		memcpy(x.s,buf.s,buf.len);
		x.s[x.len]=0;		
	}
	return x;
}

/**
 * Generate notifications and put them in the notification queue to be sent
 * \note Must be called with a lock on the r_public pv
 * @param pv - r_public to generate for
 * @param c - r_contact to generate for or NULL if for all
 * @param ps - r_subscriber to generated for or NULL if for all
 * @param event_type - event type
 * @param send_now - whether to send immediately or delay
 * @returns 1 on success, 0 on failure
 */
int S_event_reg(void *pv,void *c,void *ps,int event_type,int send_now)
{
	r_public *p=(r_public*)pv;
	r_subscriber *s=(r_subscriber*)ps;
	str content={0,0};
	long subsExpires;

	r_act_time();
	switch (event_type){
		case IMS_REGISTRAR_NONE:
			if (send_now) notification_timer(0,0);
			return 0;
			break;
		case IMS_REGISTRAR_SUBSCRIBE:
			content = r_get_reginfo_full(p,event_type,&subsExpires);
			r_create_notifications(p,0,s,content,subsExpires);			
			if (content.s) pkg_free(content.s);
			if (send_now) notification_timer(0,0);
			return 1;
			break;
			
		case IMS_REGISTRAR_CONTACT_REGISTERED:
		case IMS_REGISTRAR_CONTACT_CREATED:	
		case IMS_REGISTRAR_CONTACT_REFRESHED:
		case IMS_REGISTRAR_CONTACT_SHORTENED:
		case IMS_REGISTRAR_CONTACT_EXPIRED:
		case IMS_REGISTRAR_CONTACT_DEACTIVATED:
		case IMS_REGISTRAR_CONTACT_PROBATION:
		case IMS_REGISTRAR_CONTACT_UNREGISTERED:
		case IMS_REGISTRAR_CONTACT_REJECTED:
			content = r_get_reginfo_partial(p,c,event_type,&subsExpires);	
			r_create_notifications(p,c,s,content,subsExpires);
			if (content.s) pkg_free(content.s);
			if (send_now) notification_timer(0,0);
			return 1;
			break;
				
		default:
			LOG(L_ERR,"ERR:"M_NAME":S_event_reg: Unknown event %d\n",event_type);
			if (send_now) notification_timer(0,0);
			return 0;	
	}		
}


static str method={"NOTIFY",6};
static str event_hdr ={"Event: reg\r\n",12};
static str maxfwds_hdr ={"Max-Forwards: 70\r\n",18};
static str subss_hdr1={"Subscription-State: ",20};
static str subss_hdr2={"\r\n",2};
static str ctype_hdr1={"Content-Type: ",14};
static str ctype_hdr2={"\r\n",2};

/**
 * Callback for the UAC response to NOTIFY
 */
void uac_request_cb(struct cell *t,int type,struct tmcb_params *ps)
{
	LOG(L_DBG,"DBG:"M_NAME":uac_request_cb: Type %d\n",type);
}

/**
 * Creates a NOTIFY message and sends it
 * @param n - the r_notification to create the NOTIFY after
 */
void send_notification(r_notification *n)
{
	str h={0,0};

	LOG(L_DBG,"DBG:"M_NAME":send_notification: NOTIFY about <%.*s>\n",n->uri.len,n->uri.s);
	
	//tmb.print_dlg(stdout,n->dialog);
	
	h.len = 0;
	h.len += contact_hdr1.len + contact_hdr2.len + scscf_name_str.len;
	if (n->subscription_state.len) h.len += subss_hdr1.len + subss_hdr2.len + n->subscription_state.len;
	h.len+=event_hdr.len;
	h.len+=maxfwds_hdr.len;
	if (n->content_type.len) h.len += ctype_hdr1.len + ctype_hdr2.len + n->content_type.len;
	h.s = pkg_malloc(h.len);
	if (!h.s){
		LOG(L_ERR,"ERR:"M_NAME":send_notification: Error allocating %d bytes\n",h.len);
		h.len = 0;
	}

	h.len = 0;
	STR_APPEND(h,contact_hdr1);
	STR_APPEND(h,scscf_name_str);
	STR_APPEND(h,contact_hdr2);

	STR_APPEND(h,event_hdr);
	STR_APPEND(h,maxfwds_hdr);
	if (n->subscription_state.len) {
		STR_APPEND(h,subss_hdr1);
		STR_APPEND(h,n->subscription_state);
		STR_APPEND(h,subss_hdr2);
	}
	if (n->content_type.len) {
		STR_APPEND(h,ctype_hdr1);
		STR_APPEND(h,n->content_type);
		STR_APPEND(h,ctype_hdr2);
	}
	
	//LOG(L_CRIT,"DLG:%p\n",n->dialog);
	if (n->content.len)	
		tmb.t_request_within(&method, &h, &(n->content), n->dialog, uac_request_cb, 0);		
	else 
		tmb.t_request_within(&method, &h, 0, n->dialog, uac_request_cb, 0);		
	if (h.s) pkg_free(h.s);
}




/**
 * The Notification timer looks for unsent notifications and sends them.
 *  - because not all events should wait until the notifications for them are sent
 * @param ticks - the current time
 * @param param - pointer to the domain_list
 */
void notification_timer(unsigned int ticks, void* param)
{
	r_notification *n;
	lock_get(notification_list->lock);
	while(notification_list->head){
		n = notification_list->head;
		notification_list->head = n->next;
		if (n->next) n->next->prev=0;
		else notification_list->tail=n->next;
		lock_release(notification_list->lock);			
		
		send_notification(n);
		free_r_notification(n);
		lock_get(notification_list->lock);
	}	
	lock_release(notification_list->lock);				
}



/**
 * Creates a notification based on the given parameters
 * @param req_uri - the Request-URI for the NOTIFY
 * @param uri - uri to send to
 * @param subscription_state - the Subscription-State header value
 * @param event - the event
 * @param content_type - content type
 * @param content - content
 * @param dialog - dialog to send on
 * @returns the r_notification or NULL on error
 */
r_notification* new_r_notification(str req_uri,str uri,str subscription_state,str event,
					str content_type,str content,dlg_t *dialog,int version)
{
	r_notification *n=0;
	str buf;	
	char bufc[MAX_REGINFO_SIZE];
	
	n = shm_malloc(sizeof(r_notification));
	if (!n){
		LOG(L_ERR,"ERR:"M_NAME":new_r_notification: Error allocating %d bytes\n",
			sizeof(r_notification));
		goto error;
	}
	memset(n,0,sizeof(r_notification));
	
	STR_SHM_DUP(n->req_uri,req_uri,"new_r_notification");
	if (!n->req_uri.s) goto error;
	
	STR_SHM_DUP(n->uri,uri,"new_r_notification");
	if (!n->uri.s) goto error;
	
	STR_SHM_DUP(n->subscription_state,subscription_state,"new_r_notification");
	if (!n->subscription_state.s) goto error;
	
	STR_SHM_DUP(n->event,event,"new_r_notification");
	if (!n->event.s) goto error;

	STR_SHM_DUP(n->content_type,content_type,"new_r_notification");
	if (!n->content_type.s) goto error;

	sprintf(bufc,content.s,version);
	buf.s = bufc;
	buf.len = strlen(bufc);
	STR_SHM_DUP(n->content,buf,"new_r_notification");
	if (!n->content.s) goto error;
	
	n->dialog = dialog;
	
	return n;
error:
	free_r_notification(n);
	return 0;
}

/**
 * Adds a notification to the list of notifications at the end (FIFO).
 * @param n - the notification to be added
 */
void add_r_notification(r_notification *n)
{
	if (!n) return;
	lock_get(notification_list->lock);
	n->next = 0;
	n->prev = notification_list->tail;
	if (notification_list->tail) notification_list->tail->next = n;
	notification_list->tail = n;
	if (!notification_list->head) notification_list->head = n;		
	lock_release(notification_list->lock);
}

/**
 * Frees up space taken by a notification
 * @param n - the notification to be freed
 */
void free_r_notification(r_notification *n)
{
	if (n){
		if (n->req_uri.s) shm_free(n->req_uri.s);
		if (n->uri.s) shm_free(n->uri.s);
		if (n->subscription_state.s) shm_free(n->subscription_state.s);
		if (n->event.s) shm_free(n->event.s);
		if (n->content_type.s) shm_free(n->content_type.s);
		if (n->content.s) shm_free(n->content.s);
		shm_free(n);
	}
}










