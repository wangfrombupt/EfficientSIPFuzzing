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
 * Proxy-CSCF - Registrar Storage
 * 
 * This is reversed registrar in the sense that contacts are pointing to public identities
 * and not he other way around. This is required as the P-CSCF retrieves the identities
 * based on the contacts. This being the operation most employed, it doesn't make
 * sense to keep the registrar in the normal order.
 * 
 * The Registrar is kept as a hash table. If you have a large number of users, increase
 * its size for better search times.
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#include <time.h>

#include "mod.h"
#include "registrar_storage.h"
//#include "registrar_notify.h"
#include "nat_helper.h"
#include "security.h"
#include "dlg_state.h"

time_t time_now;				/**< current time 							*/
r_hash_slot *registrar=0;		/**< the actual registrar					*/
int 	   r_hash_size=128;		/**< number of hash slots in the registrar	*/


/**
 * Update the time.
 */
inline void r_act_time()
{
	time_now=time(0);
}

/**
 * Returns if a contact is valid.
 * Caller should do actualization of time_now
 * @param c - the contact to check
 * @returns 1 if valid, else 0
 */
inline int r_valid_contact(r_contact *c)
{
//	LOG(L_ERR,"%.*s -> %ld\n",c->uri.len,c->uri.s,c->expires-time_now);
	return (c->reg_state!=NOT_REGISTERED && c->reg_state!=DEREGISTERED)&&(c->expires>time_now);
}

/**
 * Returns if a contact is valid and registered.
 * Caller should do actualization of time_now
 * @param c - the contact to check
 * @returns 1 if valid, else 0
 */
inline int r_reg_contact(r_contact *c)
{
//	LOG(L_ERR,"%.*s -> %ld\n",c->uri.len,c->uri.s,c->expires-time_now);
	return (c->reg_state==REGISTERED)&&(c->expires>time_now);
}

/**
 * Computes the hash for a contact.
 * @param aor - the string of the contact
 * @param port - the port of the contact
 * @param transport - transport for the contact - ignored for now
 * @param hash_size - size of the hash, to % with
 * @returns the hash for the contact
 */
inline unsigned int get_contact_hash(str aor,int port,int transport,int hash_size)
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
   v = port;
   h_inc;
//   v = transport;
//   h_inc;/* because xten does not care about transport */

   h=((h)+(h>>11))+((h>>13)+(h>>23));
   return (h)%hash_size;
#undef h_inc 
}

/**
 * Initialize the registrar.
 * @param hash_size - number of hash slots to create
 * @returns 1 if 0, 0 if not
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
	r_contact *c,*nc;
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
			c = registrar[i].head;
			while(c){
				nc = c->next;
				free_r_contact(c);
				c = nc;
			}
		r_unlock(i);
		lock_dealloc(registrar[i].lock);
	}
	shm_free(registrar);	
}

/**
 * Lock a registrar hash slot.
 * @param hash - index to lock
 */
inline void r_lock(unsigned int hash)
{
//	LOG(L_CRIT,"GET %d\n",hash);
	lock_get(registrar[hash].lock);
//	LOG(L_CRIT,"GOT %d\n",hash);
}
/**
 * UnLock a registrar hash slot.
 * @param hash - index to unlock
 */
inline void r_unlock(unsigned int hash)
{
//	LOG(L_CRIT,"REL %d\n",hash);	
	lock_release(registrar[hash].lock);
}

/**
 * Creates a registrar public record.
 * This does not insert it in the registrar
 * @param aor - the public identity as address of record
 * @param is_default - if this is the default public id
 * @returns - the r_public created, NULL on error
 */
