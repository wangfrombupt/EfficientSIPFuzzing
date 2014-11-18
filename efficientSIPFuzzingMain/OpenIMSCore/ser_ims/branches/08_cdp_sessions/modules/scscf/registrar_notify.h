/*
 * $Id: registrar_notify.h 378 2007-07-05 15:56:44Z vingarzan $
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
 
#ifndef S_CSCF_REGISTRAR_NOTIFY_H_
#define S_CSCF_REGISTRAR_NOTIFY_H_

#include "../../sr_module.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "ims_pm.h"


#define MSG_REG_SUBSCRIBE_OK "Subscription to REG saved"
#define MSG_REG_UNSUBSCRIBE_OK "Subscription to REG dropped"


/** Event types for "reg" to generated notifications after */
enum {
	IMS_REGISTRAR_NONE,					/**< no event - donothing 							*/	
	IMS_REGISTRAR_SUBSCRIBE,			/**< Initial SUBSCRIBE - just send all data - this should not be treated though */
	IMS_REGISTRAR_SUBSCRIBE_EXPIRED,	/**< The subscribe has expired 						*/
	
	IMS_REGISTRAR_CONTACT_REGISTERED, 	/**< Registered with REGISTER						*/
	IMS_REGISTRAR_CONTACT_CREATED,	 	/**< Registered administratively 					*/
	IMS_REGISTRAR_CONTACT_REFRESHED, 	/**< The expiration was refreshed					*/
	IMS_REGISTRAR_CONTACT_SHORTENED, 	/**< The expiration was administratively shortened	*/
	IMS_REGISTRAR_CONTACT_EXPIRED,	 	/**< A contact has expired and will be removed		*/
	IMS_REGISTRAR_CONTACT_DEACTIVATED,	/**< Administratively removed, user should retry 	*/
	IMS_REGISTRAR_CONTACT_PROBATION,	/**< Administratively removed, user should retry later	*/
	IMS_REGISTRAR_CONTACT_UNREGISTERED,	/**< User unregistered with Expires 0				*/
	IMS_REGISTRAR_CONTACT_REJECTED	 	/**< Administratively removed, user should not retry */
}IMS_Registrar_events;

/** reg event notification structure */
typedef struct _r_notification {	
	str req_uri;						/**< Request-URI to send to			*/
	str uri;							/**< URI of the destination			*/
	str subscription_state;				/**< Subscription-state header value*/
	str event;							/**< reg event						*/
		
	str content_type;					/**< content type					*/
	str content;						/**< content						*/
	
	dlg_t *dialog;						/**< dialog to send on				*/
	
	#ifdef WITH_IMS_PM
		unsigned short is_scscf_dereg;		/**< if this is a notification for S-CSCF de-registration */
	#endif
	
	struct _r_notification *next;		/**< next notification in the list	*/
	struct _r_notification *prev;		/**< previous notification in the list	*/
} r_notification;

/** Notification List Structure */
typedef struct {	
	gen_lock_t *lock;					/**< lock for notifications ops		*/
	r_notification *head;				/**< first notification in the list	*/
	r_notification *tail;				/**< last notification in the list	*/
} r_notification_list;


int r_notify_init();
void r_notify_destroy();


int S_can_subscribe(struct sip_msg *msg,char *str1,char *str2);

int S_subscribe(struct sip_msg *msg,char *str1,char *str2);

int S_SUBSCRIBE_reply(struct sip_msg *msg, int code,  char *text,int *expires,str *contact);

int S_event_reg(void *p,void *c,void *s,int event_type,int send_now);


void send_notification(r_notification *n);

void notification_timer(unsigned int ticks, void* param);

r_notification* new_r_notification(str req_uri,str uri,str subscription_state,str event,
					str content_type,str content,dlg_t *dialog,int version);
void add_r_notification(r_notification *n);
void free_r_notification(r_notification *n);


#endif //S_CSCF_REGISTRAR_NOTIFY_H_
