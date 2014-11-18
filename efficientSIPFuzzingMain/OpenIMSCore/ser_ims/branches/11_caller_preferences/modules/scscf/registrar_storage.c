/*
 * $Id: registrar_storage.c 595 2008-10-22 17:15:11Z jsbach $
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

/*
 * 
 * Big change done, a new hash slot is added at the end of the table.
 * All the wildcarded PSIs will be added there in the r_public format 
 * the functions update_r_public, update_r_public_previous_lock are the ones
 * that will check that the subscription being saved is a wildcarded PSI and
 * will then insert it in the last slot
 * 
 * The function get_r_public will try to match the aor given with the wildcarded PSIs
 * if there is no record for that aor. 
 * 
 * 
 * \Author Alberto Diez  alberto dot diez -at- fokus dot fraunhofer dot de
 * 
 * 
 * r_storage_init changed to add 1 slot more
 * r_storage_destroy changed to delete 1 slot more
 * new_r_public changed, if its a wpsi then i give the fixed hash number (r_hash_size)
 * 				and i compile the regexp and save it in the r_public->regexp
 * 
 * 
*/











#include <time.h>

#include "mod.h"
#include "registrar_storage.h"
#include "dlg_state.h"


extern struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/

extern int scscf_support_wildcardPSI;

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
	registrar = shm_malloc(sizeof(r_hash_slot)*(r_hash_size+1)); // 1 slot extra for the WildPSI
	memset(registrar,0,sizeof(r_hash_slot)*(r_hash_size+1));
	
	for(i=0;i<r_hash_size+1;i++){
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
	for(i=0;i<r_hash_size+1;i++){
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
	// LOG(L_CRIT,"GET %d\n",hash);
	lock_get(registrar[(hash)].lock);
	// LOG(L_CRIT,"GOT %d\n",hash);	
}

/**
 * UnLocks the required slot of the registrar
 * @param hash - the index of the slot
 */
inline void r_unlock(unsigned int hash)
{
	lock_release(registrar[(hash)].lock);
	// LOG(L_CRIT,"RELEASED %d\n",hash);	
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
	
	STR_SHM_DUP( &(s->subscriber), &(subscriber),"new_r_subscriber");
	
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
r_contact* add_r_contact(struct sip_msg* msg, r_public *p,str uri,int expires,str ua,str path)
{
	r_contact *c;
	if (!p) return 0;
	
	c = new_r_contact(uri,expires,ua,path);
	if (!c) return 0;

	create_user_pref(msg, c) ;

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
r_contact* update_r_contact(struct sip_msg* msg,  r_public *p,str uri,int *expires, str *ua,str *path)
{
	r_contact *c;
	
	if (!p) return 0;
	c = get_r_contact(p,uri);
	if (!c){
		if (expires && ua && path)
			return add_r_contact(msg, p,uri,*expires,*ua,*path);
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
	/* del_user_pref(c);		     gets bug indication, check allocated mem struct of user_pref! */ 
	shm_free(c);
}


void free_regexp_list(t_regexp_list **regexp);

/*
 * This function just needed because for regexp * by itself has no meaning
 * it has to be .* that means any character none or more times
 *  * ->.*
 *  ? ->.  or .?  we do .? because it is more logical
 *  . -> \.
 *  and we terminate it with \0 because its for regcomp
*/
char *escape_all_wildcards_and_copy(str wpsi)
{
	char *res;
	int i,j,len;
	len=1+wpsi.len; // the \0
	j=0;
	//first lets see how many wildcards there are to see how much space is needed
	for (i=0;i<wpsi.len;i++)
	{
		switch(wpsi.s[i])
		{
			case '?': case '*':// case '.':
				len++;
				break;
		}
	}
	res=shm_malloc(len);
	// then do the copy
	for (i=0;i<wpsi.len;i++)
	{
		switch(wpsi.s[i]) {
			case '.':
				res[j++]='\\';
				res[j++]=wpsi.s[i];
				break;
			case '?' : case '*':
				res[j++]='.';
			default :
				res[j++]=wpsi.s[i];	
				break;
		}
	}
	res[j]=0;
	return res;
}

/**
 * Creates a registrar r_public.
 * This does not insert it in the registrar
 * @param aor - the public identity as address of record
 * @param reg_state - desired registration state
 * @param s - the ims subscription to which this belongs
 * @returns - the r_public created, NULL on error
 */
 /**
  * In case of a Wildcarded PSI, it compiles the regexp for quick processing
  */
r_public* new_r_public(str aor, enum Reg_States reg_state, ims_subscription *s)
{
	r_public *p;
	
	/*I didn't want to make this so complicated..*/
	t_regexp_unit *newwpsi;
	char *temp=0;
	int i,j;
	//int len;
	//int errcode;
	
	p = shm_malloc(sizeof(r_public));
	if (!p){
		LOG(L_ERR,"ERR:"M_NAME":new_r_public(): Unable to alloc %d bytes\n",
			sizeof(r_public));
		goto error;
	}	
	memset(p,0,sizeof(r_public));
	if (s->wpsi !=1)
	{
		//very sensible thing to do...
		p->hash = get_aor_hash(aor,r_hash_size);
		
	} else {
		
		//here it gets a bit complicated
		
		/* if its a wildcarded psi....*/
		p->hash=r_hash_size;
		/*will be in the last slot*/
		p->regexp=shm_malloc(sizeof(t_regexp_list));
		p->regexp->head=NULL;
		p->regexp->tail=NULL;
		/*lets compile the regular expression and compile it..*/
		for (i=0;i<s->service_profiles_cnt;i++)
		{
			for (j=0;j<s->service_profiles[i].public_identities_cnt;j++)
			{
				if (s->service_profiles[i].public_identities[j].wildcarded_psi.s && s->service_profiles[i].public_identities[j].wildcarded_psi.len>0)
				 {
					// for each wildcardpsi we add a member to the regexp list in the r_public
					newwpsi=shm_malloc(sizeof(t_regexp_unit));
					newwpsi->next=NULL;
					newwpsi->prev=NULL;
					// now we have to deal with using an external function that expects
					// some string with a finishing \0 character
					//len=s->service_profiles[i].public_identities[j].wildcarded_psi.len;
					temp=escape_all_wildcards_and_copy(s->service_profiles[i].public_identities[j].wildcarded_psi);
					//temp=shm_malloc(len+1);
					//memcpy(temp,s->service_profiles[i].public_identities[j].wildcarded_psi.s,len);
					//temp[len]=0;
					LOG(L_DBG,"New_r_public : REGULAR EXPRESSION is %s\n\n",temp);
					/*errcode=regcomp(&(newwpsi->exp),temp,REG_ICASE|REG_EXTENDED|REG_NOSUB);
					if(errcode!=0)
					{
						// there was an error
						LOG(L_DBG,"there was at least ONE error in regcomp %i\n",errcode);
						//shm_free(temp); temp=NULL;
						len=regerror(errcode,&(newwpsi->exp),NULL,0);
						temp=shm_malloc(len);
						regerror(errcode,&(newwpsi->exp),temp,len);
						LOG(L_DBG,"and that was %s",temp);
						shm_free(temp); temp=NULL;
						
						shm_free(newwpsi);
						goto error;
					}*/
					newwpsi->s=temp;
					//LOG(L_DBG,"there was NO error in regcomp\n");
					//shm_free(temp); temp=NULL;
					newwpsi->prev=p->regexp->tail;
					if (p->regexp->tail)
					{
						p->regexp->tail->next=newwpsi;
						p->regexp->tail=newwpsi;
					} else {
						// no tail means no head too...
						p->regexp->head=newwpsi;
						p->regexp->tail=newwpsi;
					}
					
				}
			}
		}
		
		
		
			
		
	}
	//LOG(L_DBG,"after the mess inside new_r_public\n");
	p->aor.s = shm_malloc(aor.len); // I wonder if this should be done in the case of wildpsi
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
	//LOG(L_DBG,"new_r_public we are returning %p\n",p);	
	return p;
error:
	if (p){
		if (p->aor.s) shm_free(p->aor.s);
		// free the regexp list of shit!	
		if (p->regexp) free_regexp_list(&(p->regexp));
		//if (temp) shm_free(temp);
		shm_free(p);
	}
	//LOG(L_DBG,"new_r_public we are returning error\n");	
	return 0;	
}




/**
 * Searches in the last hash slot (Wildcarded PSI) for a match on the aor within the regular expressions
 * \note Aquires the lock on the last hash slot on success, so release it when you are done.
 * @param aor - the address of record to look for
 * @returns - the r_public found, 0 if not found
 */

r_public* get_matching_wildcard_psi(str aor)
{
	r_public *p=0;
	t_regexp_unit *t;
	regex_t exp;
	char *c,*temp;
	int len;
	int errcode;
	
	p = registrar[r_hash_size].head;
	
	r_lock(r_hash_size);
	
	while (p)
	{
		
		for(t=p->regexp->head;t;t=t->next)
		{
			LOG(L_DBG,"get_matching_wildcard_psi match %.*s with %s\n",aor.len,aor.s,t->s);
			
			c=shm_malloc((aor.len)+1);
			memcpy(c,aor.s,aor.len);
			c[aor.len]='\0';
			
			
					errcode=regcomp(&(exp),t->s,REG_ICASE|REG_EXTENDED|REG_NOSUB);
					if(errcode!=0)
					{
						// there was an error
						LOG(L_DBG,"there was at least ONE error in regcomp %i\n",errcode);
						//shm_free(temp); temp=NULL;
						len=regerror(errcode,&(exp),NULL,0);
						temp=shm_malloc(len);
						regerror(errcode,&(exp),temp,len);
						LOG(L_DBG,"and that was %s",temp);
						shm_free(temp); temp=NULL;
						//shm_free(newwpsi);
						r_unlock(r_hash_size);
						return NULL;
					}
					
			
			if (regexec(&(exp),c,0,NULL,0)==0)
			{
				//LOG(L_DBG,"A match has been produced!!!!!\n");
				shm_free(c);
				return p;
			} else 
			{
				//LOG(L_ERR,"there was no match\n");
			}
			shm_free(c);				 
		}		
		p=p->next;
	}
	LOG(L_ERR,"get_r_public (Trying wildcard PSI) found no match\n");
	r_unlock(r_hash_size);
	
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
	
	/*
	 * Here i plan to put the function that tries to match the aor with the wPSI
	 * There might be an issue 
	 * if i return this locked, then i have to wait for the lock in the next one...
	 * mmmh... might be an issue
	 * */
	 if (scscf_support_wildcardPSI)
	 	return(get_matching_wildcard_psi(aor));
	 else
	 	return 0;
	 
}

/* This function gets a r_public record from the Wildcarded PSI slot whose  Public Identity
 * is equal to the one given as an argument. 
 * Main costumer is update_r_public, because there you dont want to find an AOR to send a message
 * but you really want the record..  first of being use check its a WildCardPSI
 * \note Aquires the lock of the hash on success, so release it when you are done
 * @param pi -the public identity of the WildCardPSI
 * @returns - the r_public found, 0 if not found
 *
*/

r_public* get_r_public_wpsi(str pi)
{
	r_public *p=0;
	r_lock(r_hash_size);
	p = registrar[r_hash_size].head;
	while(p){
		if (p->aor.len == pi.len &&
			strncasecmp(p->aor.s,pi.s,pi.len)==0) return p;
		p = p->next;
	}
	r_unlock(r_hash_size);
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
	LOG(L_ERR,"no match found but get_r_public_nolock doesn't look in the WildCardedPSI area\n");
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
	//LOG(L_CRIT,"update_r_public():with aor %.*s\n",aor.len,aor.s);
	
	if ((*s)->wpsi) {
		p = get_r_public_wpsi(aor);
	} else {
		p = get_r_public(aor);
	}
	
	if (!p){
		//LOG(L_DBG,"updating a new r_public profile\n");			 
		if (reg_state && *reg_state && *reg_state!=NOT_REGISTERED && s){
			p = add_r_public(aor,*reg_state,*s);
			
			if (!p) return p;			
			if (ccf1) {
				if (p->ccf1.s) shm_free(p->ccf1.s);
				STR_SHM_DUP( &(p->ccf1), ccf1,"SHM CCF1");
			}
			if (ccf2) {
				if (p->ccf2.s) shm_free(p->ccf2.s);
				STR_SHM_DUP( &(p->ccf2), ccf2,"SHM CCF2");
			}
			if (ecf1) {
				if (p->ecf1.s) shm_free(p->ecf1.s);
				STR_SHM_DUP( &(p->ecf1), ecf1,"SHM ECF1");
			}
			if (ecf2) {
				if (p->ecf2.s) shm_free(p->ecf2.s);
				STR_SHM_DUP( &(p->ecf2), ecf2,"SHM ECF2");
			}
			//LOG(L_DBG,"update_r_public():  it was actually adding\n");
			return p;
		}
		else return 0;
	}else{
		//LOG(L_DBG,"updating a not so new r_public profile\n");		
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
			
			
			 if ((*s)->wpsi)
			 {	
			 	
			 	 p->s=NULL; 
			 	 			 	 
			 	 if (p->prev) p->prev->next=p->next;
			 	 else registrar[r_hash_size].head=p->next;	
			 	 if (p->next) p->next->prev=p->prev;
			 	 else registrar[r_hash_size].tail=p->prev;
			 	 
			 	 free_r_public(p);
			 	 r_unlock(r_hash_size);			 
			 	 p=add_r_public(aor,0,*s);
			 	 
			 }
			
			
		}
		if (ccf1) {
			if (p->ccf1.s) shm_free(p->ccf1.s);
			STR_SHM_DUP( &(p->ccf1), ccf1,"SHM CCF1");
		}
		if (ccf2) {
			if (p->ccf2.s) shm_free(p->ccf2.s);
			STR_SHM_DUP( &(p->ccf2), ccf2,"SHM CCF2");
		}
		if (ecf1) {
			if (p->ecf1.s) shm_free(p->ecf1.s);
			STR_SHM_DUP( &(p->ecf1), ecf1,"SHM ECF1");
		}
		if (ecf2) {
			if (p->ecf2.s) shm_free(p->ecf2.s);
			STR_SHM_DUP( &(p->ecf2), ecf2,"SHM ECF2");
		}
		//LOG(L_DBG,"update_r_public():    return normaly\n");
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
				STR_SHM_DUP( &(p->ccf1), ccf1,"SHM CCF1");
			}
			if (ccf2) {
				if (p->ccf2.s) shm_free(p->ccf2.s);
				STR_SHM_DUP( &(p->ccf2), ccf2,"SHM CCF2");
			}
			if (ecf1) {
				if (p->ecf1.s) shm_free(p->ecf1.s);
				STR_SHM_DUP( &(p->ecf1), ecf1,"SHM ECF1");
			}
			if (ecf2) {
				if (p->ecf2.s) shm_free(p->ecf2.s);
				STR_SHM_DUP( &(p->ecf2), ecf2,"SHM ECF2");
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
			STR_SHM_DUP( &(p->ccf1), ccf1,"SHM CCF1");
		}
		if (ccf2) {
			if (p->ccf2.s) shm_free(p->ccf2.s);
			STR_SHM_DUP( &(p->ccf2), ccf2,"SHM CCF2");
		}
		if (ecf1) {
			if (p->ecf1.s) shm_free(p->ecf1.s);
			STR_SHM_DUP( &(p->ecf1), ecf1,"SHM ECF1");
		}
		if (ecf2) {
			if (p->ecf2.s) shm_free(p->ecf2.s);
			STR_SHM_DUP( &(p->ecf2), ecf2,"SHM ECF2");
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

void free_regexp_list(t_regexp_list **regexp)
{
	t_regexp_unit *un,*dos;
	
	un=(*regexp)->head;
		while (un)
		{
			dos=un->next;
			if (un->s) shm_free(un->s);
			//if (1) regfree(&(un->exp));
			shm_free(un);
			un=dos;
		}
		shm_free(*regexp);
	
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
	
	if (p->regexp) free_regexp_list(&(p->regexp));	

	
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
					LOG(log_level,ANSI_GREEN"INF:"M_NAME":         UP: {"ANSI_BLUE" iptv[%s], text[%s], data[%s], audio[%s], video[%s], control[%s], isfocus[%s], automata[%s], application[%s]\n          mobility[%s]"ANSI_GREEN" }\n", 
						((c->user_pref->iptv == TRUE)?"yes":"no"),
						((c->user_pref->text == TRUE)?"yes":"no"),
						((c->user_pref->data == TRUE)?"yes":"no"),
						((c->user_pref->audio == TRUE)?"yes":"no"),
						((c->user_pref->video == TRUE)?"yes":"no"),
						((c->user_pref->control == TRUE)?"yes":"no"),
						((c->user_pref->isfocus == TRUE)?"yes":"no"),
						((c->user_pref->automata == TRUE)?"yes":"no"),
						((c->user_pref->application== TRUE)?"yes":"no"),
						((c->user_pref->mobility.val_list[0] == MOBILE)?"mobile":"fixed")  
						) ;
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

