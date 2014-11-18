/*
 * $Id: registrar_storage.c 430 2007-08-01 13:18:42Z vingarzan $
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
 * Serving-CSCF - Registrar Storage Operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include <time.h>

#include "mod.h"
#include "registrar_storage.h"
#include "dlg_state.h"


extern struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/

int r_hash_size;						/**< Size of S-CSCF registrar hash table		*/
r_hash_slot *registrar=0;				/**< The S-CSCF registrar 						*/

time_t time_now;						/**< Current time of the S-CSCF registrar 		*/
		
/**
 * Refresh the current registrar time.
 */
inline void r_act_time()
{
	time_now=time(0);
}

/**
 * Returns if a contact is valid (as in not expired).
 * Caller should do actualization of time_now
 * @param c - the contact to check
 * @returns 1 if valid, else 0
 */
inline int r_valid_contact(r_contact *c)
{
	return (c->expires>time_now);
}

/**
 * Returns if a subscriber is valid (as in not expired).
 * Caller should do actualization of time_now
 * @param s - the contact to check
 * @returns 1 if valid, else 0
 */
inline int r_valid_subscriber(r_subscriber *s)
{
	return (s->expires>time_now);
}

/**
 * Computes the hash for a string.
 * @param aor - the aor to compute the hash on
 * @param hash_size - value to % with
 * @returns the hash % hash_size
 */
inline unsigned int get_aor_hash(str aor,int hash_size)
{
#define h_inc h+=v^(v>>3)
   char* p;
   register unsigned v;
   register unsigned h;

   h=0;
   for (p=aor.s; p<=(aor.s+aor.len-4); p+=4){
       v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
       h_inc;
   }
   v=0;
   for (;p<(aor.s+aor.len); p++) {
       v<<=8;
       v+=*p;
   }
   h_inc;

   h=((h)+(h>>11))+((h>>13)+(h>>23));
   return (h)%hash_size;
#undef h_inc 
}

/**
 * Initialize the registrar
 * @param hash_size - number of hash slots to create
 * @returns 1 on success, 0 on error
 */
int r_storage_init(int hash_size)
{
	int i;
	
	r_hash_size = hash_size;
	registrar = shm_malloc(sizeof(r_hash_slot)*r_hash_size);
	memset(registrar,0,sizeof(r_hash_slot)*r_hash_size);
	
	for(i=0;i<r_hash_size;i++){
		registrar[i].lock = lock_alloc();
		if (!registrar[i].lock){
			LOG(L_ERR,"ERR:"M_NAME":r_storage_init(): Error creating lock\n");
			return 0;
		}
		registrar[i].lock = lock_init(registrar[i].lock);
	}
			
	if (!registrar) return 0;
	
	return 1;
}

/**
 * Destroy the registrar
 */
void r_storage_destroy()
{
	int i;
	r_public *p,*np;
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
			p = registrar[i].head;
			while(p){
				np = p->next;
				free_r_public(p);
				p = np;
			}
		r_unlock(i);
		lock_dealloc(registrar[i].lock);
	}
	shm_free(registrar);
}


/**
 * Locks the required slot of the registrar.
 * @param hash - the index of the slot
 */
inline void r_lock(unsigned int hash)
{
//	LOG(L_CRIT,"GET %d\n",hash);
	lock_get(registrar[(hash)].lock);
//	LOG(L_CRIT,"GOT %d\n",hash);	
}

/**
 * UnLocks the required slot of the registrar
 * @param hash - the index of the slot
 */
inline void r_unlock(unsigned int hash)
{
	lock_release(registrar[(hash)].lock);
//	LOG(L_CRIT,"RELEASED %d\n",hash);	
}










/**
 * Creates a registrar public id subscriber 
 * This does not insert it in the registrar
 * @param subscriber - the contact of the subscribption - the subscriber
 * @param event - what event to subscribe to
 * @param expires - time of expiration
 * @param dialog - dialog for the subscription
 * @returns the r_subscriber created, NULL on error
 */
