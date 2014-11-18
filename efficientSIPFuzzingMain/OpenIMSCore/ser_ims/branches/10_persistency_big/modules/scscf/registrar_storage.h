/*
 * $Id: registrar_storage.h 516 2008-02-01 19:33:35Z albertoberlios $
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

  
#ifndef S_CSCF_REGISTRAR_STORAGE_H_
#define S_CSCF_REGISTRAR_STORAGE_H_

#include "../../sr_module.h"
#include "../../locking.h"
#include "../tm/tm_load.h"


#include "registrar_parser.h"
#include "registrar_notify.h"


/* REGISTRAR Structures */


/** Events for subscriptions */
enum {
	IMS_EVENT_NONE,				/**< Generic, no event					*/
	IMS_EVENT_REG				/**< Registration event					*/
}IMS_Events;

/** registrar subscriber structure */
typedef struct _r_subscriber {
	str subscriber;				/**< The subscribers contact 			*/
	char event;

	time_t expires;				/**< Time of expiration		 			*/
	dlg_t *dialog;				/**< tm dialog to send notify out		*/
	
	int version;				/**< Last version sent to this subs.	*/
	
	struct _r_subscriber *next;/**< the next subscriber in the list		*/
	struct _r_subscriber *prev;/**< the previous subscriber in the list	*/
} r_subscriber;

/** registrar contact structure */
typedef struct _r_contact {
	str uri;					/**< uri of contact						*/	
	time_t expires;				/**< time of expiration					*/
	str ua;						/**< user agent string					*/
	str path;					/**< path headers (P-CSCF to route to)	*/

	struct _r_contact *next;	/**< the next contact in the list		*/
	struct _r_contact *prev;	/**< the previous contact in the list	*/
} r_contact;

/** Enumeration for Registration States */
enum Reg_States {
	NOT_REGISTERED=0,			/**< User not-registered, no profile stored	*/
	REGISTERED=1,				/**< User registered						*/
	UNREGISTERED=-1				/**< User not-registered, profile stored	*/
} ;

typedef struct _t_regexp_unit {
	//regex_t exp; this is apearently useless
	char *s; // it is null terminated
	struct _t_regexp_unit *next,*prev;
} t_regexp_unit;

typedef struct _t_regexp_list {
	t_regexp_unit *head;
	t_regexp_unit *tail;
} t_regexp_list;


/** registrar public identity structure */
typedef struct _r_public {
	unsigned int hash;			/**< the hash value 						*/
	str aor;					/**< the public identity 					*/
	str early_ims_ip;			/**< IP Address for Early-IMS Auth			*/
	enum Reg_States reg_state;	/**< registration state						*/
	ims_subscription *s;		/**< subscription to which it belongs 		*/
	t_regexp_list *regexp;		/**< regular expresion in case of wild PSI 	*/
	str ccf1,ccf2,ecf1,ecf2;	/**< charging functions						*/

	r_contact *head,*tail;		/**< list of contacts						*/
	r_subscriber *shead,*stail;	/**< list of subscribers attached			*/
	
	struct _r_public *next,*prev; /**< collision hash neighbours			*/
} r_public;


/** S-CSCF registrar hash slot */
typedef struct {
	r_public *head;					/**< first slot in the table			*/
	r_public *tail;					/**< last slot in the table				*/
	gen_lock_t *lock;				/**< slot lock 							*/	
} r_hash_slot;



/** funtion to find a public identity in a registrar */
typedef r_public* (*get_r_public_f)(str aor);
/** function to unlock a registrar slot */
typedef void (*r_unlock_f)(int hash);
/** function to get the expiration of a public identity in a registrar */
typedef int (*get_r_public_expires_f)(str aor);


void r_act_time();
inline int r_valid_contact(r_contact *c);
inline int r_valid_subscriber(r_subscriber *c);

inline unsigned int get_aor_hash(str aor,int hash_size);

int r_storage_init(int hash_size);
void r_storage_destroy();

inline void r_lock(unsigned int hash);
inline void r_unlock(unsigned int hash);



r_subscriber* new_r_subscriber(str subscriber,int event,int expires,dlg_t *dialog);
r_subscriber* get_r_subscriber(r_public *p, str subscriber,int event);
r_subscriber* add_r_subscriber(r_public  *p,str subscriber,int event,int expires,dlg_t *dialog);
r_subscriber* update_r_subscriber(r_public *p,str subscriber,int event,int* expires,dlg_t *dialog);
void del_r_subscriber(r_public *p,r_subscriber *s);
void free_r_subscriber(r_subscriber *s);

r_contact* new_r_contact(str uri,int expires,str ua,str path);
r_contact* get_r_contact(r_public *p, str uri);
r_contact* add_r_contact(r_public *p,str uri,int expires,str ua,str path);
r_contact* update_r_contact(r_public *p,str uri,int *expires, str *ua,str *path);
void del_r_contact(r_public *p,r_contact *c);
void free_r_contact(r_contact *c);

r_public* new_r_public(str aor, enum Reg_States reg_state, ims_subscription *s);
r_public* get_r_public(str aor);
int get_r_public_expires(str aor);
r_public* get_r_public_nolock(str aor);
r_public* get_r_public_previous_lock(str aor,int locked_hash);
r_public* add_r_public(str aor,enum Reg_States reg_state,ims_subscription *s);
r_public* add_r_public_previous_lock(str aor,int locked_hash,enum Reg_States reg_state,ims_subscription *s);
r_public* update_r_public(str aor,enum Reg_States *reg_state,ims_subscription **s,
	str *ccf1, str *ccf2, str *ecf1, str *ecf2);
r_public* update_r_public_previous_lock(str aor,int locked_hash,enum Reg_States *reg_state,ims_subscription **s,
	str *ccf1, str *ccf2, str *ecf1, str *ecf2);	
void r_public_expire(str public_id);
void r_private_expire(str private_id);
void del_r_public(r_public *p);
void free_r_public(r_public *p);

void print_r(int log_level);



#endif //S_CSCF_REGISTRAR_STORAGE_H_
