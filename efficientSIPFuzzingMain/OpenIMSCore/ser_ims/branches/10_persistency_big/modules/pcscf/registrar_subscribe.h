/*
 * $Id: registrar_subscribe.h 161 2007-03-01 14:06:01Z vingarzan $
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

 
#ifndef P_CSCF_REGISTRAR_SUBSCRIBE_H_
#define P_CSCF_REGISTRAR_SUBSCRIBE_H_

#include "../../sr_module.h"
#include "../../locking.h"
#include "../tm/tm_load.h"

#include "registrar_storage.h"

/** REGINFO constants */
enum {
	IMS_REGINFO_FULL,
	IMS_REGINFO_PARTIAL,
	IMS_REGINFO_ACTIVE,
	IMS_REGINFO_TERMINATED
}IMS_Reginfo_states;

/** Event types for "reg" to generated notifications after */
enum {
	IMS_REGINFO_NONE,					/**< no event - donothing */	
	IMS_REGINFO_SUBSCRIBE,				/**< Initial SUBSCRIBE - just send all data - this should not be treated though */
	IMS_REGINFO_SUBSCRIBE_EXPIRED,		/**< The subscribe has expired */
	
	IMS_REGINFO_CONTACT_REGISTERED,		/**< Registered with REGISTER	*/
	IMS_REGINFO_CONTACT_CREATED,		/**< Registered administratively 	*/
	IMS_REGINFO_CONTACT_REFRESHED, 		/**< The expiration was refreshed	*/
	IMS_REGINFO_CONTACT_SHORTENED, 		/**< The expiration was administratively shortened	*/
	IMS_REGINFO_CONTACT_EXPIRED,		/**< A contact has expired and will be removed	*/
	IMS_REGINFO_CONTACT_DEACTIVATED,	/**< Administratively removed, user should retry */
	IMS_REGINFO_CONTACT_PROBATION,		/**< Administratively removed, user should retry later	*/
	IMS_REGINFO_CONTACT_UNREGISTERED,	/**< User unregistered with Expires 0	*/
	IMS_REGINFO_CONTACT_REJECTED	 	/**< Administratively removed, user should not retry */
}IMS_Registrar_events;

/** reg Subscription Structure */
typedef struct _r_subscription {
	unsigned int hash;
	str req_uri;			/**< public id of the user, same thing for To: 	*/
	int duration;			/**< duration of subscription					*/
	time_t expires;			/**< time of expiration							*/
	char attempts_left;		/**< number of unsuccesful attempts to subscribe*/

	dlg_t *dialog; 
		
	struct _r_subscription *next, *prev;
} r_subscription;

/** Subscription list */
typedef struct {	
	gen_lock_t *lock;		/**< lock fo subscription list operations 	*/
	r_subscription *head;	/**< first subscription in the list			*/
	r_subscription *tail;	/**< last subscription in the list			*/ 
} r_subscription_hash_slot;

void subs_lock(unsigned int hash);
void subs_unlock(unsigned int hash);
unsigned int get_subscription_hash(str uri);
int r_subscription_init();
void r_subscription_destroy();


int P_subscribe(struct sip_msg *rpl, char* str1, char* str2);

int r_subscribe(str uri,int duration);


int r_send_subscribe(r_subscription *s,int duration);

void r_subscribe_response(struct cell *t,int type,struct tmcb_params *ps);

int P_notify(struct sip_msg *msg,char *str1,char *str2);

void subscription_timer(unsigned int ticks, void* param);

r_subscription* new_r_subscription(str req_uri,int duration);
void add_r_subscription(r_subscription *s);
int update_r_subscription(r_subscription *s,int expires);
r_subscription* get_r_subscription(str aor);
int is_r_subscription(str aor);
void del_r_subscription(r_subscription *s);
void del_r_subscription_nolock(r_subscription *s);
void free_r_subscription(r_subscription *s);
void print_subs(int log_level);

/** reg Contact structure */
typedef struct _r_regcontact {
	str id;
	str uri;
	int state;
	int event;
	int expires;
	
	struct _r_regcontact *next;
} r_regcontact;

/** reg Registration structure */
typedef struct _r_registration {
	str id;
	str aor;
	int state;
	
	r_regcontact *contact;
	struct _r_registration *next;
} r_registration;

/** reg Notification structure */
typedef struct {
	int state;
	r_registration *registration;
} r_notification;

int parser_init(char *dtd_filename);
void parser_destroy();

r_notification* r_notification_parse(str xml);
int r_notification_process(r_notification *n,int expires);
void r_notification_print(r_notification *n);
void r_notification_free(r_notification *n);


#endif //P_CSCF_REGISTRAR_SUBSCRIBE_H_