r_subscriber* new_r_subscriber(str subscriber,int event,int expires,dlg_t *dialog)
{
	r_subscriber *s;
	
	s = shm_malloc(sizeof(r_subscriber));
	if (!s) {
		LOG(L_ERR,"ERR:"M_NAME":new_r_subscriber(): Unable to alloc %d bytes\n",
			sizeof(r_subscriber));
		goto error;
	}
	memset(s,0,sizeof(r_subscriber));
	
	STR_SHM_DUP(s->subscriber,subscriber,"new_r_subscriber");
	
	s->event = event;
	
	s->expires = expires;

	s->dialog = dialog;
					
	return s;
error:
out_of_memory:
	if (s){
		if (s->subscriber.s) shm_free(s->subscriber.s);
		shm_free(s);		
	}
	return 0;
}


/**
 * Searches for a r_subscriber subscriber and returns it.
 * @param p - the r_public record to look into
 * @param subscriber - the uri of the subscriber
 * @param event - what event to look for
 * @returns - the r_subscriber found, 0 if not found
 */
r_subscriber* get_r_subscriber(r_public *p, str subscriber,int event)
{
	r_subscriber *s=0;
	if (!p) return 0;
	s = p->shead;
	while(s){
		if (s->event == event &&
			s->subscriber.len == subscriber.len &&
			strncasecmp(s->subscriber.s,subscriber.s,subscriber.len)==0) return s;
		s = s->next;
	}
	return 0;
}

/**
 * Adds a new subscriber to the list for the user
 * \note Make sure that get_r_subscriber(p,subscriber) returns 0 in order to avoid ureachable duplicates
 * @param p - the r_public to add to
 * @param subscriber - the contact of the subscribption - the subscriber
 * @param event - what event to subscribe to
 * @param expires - time of expiration
 * @param dialog - dialog for the subscription
 * @returns the r_subscriber that has been added, NULL on error
 */
r_subscriber* add_r_subscriber(r_public  *p,str subscriber,int event,int expires,dlg_t *dialog)
{
	r_subscriber *s;

	if (!p) return 0;
	
	s = new_r_subscriber(subscriber,event,expires,dialog);
	if (!s) return 0;		
	s->next = 0;
	s->prev = p->stail;
	if (p->stail) p->stail->next = s;
	p->stail = s;
	if (!p->shead) p->shead=s;
	
	return s;
}

/**
 * Updates the r_subscriber with the new expires value
 * If not found, it will be inserted
 * @param p - the r_public to add to
 * @param subscriber - the subscriber to update
 * @param expires - new expires value, NULL if not necessary
 * @param ua - new user agent string, NULL if no update necessary
 * @param dialog - dialog for the subscription
 * @returns the newly added r_public, 0 on error
 */
r_subscriber* update_r_subscriber(r_public *p,str subscriber,int event,int* expires,dlg_t *dialog)
{
	r_subscriber *s;
	
	if (!p) return 0;
	s = get_r_subscriber(p,subscriber,event);
	if (!s){
		if (expires)
			return add_r_subscriber(p,subscriber,event,*expires,dialog);
		else return 0;
	}else{
		if (expires) s->expires = *expires;
		if (s->dialog && s->dialog!=dialog) tmb.free_dlg(s->dialog);
		s->dialog = dialog;
		return s;
	}
}

/**
 * Drops and deallocates a r_subscriber.
 * \note When calling be sure that get_r_subscriber(p,uri) returns c, to avoid a bogus removal.
 * @param p - the r_public record to look into
 * @param s - the r_subscriber to remove
 */
void del_r_subscriber(r_public *p,r_subscriber *s)
{
//	LOG(L_ERR,"DBG:"M_NAME":del_r_subscriber: LIST: %p --> %p\n",p->shead,p->stail);
//	LOG(L_ERR,"DBG:"M_NAME":del_r_subscriber: %p<- S:%p ->%p\n",s->prev,s,s->next); 
	if (p->shead == s) p->shead = s->next;
	else s->prev->next = s->next;
	if (p->stail == s) p->stail = s->prev;
	else s->next->prev = s->prev;
	if (s->dialog) tmb.free_dlg(s->dialog);
	
	free_r_subscriber(s);
}