r_public* new_r_public(str aor, int is_default)
{
	r_public *p;
	
	p = shm_malloc(sizeof(r_public));
	if (!p){
		LOG(L_ERR,"ERR:"M_NAME":new_r_public(): Unable to alloc %d bytes\n",
			sizeof(r_public));
		goto error;
	}	
	memset(p,0,sizeof(r_public));
		
	p->aor.s = shm_malloc(aor.len);
	if (!p->aor.s){
		LOG(L_ERR,"ERR:"M_NAME":new_r_public(): Unable to alloc %d bytes\n",
			aor.len);
		goto error;
	}
	p->aor.len = aor.len;
	memcpy(p->aor.s,aor.s,aor.len);
	
	p->is_default = is_default;
	
		
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
 * \note Must be called with a lock on the domain to avoid races
 * @param c - the r_contact to look into
 * @param aor - the address of record to look for
 * @returns - the r_public found, 0 if not found
 */
r_public* get_r_public(r_contact *c, str aor)
{
	r_public *p=0;
	if (!c) return 0;
	p = c->head;
	while(p){
		if (p->aor.len == aor.len &&
			strncasecmp(p->aor.s,aor.s,aor.len)==0) return p;
		p = p->next;
	}
	return 0;
}


/**
 * Creates and Adds a new r_public record. 
 * \note Must be called with a lock on the domain to avoid races
 * \note When calling be sure that get_r_public(d,aor) returns 0, 
 * to avoid unreachable duplicates
 * @param c - the r_contact to add to
 * @param aor - the address of record
 * @param is_default - if this is the default contact
 * @returns the newly added r_public, 0 on error
 */
r_public* add_r_public(r_contact *c,str aor,int is_default)
{
	r_public *p;
	if (!c) return 0;
	p = new_r_public(aor,is_default);
	if (!p) return 0;
	p->next=0;
	p->prev=c->tail;
	if (c->tail) c->tail->next = p;
	c->tail = p;
	if (!c->head) c->head=p;
	
	return p;
}

/**
 * Updates the r_public with the new is_default.
 * If not found, it will be inserted
 * \note Must be called with a lock on the domain to avoid races
 * @param c - the r_contact to add to
 * @param aor - the address of record
 * @param is_default - if this is the default contact
 * @returns the newly added r_public, 0 on error
 */
r_public* update_r_public(r_contact *c,str aor,int *is_default)
{
	r_public *p;

	if (!c) return 0;
	p = get_r_public(c,aor);
	if (!p){
		if (is_default)
			return add_r_public(c,aor,*is_default);
		else return 0;
	}else{
		if (is_default)	p->is_default = *is_default;
		return p;
	}
}


/**
 * Drops and deallocates a r_public.
 * \note Must be called with a lock on the domain to avoid races
 * \note When calling be sure that get_r_public(d,aor) returns p,
 * to avoid a bogus removal
 * @param c - the r_contact that this r_public is attached currently to
 * @param p - the r_public to remove
 */
void del_r_public(r_contact *c,r_public *p)
{
	if (c->head == p) c->head = p->next;
	else p->prev->next = p->next;
	if (c->tail == p) c->tail = p->prev;
	else p->next->prev = p->prev;
	free_r_public(p);
}

/**
 * Frees memory taken by a r_public aor structure
 * \note Must be called with a lock on the domain to avoid races
 * @param p - the r_public to be deallocated
 */
void free_r_public(r_public *p)
{
	if (!p) return;
	if (p->aor.s) shm_free(p->aor.s);
	shm_free(p);
}

/**
 * Creates a registrar ipsec container.
 * This does not insert it in the registrar
 * the strings are duplicated in shm
 * @param spi_uc - SPI for UserEndpoint Client
 * @param spi_us - SPI for UserEndpoint Server
 * @param spi_pc - SPI for P-CSCF Client
 * @param spi_ps - SPI for UserEndpoint Server
 * @param port_uc - port for UserEndpoint Client
 * @param port_us - port for UserEndpoint Server
 * @param ealg_setkey - Cypher Algorithm
 * @param r_ealg - received Cypher Algorithm
 * @param ck_esp - Cypher Key
 * @param r_alg - received Integrity Algorithm
 * @param ik_esp - Integrity Key
 * @returns the new r_ipsec* container or NULL on error
 */
r_ipsec* new_r_ipsec(int spi_uc,int spi_us,int spi_pc,int spi_ps,int port_uc,int port_us,
	str ealg_setkey,str r_ealg, str ck_esp,str alg_setkey,str r_alg, str ik_esp)
{
	r_ipsec *ipsec;
	
	ipsec = shm_malloc(sizeof(r_ipsec));
	if (!ipsec) {
		LOG(L_ERR,"ERR:"M_NAME":new_r_ipsec(): Unable to alloc %d bytes\n",
			sizeof(r_ipsec));
		return 0;
	}
	memset(ipsec,0,sizeof(r_ipsec));
	
	ipsec->spi_uc = spi_uc;
	ipsec->spi_us = spi_us;
	ipsec->spi_pc = spi_pc;
	ipsec->spi_ps = spi_ps;
	ipsec->port_uc = port_uc;
	ipsec->port_us = port_us;
	
	
	STR_SHM_DUP(ipsec->ealg,ealg_setkey,"new_r_ipsec");
	STR_SHM_DUP(ipsec->r_ealg,r_ealg,"new_r_ipsec");
	STR_SHM_DUP(ipsec->alg,alg_setkey,"new_r_ipsec");
	STR_SHM_DUP(ipsec->r_alg,r_alg,"new_r_ipsec");
	STR_SHM_DUP(ipsec->ck,ck_esp,"new_r_ipsec");
	STR_SHM_DUP(ipsec->ik,ik_esp,"new_r_ipsec");
			
	return ipsec;
out_of_memory:
	if (ipsec) free_r_ipsec(ipsec);
	return 0;	
}
		
/**
 * Frees memory taken by a r_ipsec structure
 * \note Must be called with a lock on the domain to avoid races
 * @param ipsec - the r_public to be deallocated
 */	
void free_r_ipsec(r_ipsec *ipsec)
{
	if (!ipsec) return;
	if (ipsec->ealg.s) shm_free(ipsec->ealg.s);
	if (ipsec->r_ealg.s) shm_free(ipsec->r_ealg.s);
	if (ipsec->alg.s) shm_free(ipsec->alg.s);
	if (ipsec->r_alg.s) shm_free(ipsec->r_alg.s);
	if (ipsec->ck.s) shm_free(ipsec->ck.s);	
	if (ipsec->ik.s) shm_free(ipsec->ik.s);	
	shm_free(ipsec);
}

/**
 * Creates a registrar tls container.
 * This does not insert it in the registrar
 * the strings are duplicated in shm
 * @param port_tls - port for UserEndpoint Client
 * @param session_hash - TLS Session hash
 * @returns the new r_tls* container or NULL on error
 */
r_tls* new_r_tls(int port_tls, unsigned long session_hash)
{
	r_tls *tls;
	
	tls = shm_malloc(sizeof(r_tls));
	if (!tls) {
		LOG(L_ERR,"ERR:"M_NAME":new_r_tls(): Unable to alloc %d bytes\n",
			sizeof(r_tls));
		return 0;
	}
	memset(tls,0,sizeof(r_tls));
	
	tls->port_tls = port_tls;
	tls->session_hash = session_hash;		
	return tls;
}
		
/**
 * Frees memory taken by a r_tls structure
 * \note Must be called with a lock on the domain to avoid races
 * @param tls - the r_public to be deallocated
 */	
void free_r_tls(r_tls *tls)
{
	if (!tls) return;
	shm_free(tls);
}

r_security *new_r_security(str sec_header,r_security_type type,float q)
{
	r_security *s=0;

	s = shm_malloc(sizeof(r_security));
	if (!s){
		LOG(L_ERR,"ERR:"M_NAME":save_contact_security: Error allocating %d bytes.\n",sizeof(r_security));
		goto error ;
	}
	memset(s,0,sizeof(r_security));
	
	s->type = type;
	s->q = q;
	STR_SHM_DUP(s->sec_header,sec_header,"save_contact_security");
	if (!s->sec_header.s) goto error;	
	return s;
error:
out_of_memory:
	if (s) shm_free(s);
	return 0;
}


void free_r_security(r_security *s)
{
	if (!s) return;
	switch (s->type){
		case SEC_NONE:
			break;
		case SEC_TLS:
			if (s->data.tls) free_r_tls(s->data.tls);
			break;
		case SEC_IPSEC:
			if (s->data.ipsec) free_r_ipsec(s->data.ipsec);
			break;		
	}
	if (s->sec_header.s) shm_free(s->sec_header.s);
	shm_free(s);
}

/**
 * Searches for a r_contact contact and returns it.
 * \note Will lock the hash_slot if found! So release it when you are done!
 * @param host - the IP in string format
 * @param port - the port number
 * @param transport - the transport type
 * @returns - the r_contact found, 0 if not found
 */
r_contact* get_r_contact(str host,int port,int transport)
{
	r_contact *c=0;
	unsigned int hash;
	if (!registrar) return 0;
	hash = get_contact_hash(host,port,transport,r_hash_size);
	r_lock(hash);
	c = registrar[hash].head;
	while(c){
		if (c->port == port &&
//			c->transport == transport && /* because xten doesn't care about protocols */ 
			c->host.len == host.len &&
			strncasecmp(c->host.s,host.s,host.len)==0) return c;
		c = c->next;
	}
	r_unlock(hash);
	return 0;
}

/**
 * Creates a registrar contact 
 * This does not insert it in the registrar
 * @param host - the host part of the contact, in string
 * @param port - the port number of the contact
 * @param transport - the transport of the contact
 * @param uri - URI of the contact
 * @param reg_state - Registration state
 * @param expires - expires in
 * @param service_route - array of service routes
 * @param service_route_cnt - the size of the array above
 * @returns the new r_contact* or NULL on error
 */
r_contact* new_r_contact(str host,int port,int transport,str uri,enum Reg_States reg_state,int expires,
	str *service_route,int service_route_cnt)
{
	r_contact *c;
	int i;
	
	c = shm_malloc(sizeof(r_contact));
	if (!c) {
		LOG(L_ERR,"ERR:"M_NAME":new_r_contact(): Unable to alloc %d bytes\n",
			sizeof(r_contact));
		goto error;
	}
	memset(c,0,sizeof(r_contact));
	
	STR_SHM_DUP(c->host,host,"new_r_contact");
	c->port = port;
	c->transport = transport;
	c->hash = get_contact_hash(host,port,transport,r_hash_size);
	STR_SHM_DUP(c->uri,uri,"new_r_contact");		
	c->reg_state = reg_state;
	c->expires = expires;	
		
	if (service_route_cnt && service_route){
		c->service_route = shm_malloc(service_route_cnt*sizeof(str));
		if (!c->service_route){
			LOG(L_ERR,"ERR:"M_NAME":new_r_contact(): Unable to alloc %d bytes\n",
				service_route_cnt*sizeof(str));
			goto error;
		}
		for(i=0;i<service_route_cnt;i++)
			STR_SHM_DUP(c->service_route[i],service_route[i],"new_r_contact");
		c->service_route_cnt = service_route_cnt;
	}
		
	return c;
error:
out_of_memory:
	if (c){
		free_r_contact(c);			
	}
	return 0;
}

/**
 * Creates and Adds a new r_contact 
 * \note Aquires the lock on the hash_slot on success, so release it when you are done.
 * \note When calling be sure that get_r_contact(...) returns 0, to avoid unreachable duplicates
 * @param host - the host part of the contact, in string
 * @param port - the port number of the contact
 * @param transport - the transport of the contact
 * @param uri - URI of the contact
 * @param reg_state - Registration state
 * @param expires - expires in
 * @param service_route - array of service routes
 * @param service_route_cnt - the size of the array above
 * @param pinhole - NAT pin hole
 * @returns the newly added r_contact, 0 on error
 */
r_contact* add_r_contact(str host,int port,int transport,str uri,
	enum Reg_States reg_state,int expires,str *service_route,int service_route_cnt, r_nat_dest *pinhole)
{
	r_contact *c;

	if (!registrar) return 0;
	c = new_r_contact(host,port,transport,uri,reg_state, expires, service_route, service_route_cnt);
	if (!c) return 0;
	c->next=0;
	r_lock(c->hash);
		c->prev=registrar[c->hash].tail;
		if (c->prev) c->prev->next = c;
		registrar[c->hash].tail = c;
		if (!registrar[c->hash].head) registrar[c->hash].head=c;
		c->pinhole = pinhole;
	return c;
}

/**
 * Updates the r_contact with the new states
 * If not found, it will be inserted
 * \note Aquires the lock on the hash_slot on success, so release it when you are done.
 * @param host - the host part of the contact, in string
 * @param port - the port number of the contact
 * @param transport - the transport of the contact
 * @param uri - URI of the contact
 * @param reg_state - Registration state
 * @param expires - expires in
 * @param service_route - array of service routes
 * @param service_route_cnt - the size of the array above
 * @param pinhole - NAT pin hole
 * @returns the updated added r_contact, 0 on error
 */
r_contact* update_r_contact(str host,int port,int transport,
	str *uri,enum Reg_States *reg_state,int *expires,str **service_route,int *service_route_cnt, r_nat_dest **pinhole)
{
	r_contact *c=0;
	int i;
	
	
	c = get_r_contact(host,port,transport);
	if (!c){
		if (uri&&reg_state && expires && service_route && service_route_cnt)
			return pinhole?add_r_contact(host,port,transport,*uri,*reg_state,*expires,*service_route,*service_route_cnt, *pinhole):
						   add_r_contact(host,port,transport,*uri,*reg_state,*expires,*service_route,*service_route_cnt, 0);
		else return 0;
	}else{
		/* first drop the old temporary public ids */
		if (c->reg_state==DEREGISTERED && reg_state && *reg_state==REGISTERED){
			while(c->head){
				del_r_public(c,c->head);
			}
		}

		if (uri) {
			if (c->uri.s) shm_free(c->uri.s);
			STR_SHM_DUP(c->uri,*uri,"update_r_contact");
		}
		if (reg_state) c->reg_state = *reg_state;
		if (expires) c->expires = *expires;
		if (service_route){
			if (c->service_route){
				for(i=0;i<c->service_route_cnt;i++)
					if (c->service_route[i].s) shm_free(c->service_route[i].s);
				shm_free(c->service_route);
				c->service_route=0;
				c->service_route_cnt=0;
			}
			if (*service_route&&*service_route_cnt){
				c->service_route = shm_malloc((*service_route_cnt)*sizeof(str));
				if (!c->service_route){
					LOG(L_ERR,"ERR:"M_NAME":new_r_contact(): Unable to alloc %d bytes\n",
						(*service_route_cnt)*sizeof(str));					
				}else{
					for(i=0;i<*service_route_cnt;i++)
						STR_SHM_DUP(c->service_route[i],(*service_route)[i],"new_r_contact");
					c->service_route_cnt = *service_route_cnt;											
				}			
			}
		}
		if (pinhole) c->pinhole = *pinhole;
		return c;
	}
	
out_of_memory:
	return c;	
}

/**
 * Updates the r_contact with the new security values.
 * @param host - the host part of the contact, in string
 * @param port - the port number of the contact
 * @param transport - the transport of the contact
 * @param uri - URI of the contact
 * @param reg_state - Registration state
 * @param expires - expires in
 * @param ipsec - the new IPSec tunnel
 * @returns the newly added r_public, 0 on error
 */
r_contact* update_r_contact_sec(str host,int port,int transport,
	str *uri,enum Reg_States *reg_state,int *expires,
	r_security *s)
{
	r_contact *c;
	
	c = get_r_contact(host,port,transport);
	if (!c){
		if (uri&&reg_state){
			c = add_r_contact(host,port,transport,*uri,*reg_state,*expires,(str*) 0,0,0);
			c->security_temp = s;
			r_unlock(c->hash);
			return c;
		}
		else return 0;
	}else{
		if (c->security_temp){
			P_security_drop(c,c->security_temp);
			free_r_security(c->security_temp);
		}		
		c->security_temp = s;
		r_unlock(c->hash);
		return c;
	}
}

/** 
 * Gets the contact structure with the uri the same as the one given.
 * \note Aquires the lock on the hash_slot on success, so release it when you are done.
 * @param host - the host part of the contact, in string
 * @param port - the port number of the contact
 * @param transport - the transport of the contact 
 * @return the r_nat_dest on success or NULL on failure
 */
r_nat_dest* get_r_nat_pinhole(str host, int port, int transport) {
	r_contact * contact;
	
	contact = get_r_contact(host, port, transport);
	if(contact != NULL) {
		return contact -> pinhole;
	}
	LOG(L_ERR, "ERR"M_NAME":get_r_nat_pinhole: cannot find contact %d://%.*s:%d\n", transport,host.len, host.s, port);
	return NULL;
}


/**
 * Drops and deallocates a r_contact.
 * \note Don't forget to release the lock on the !!OLD!! hash value
 * @param c - the r_contact to remove
 */
void del_r_contact(r_contact *c)
{
	P_drop_all_dialogs(c->host,c->port,c->transport);
	if (registrar[c->hash].head == c) registrar[c->hash].head = c->next;
	else c->prev->next = c->next;
	if (registrar[c->hash].tail == c) registrar[c->hash].tail = c->prev;
	else c->next->prev = c->prev;
	free_r_contact(c);
}


/**
 * Frees memory taken by a r_contact structure
 * @param c - the r_contact to be deallocated
 */
void free_r_contact(r_contact *c)
{
	r_public *p,*n;
	int i;

	if (!c) return;
	if (c->pinhole) shm_free(c->pinhole);
	if (c->host.s) shm_free(c->host.s);
	if (c->uri.s) shm_free(c->uri.s);
	if (c->service_route){
		i=0;
		for(i=0;i<c->service_route_cnt;i++)
			if (c->service_route[i].s)
				shm_free(c->service_route[i].s);
		shm_free(c->service_route);
	}
	p = c->head;
	while(p){
		n = p->next;
		free_r_public(p);
		p = n;
	}
	if (c->security_temp) free_r_security(c->security_temp);
	if (c->security) free_r_security(c->security);
	shm_free(c);
}




/**
 * Debug print the contents of the entire registrar.
 */
void print_r(int log_level)
{
	r_public *p;
	r_contact *c;
	int i,j;

	if (debug<log_level) return; /* to avoid useless calls when nothing will be printed */	
	r_act_time();
	LOG(log_level,"INF:"M_NAME":----------  Registrar Contents begin --------\n");
	if (!registrar) return;
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
		c = registrar[i].head;
		while(c){
			LOG(log_level,ANSI_GREEN"INF:"M_NAME":[%4d] C: <"ANSI_RED"%d://%.*s:%d"ANSI_GREEN"> Exp:["ANSI_MAGENTA"%4ld"ANSI_GREEN"] R:["ANSI_MAGENTA"%2d"ANSI_GREEN"] <%.*s>\n",i,
			c->transport,c->host.len,c->host.s,c->port,
				c->expires-time_now,c->reg_state,
				c->uri.len,c->uri.s);					
			for(j=0;j<c->service_route_cnt;j++)
				LOG(log_level,ANSI_GREEN"INF:"M_NAME":         SR: <"ANSI_YELLOW"%.*s"ANSI_GREEN">\n",c->service_route[j].len,c->service_route[j].s);
			if(c->pinhole) {
				if (c->pinhole->nat_addr.len>=4)
					LOG(log_level,ANSI_GREEN"INF:"M_NAME":        NAT:<"ANSI_CYAN"%d.%d.%d.%d:%d"ANSI_GREEN">\n",
						c->pinhole->nat_addr.u.addr[0],
						c->pinhole->nat_addr.u.addr[1],
						c->pinhole->nat_addr.u.addr[2],
						c->pinhole->nat_addr.u.addr[3],
						(unsigned short)c->pinhole->nat_port);
				
			} 
			if (c->security){
				switch(c->security->type){
					case SEC_NONE:
						break;						
					case SEC_TLS:
						if (c->security->data.tls)
						LOG(log_level,ANSI_GREEN"INF:"M_NAME":        TLS: %.*s tls://%.*s:%d\n",
							c->security->sec_header.len,c->security->sec_header.s,
							c->host.len,c->host.s,c->security->data.tls->port_tls);
						break;
					case SEC_IPSEC:
						if (c->security->data.ipsec)
						LOG(log_level,ANSI_GREEN"INF:"M_NAME":        IPSec: UAS: %d:%d->%d  UAC: %d:%d<-%d E[%.*s] I[%.*s]\n",
							c->security->data.ipsec->spi_uc,c->security->data.ipsec->port_uc,c->security->data.ipsec->spi_ps,
							c->security->data.ipsec->spi_us,c->security->data.ipsec->port_us,c->security->data.ipsec->spi_pc,
							c->security->data.ipsec->ealg.len,c->security->data.ipsec->ealg.s,c->security->data.ipsec->alg.len,c->security->data.ipsec->alg.s);
				}					
			}
			if (c->security_temp){
				switch(c->security_temp->type){
					case SEC_NONE:
						break;						
					case SEC_TLS:
						if (c->security_temp->data.tls)
						LOG(log_level,ANSI_GREEN"INF:"M_NAME":        TLS: %.*s tls://%.*s:%d\n",
							c->security_temp->sec_header.len,c->security_temp->sec_header.s,
							c->host.len,c->host.s,c->security_temp->data.tls->port_tls);						
						break;
					case SEC_IPSEC:
						if (c->security_temp->data.ipsec)
						LOG(log_level,ANSI_GREEN"INF:"M_NAME":        Temp.IPSec: UAS: %d:%d->%d  UAC: %d:%d<-%d E[%.*s] I[%.*s]\n",
							c->security_temp->data.ipsec->spi_uc,c->security_temp->data.ipsec->port_uc,c->security_temp->data.ipsec->spi_ps,
							c->security_temp->data.ipsec->spi_us,c->security_temp->data.ipsec->port_us,c->security_temp->data.ipsec->spi_pc,
							c->security_temp->data.ipsec->ealg.len,c->security_temp->data.ipsec->ealg.s,c->security_temp->data.ipsec->alg.len,c->security_temp->data.ipsec->alg.s);
				}					
			}
			
			p = c->head;
			while(p){
				LOG(log_level,ANSI_GREEN"INF:"M_NAME":          P: D[%c] <"ANSI_BLUE"%.*s"ANSI_GREEN"> \n",
					(p->is_default?'X':' '),p->aor.len,p->aor.s);
				p = p->next;
			}
			c = c->next;
		}
		r_unlock(i);
	}
	LOG(log_level,"INF:"M_NAME":----------  Registrar Contents end ----------\n");
}