/**
 * Frees memory taken by a r_subscriber structure
 * @param s - the r_contact to be deallocated
 */
void free_r_subscriber(r_subscriber *s)
{
	if (!s) return;
	if (s->subscriber.s) shm_free(s->subscriber.s);
	shm_free(s);
}


/**
 * Searches for a r_contact contact and returns it.
 * @param p - the r_public record to look into
 * @param uri - the uri of the contact
 * @returns - the r_contact found, 0 if not found
 */
r_contact* get_r_contact(r_public *p, str uri)
{
	r_contact *c=0;
	if (!p) return 0;
	c = p->head;
	while(c){
		if (c->uri.len == uri.len &&
			strncasecmp(c->uri.s,uri.s,uri.len)==0) return c;
		c = c->next;
	}
	return 0;
}

/**
 * Creates a registrar contact.
 * This does not insert it in the registrar
 * @param uri - the contact uri
 * @param expires - the time of expiration
 * @param ua - the useragent string
 * @param path - Path header received at registration
 * @returns the new r_contact or NULL on error
 */
r_contact* new_r_contact(str uri,int expires,str ua,str path)
{
	r_contact *c;
	
	c = shm_malloc(sizeof(r_contact));
	if (!c) {
		LOG(L_ERR,"ERR:"M_NAME":new_r_contact(): Unable to alloc %d bytes\n",
			sizeof(r_contact));
		goto error;
	}
	memset(c,0,sizeof(r_contact));
	
	c->uri.s = shm_malloc(uri.len);
	if (!c->uri.s){
		LOG(L_ERR,"ERR:"M_NAME":new_r_contact(): Unable to alloc %d bytes\n",
			uri.len);
		goto error;
	}	
	c->uri.len = uri.len;
	memcpy(c->uri.s,uri.s,uri.len);
	
	c->expires = expires;
	
	c->ua.s = shm_malloc(ua.len);
	if (!c->ua.s){
		LOG(L_ERR,"ERR:"M_NAME":new_r_contact(): Unable to alloc %d bytes\n",
			ua.len);
		goto error;
	}
	c->ua.len = ua.len;
	memcpy(c->ua.s,ua.s,ua.len);	

	if (path.len){
		c->path.s = shm_malloc(path.len);
		if (!c->path.s){
			LOG(L_ERR,"ERR:"M_NAME":new_r_contact(): Unable to alloc %d bytes\n",
				path.len);
			goto error;
		}
		c->path.len = path.len;
		memcpy(c->path.s,path.s,path.len);	
	}
		
	return c;
error:
	if (c){
		if (c->uri.s) shm_free(c->uri.s);
		if (c->ua.s) shm_free(c->ua.s);
		shm_free(c);
			
	}
	return 0;
}

/**
 * Creates and Adds a new r_contact 
 * \note When calling be sure that get_r_contact(p,uri) returns 0, to avoid unreachable duplicates.
 * @param p - the r_public to add to
 * @param uri - the uri of the contact
 * @param expires - the expiration time
 * @param ua - the user agent string
 * @param path - Path header received at registration
 * @returns the newly added r_contact or NULL on error
 */
r_contact* add_r_contact(r_public *p,str uri,int expires,str ua,str path)
{
	r_contact *c;
	if (!p) return 0;
	c = new_r_contact(uri,expires,ua,path);
	if (!c) return 0;
	c->next=0;
	c->prev=p->tail;
	if (p->tail) {
		p->tail->next = c;
		p->tail = c;
	}
	else p->tail = c;
	if (!p->head) p->head=c;
	
	return c;
}

/**
 * Updates the r_contact with the new expires, ua valu, path value.
 * \note If not found it is added
 * \note Must be called with a lock on the hash slot to avoid races
 * @param p - the r_public to add to
 * @param uri - the contact uri
 * @param expires - new expires value, NULL if not necessary
 * @param ua - new user agent string, NULL if no update necessary
 * @param path - Path header received at registration
 * @returns the updated r_contact or NULL on error
 */
r_contact* update_r_contact(r_public *p,str uri,int *expires, str *ua,str *path)
{
	r_contact *c;
	
	if (!p) return 0;
	c = get_r_contact(p,uri);
	if (!c){
		if (expires && ua && path)
			return add_r_contact(p,uri,*expires,*ua,*path);
		else return 0;
	}else{
		if (expires) c->expires = *expires;
		if (ua){
			if (c->ua.s) shm_free(c->ua.s);
			c->ua.s = shm_malloc(ua->len);
			if (!c->ua.s) {
				LOG(L_ERR,"ERR:"M_NAME":update_r_contact(): Error allocating %d bytes\n",
					ua->len);
				c->ua.len=0;
				return 0;
			}
			c->ua.len = ua->len;
			memcpy(c->ua.s,ua->s,ua->len);
		}
		if (path){
			if (c->path.s) shm_free(c->path.s);
			c->path.s = shm_malloc(path->len);
			if (!c->path.s) {
				LOG(L_ERR,"ERR:"M_NAME":update_r_contact(): Error allocating %d bytes\n",
					path->len);
				c->path.len=0;
				return 0;
			}
			c->path.len = path->len;
			memcpy(c->path.s,path->s,path->len);
		}
		return c;
	}
}

/**
 * Drops and deallocates a r_contact.
 * \note When calling be sure that get_r_contact(p,uri) returns c, to avoid a bogus removal
 * @param p - the r_public record to look into
 * @param c - the r_contact to remove
 */
void del_r_contact(r_public *p,r_contact *c)
{
	if (p->head == c) p->head = c->next;
	else c->prev->next = c->next;
	if (p->tail == c) p->tail = c->prev;
	else c->next->prev = c->prev;
	free_r_contact(c);
}
/**
 * Frees memory taken by a r_contact structure
 * @param c - the r_contact to be deallocated
 */
void free_r_contact(r_contact *c)
{
	if (!c) return;
	if (c->uri.s) shm_free(c->uri.s);
	if (c->ua.s) shm_free(c->ua.s);
	if (c->path.s) shm_free(c->path.s);
	shm_free(c);
}

/**
 * Creates a registrar r_public.
 * This does not insert it in the registrar
 * @param aor - the public identity as address of record
 * @param reg_state - desired registration state
 * @param s - the ims subscription to which this belongs
 * @returns - the r_public created, NULL on error
 */
r_public* new_r_public(str aor, enum Reg_States reg_state, ims_subscription *s)
{
	r_public *p;
	
	p = shm_malloc(sizeof(r_public));
	if (!p){
		LOG(L_ERR,"ERR:"M_NAME":new_r_public(): Unable to alloc %d bytes\n",
			sizeof(r_public));
		goto error;
	}	
	memset(p,0,sizeof(r_public));
	
	p->hash = get_aor_hash(aor,r_hash_size);
	
	p->aor.s = shm_malloc(aor.len);
	if (!p->aor.s){
		LOG(L_ERR,"ERR:"M_NAME":new_r_public(): Unable to alloc %d bytes\n",
			aor.len);
		goto error;
	}
	p->aor.len = aor.len;
	memcpy(p->aor.s,aor.s,aor.len);
	
	p->reg_state = reg_state;
	
	p->s = s;
	if (s) {
		lock_get(s->lock);
		s->ref_count++;
		lock_release(s->lock);
	}
		
	return p;
error:
	if (p){
		if (p->aor.s) shm_free(p->aor.s);	
		shm_free(p);
	}
	return 0;	
}

/**
 * Searches for a r_public record and returns it.
 * \note Aquires the lock on the hash slot on success, so release it when you are done.
 * @param aor - the address of record to look for
 * @returns - the r_public found, 0 if not found
 */
r_public* get_r_public(str aor)
{
	r_public *p=0;
	unsigned int hash;
	hash = get_aor_hash(aor,r_hash_size);
	r_lock(hash);
	p = registrar[hash].head;
	while(p){
		if (p->aor.len == aor.len &&
			strncasecmp(p->aor.s,aor.s,aor.len)==0) return p;
		p = p->next;
	}
	r_unlock(hash);
	return 0;
}

/**
 * Searches for a r_public record and returns its expiration value.
 * @param aor - the address of record to look for
 * @returns - the expiration of the r_public if found or -999 if not found
 */
int get_r_public_expires(str aor)
{
	int max_expires=-999;
	r_contact *c;
	r_public *p=get_r_public(aor);
	if (!p) return -999;
	r_act_time();
	for(c=p->head;c;c=c->next)
		if (c->expires-time_now>max_expires){
			max_expires = c->expires - time_now;
		}
	r_unlock(p->hash);
	return max_expires;
}

/**
 * Searches for a r_public record and returns it.
 * \note Does NOT Aquire the lock on the hash_slot
 * \note Must be called with a lock on the hash
 * @param aor - the address of record to look for
 * @returns the r_public if found found or NULL if not found
 */
r_public* get_r_public_nolock(str aor)
{
	r_public *p=0;
	unsigned int hash;
	hash = get_aor_hash(aor,r_hash_size);
	p = registrar[hash].head;
	while(p){
		if (p->aor.len == aor.len &&
			strncasecmp(p->aor.s,aor.s,aor.len)==0) return p;
		p = p->next;
	}
	return 0;
}

/**
 * Searches for a r_public record and returns, with a previous lock aquired.
 * \note Aquires the lock on the hash slot on success AND if the previous lock lock was not previously aquired,
 *  so release it when you are done, but be careful to only release it if the new lock!=previous_lock.
 * @param aor - the address of record to look for
 * @param locked_hash - previous lock, already aquired.
 * @returns - the r_public found, 0 if not found
 */
r_public* get_r_public_previous_lock(str aor,int locked_hash)
{
	r_public *p=0;
	unsigned int hash;
	hash = get_aor_hash(aor,r_hash_size);
	if (hash!=locked_hash) r_lock(hash);
	p = registrar[hash].head;
	while(p){
		if (p->aor.len == aor.len &&
			strncasecmp(p->aor.s,aor.s,aor.len)==0) return p;
		p = p->next;
	}
	if (hash!=locked_hash) r_unlock(hash);
	return 0;
}


/**
 * Creates and Adds a new r_public record. 
 * \note Aquires the lock on the hash_slot on success, so release it when you are done.
 * \note When calling be sure that get_r_public(aor) returns 0, to avoid unreachable duplicates
 * @param aor - the address of record
 * @param reg_state - current registration state
 * @param s - the subscription attached
 * @returns the newly added r_public or NULL on error
 */
r_public* add_r_public(str aor,enum Reg_States reg_state,ims_subscription *s)
{
	r_public *p;
	unsigned int hash;

	p = new_r_public(aor,reg_state,s);
	hash = p->hash;
	if (!p) return 0;	
	p->next=0;
	r_lock(hash);
		p->prev=registrar[hash].tail;
		if (p->prev) p->prev->next = p;
		registrar[hash].tail = p;
		if (!registrar[hash].head) registrar[hash].head=p;
	return p;
}

/**
 * Creates and Adds a new r_public record with a previous lock already aquired. 
 * \note Aquires the lock on the hash_slot on success and if the hash slot was not
 * previously locked, so release it when you are done if different than locked_hash.
 * \note When calling be sure that get_r_public(aor) returns 0, to avoid unreachable duplicates
 * @param aor - the address of record
 * @param locked_hash - the previously locked hash
 * @param reg_state - current registration state
 * @param s - the subscription attached
 * @returns the newly added r_public or NULL on error
 */
r_public* add_r_public_previous_lock(str aor,int locked_hash,enum Reg_States reg_state,ims_subscription *s)
{
	r_public *p;
	unsigned int hash;

	p = new_r_public(aor,reg_state,s);
	hash = p->hash;
	if (!p) return 0;	
	p->next=0;
	if (hash!=locked_hash) 
		r_lock(hash);
			p->prev=registrar[hash].tail;
			if (p->prev) p->prev->next = p;
			registrar[hash].tail = p;
			if (!registrar[hash].head) registrar[hash].head=p;
	return p;
}

/**
 * Updates the r_public with the new reg_state and ims_subscription values.
 * If not found, it will be inserted.
 * \note Aquires the lock on the hash_slot on success, so release it when you are done.
 * @param aor - the address of record
 * @param reg_state - new registration state, NULL if no update necessary
 * @param s - the new subscription attached, NULL if no update necessary
 * @returns the update r_public or NULL on error
 */
r_public* update_r_public(str aor,enum Reg_States *reg_state,ims_subscription **s,
	str *ccf1, str *ccf2, str *ecf1, str *ecf2)
{
	r_public *p=0;

	p = get_r_public(aor);
	if (!p){
		if (reg_state && *reg_state && *reg_state!=NOT_REGISTERED && s){
			p = add_r_public(aor,*reg_state,*s);
			if (!p) return p;			
			if (ccf1) {
				if (p->ccf1.s) shm_free(p->ccf1.s);
				STR_SHM_DUP(p->ccf1,*ccf1,"SHM CCF1");
			}
			if (ccf2) {
				if (p->ccf2.s) shm_free(p->ccf2.s);
				STR_SHM_DUP(p->ccf2,*ccf2,"SHM CCF2");
			}
			if (ecf1) {
				if (p->ecf1.s) shm_free(p->ecf1.s);
				STR_SHM_DUP(p->ecf1,*ecf1,"SHM ECF1");
			}
			if (ecf2) {
				if (p->ecf2.s) shm_free(p->ecf2.s);
				STR_SHM_DUP(p->ecf2,*ecf2,"SHM ECF2");
			}
			return p;
		}
		else return 0;
	}else{
		if (reg_state) p->reg_state = *reg_state;
		if (*s) {
			if (p->s){
				lock_get(p->s->lock);
				if (p->s->ref_count==1){
					free_user_data(p->s);
				}else{
					p->s->ref_count--;
					lock_release(p->s->lock);
				}
			}			
			p->s = *s;
			lock_get(p->s->lock);
				p->s->ref_count++;
			lock_release(p->s->lock);
		}
		if (ccf1) {
			if (p->ccf1.s) shm_free(p->ccf1.s);
			STR_SHM_DUP(p->ccf1,*ccf1,"SHM CCF1");
		}
		if (ccf2) {
			if (p->ccf2.s) shm_free(p->ccf2.s);
			STR_SHM_DUP(p->ccf2,*ccf2,"SHM CCF2");
		}
		if (ecf1) {
			if (p->ecf1.s) shm_free(p->ecf1.s);
			STR_SHM_DUP(p->ecf1,*ecf1,"SHM ECF1");
		}
		if (ecf2) {
			if (p->ecf2.s) shm_free(p->ecf2.s);
			STR_SHM_DUP(p->ecf2,*ecf2,"SHM ECF2");
		}
		return p;
	}
out_of_memory:
	return p;	
}

/**
 * Updates the r_public with the new reg_state and ims_subscription values, with a previous lock on the registrar.
 * If not found, it will be inserted.
 * \note Aquires the lock on the hash_slot on success and if the lock is different from previous_lock, so release it
 *  when you are done and if different then from previous lock.
 * @param aor - the address of record
 * @param locked_hash - the previous lock on the registrar
 * @param reg_state - new registration state, NULL if no update necessary
 * @param s - the new subscription attached, NULL if no update necessary
 * @returns the update r_public or NULL on error
 */
r_public* update_r_public_previous_lock(str aor,int locked_hash,enum Reg_States *reg_state,ims_subscription **s,
	str *ccf1, str *ccf2, str *ecf1, str *ecf2)
{
	r_public *p=0;
	p = get_r_public_previous_lock(aor,locked_hash);
	if (!p){
		if (reg_state && *reg_state && *reg_state!=NOT_REGISTERED && s){
			p = add_r_public_previous_lock(aor,locked_hash,*reg_state,*s);
			if (!p) return p;			
			if (ccf1) {
				if (p->ccf1.s) shm_free(p->ccf1.s);
				STR_SHM_DUP(p->ccf1,*ccf1,"SHM CCF1");
			}
			if (ccf2) {
				if (p->ccf2.s) shm_free(p->ccf2.s);
				STR_SHM_DUP(p->ccf2,*ccf2,"SHM CCF2");
			}
			if (ecf1) {
				if (p->ecf1.s) shm_free(p->ecf1.s);
				STR_SHM_DUP(p->ecf1,*ecf1,"SHM ECF1");
			}
			if (ecf2) {
				if (p->ecf2.s) shm_free(p->ecf2.s);
				STR_SHM_DUP(p->ecf2,*ecf2,"SHM ECF2");
			}
			return p;
		}
		else return 0;
	}else{
		if (reg_state) p->reg_state = *reg_state;
		if (*s) {
			if (p->s){
				lock_get(p->s->lock);
				if (p->s->ref_count==1){
					free_user_data(p->s);
				}else{
					p->s->ref_count--;
					lock_release(p->s->lock);
				}
			}			
			p->s = *s;
			lock_get(p->s->lock);
				p->s->ref_count++;
			lock_release(p->s->lock);
		}
		if (ccf1) {
			if (p->ccf1.s) shm_free(p->ccf1.s);
			STR_SHM_DUP(p->ccf1,*ccf1,"SHM CCF1");
		}
		if (ccf2) {
			if (p->ccf2.s) shm_free(p->ccf2.s);
			STR_SHM_DUP(p->ccf2,*ccf2,"SHM CCF2");
		}
		if (ecf1) {
			if (p->ecf1.s) shm_free(p->ecf1.s);
			STR_SHM_DUP(p->ecf1,*ecf1,"SHM ECF1");
		}
		if (ecf2) {
			if (p->ecf2.s) shm_free(p->ecf2.s);
			STR_SHM_DUP(p->ecf2,*ecf2,"SHM ECF2");
		}
		return p;
	}
out_of_memory:
	return p;	
}



/**
 * Expires all the contacts for the given public id
 * @param public_id - public identity to expire contacts for
 */
void r_public_expire(str public_id)
{	
	int expire;
	r_public *r_pub;
	r_contact * r_cont ;
	
	r_act_time();
	expire = time_now;
	
	r_pub	=	get_r_public(public_id);
	if (!r_pub){
		LOG(L_ERR,"ERR:"M_NAME":set_expires: identity not found in registrar <%.*s>\n",public_id.len,public_id.s);
		return;
	}
	r_cont = r_pub->head;
	
	while(r_cont!=NULL){
		r_cont->expires = expire;
		r_cont=r_cont->next;
	}
	
	r_unlock(r_pub->hash);
	print_r(L_ALERT);
}

/**
 * Expires all the contacts of public identities that are related to the given private id
 * @param private_id - private identity to expire contacts for
 */
void r_private_expire(str private_id)
{	
	int expire,i;
	r_public *p;
	r_contact *c;

	r_act_time();
	expire = time_now;
	
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
			for(p = registrar[i].head;p;p=p->next){
				if (p->s){
					lock_get(p->s->lock);
						if (p->s->private_identity.len == private_id.len &&
							strncasecmp(p->s->private_identity.s,private_id.s,private_id.len)==0){
								for(c=p->head;c;c=c->next)
									c->expires = expire;								
							}
					lock_release(p->s->lock);
				}
			}
		r_unlock(i);
	}
	print_r(L_ALERT);
}

/**
 * Drops and deallocates a r_public.
 * \note Don't forget to release the lock on the !!OLD!! hash value (yes, the memory is 
 * deallocated, so you can not do r_unlock(p->hash) because p does not exist anymore!)
 * \note When calling be sure that get_r_public(aor) returns p, to avoid a bogus removal
 * @param p - the r_public to remove
 */
void del_r_public(r_public *p)
{
	S_drop_all_dialogs(p->aor);
	if (registrar[p->hash].head == p) registrar[p->hash].head = p->next;
	else p->prev->next = p->next;
	if (registrar[p->hash].tail == p) registrar[p->hash].tail = p->prev;
	else p->next->prev = p->prev;
	free_r_public(p);
}

/**
 * Frees memory taken by a r_public aor structure
 * @param p - the r_public to be deallocated
 */
void free_r_public(r_public *p)
{
	r_contact *c,*n;
	r_subscriber *s,*m;
	if (!p) return;
	if (p->aor.s) shm_free(p->aor.s);
	if (p->early_ims_ip.s) shm_free(p->early_ims_ip.s);
	if (p->s) {
		lock_get(p->s->lock);
		p->s->ref_count--;
		if (p->s->ref_count<=0) {
			free_user_data(p->s);
		}else lock_release(p->s->lock);
	}
	if (p->ccf1.s) shm_free(p->ccf1.s);
	if (p->ccf2.s) shm_free(p->ccf2.s);
	if (p->ecf1.s) shm_free(p->ecf1.s);
	if (p->ecf2.s) shm_free(p->ecf2.s);
	
	c = p->head;
	while(c){
		n = c->next;
		free_r_contact(c);
		c = n;
	}
	s = p->shead;
	while(s){
		m = s->next;
		free_r_subscriber(s);
		s = m;
	}
	shm_free(p);
}





/**
 * Debug print the contents of the entire registrar.
 * @param log_level - logging level
 */
void print_r(int log_level)
{
	r_public *p;
	r_contact *c;
	r_subscriber *s;
	int i;
	if (debug<log_level) return; /* to avoid useless calls when nothing will be printed */
	r_act_time();
	LOG(log_level,ANSI_GREEN"INF:"M_NAME":----------  Registrar Contents begin --------\n");
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
			p = registrar[i].head;
			while(p){
				LOG(log_level,ANSI_GREEN"INF:"M_NAME":[%4d] P: <"ANSI_BLUE"%.*s"ANSI_GREEN"> R["ANSI_MAGENTA"%2d"ANSI_GREEN"] Early-IMS: <"ANSI_YELLOW"%.*s"ANSI_GREEN"> \n",i,
					p->aor.len,p->aor.s,p->reg_state,p->early_ims_ip.len,p->early_ims_ip.s);
				
				if (p->ccf1.len) LOG(log_level,ANSI_GREEN"INF:"M_NAME":         CCF1: <"ANSI_MAGENTA"%.*s"ANSI_GREEN"> CCF2: <"ANSI_MAGENTA"%.*s"ANSI_GREEN"> \n",
					p->ccf1.len,p->ccf1.s,p->ccf2.len,p->ccf2.s);
				if (p->ecf1.len) LOG(log_level,ANSI_GREEN"INF:"M_NAME":         ECF1: <"ANSI_MAGENTA"%.*s"ANSI_GREEN"> ECF2: <"ANSI_MAGENTA"%.*s"ANSI_GREEN"> \n",
					p->ecf1.len,p->ecf1.s,p->ecf2.len,p->ecf2.s);
					
				c = p->head;
				while(c){
					LOG(log_level,ANSI_GREEN"INF:"M_NAME":         C: <"ANSI_RED"%.*s"ANSI_GREEN"> Exp:["ANSI_MAGENTA"%4ld"ANSI_GREEN"]\n",
						c->uri.len,c->uri.s,c->expires-time_now);					
					LOG(log_level,ANSI_GREEN"INF:"M_NAME":           Path:"ANSI_YELLOW"%.*s"ANSI_GREEN"\n",c->path.len,c->path.s);
					LOG(log_level,ANSI_GREEN"INF:"M_NAME":           UA: <%.*s>\n",
						c->ua.len,c->ua.s);
					c = c->next;
				}
				s = p->shead;
				while(s){
					LOG(log_level,ANSI_GREEN"INF:"M_NAME":         S: Event["ANSI_BLUE"%d"ANSI_GREEN
						"] Exp:["ANSI_MAGENTA"%4ld"ANSI_GREEN"] <"ANSI_CYAN"%.*s"ANSI_GREEN"> \n",
						s->event,s->expires-time_now,s->subscriber.len,s->subscriber.s);					
					s = s->next;
				}
				p = p->next;
			} 		
		r_unlock(i);
	}
	LOG(log_level,"INF:"M_NAME":----------  Registrar Contents end ----------\n");
}

