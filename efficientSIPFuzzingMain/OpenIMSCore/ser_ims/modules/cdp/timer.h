/**
 * $Id: timer.h 2 2006-11-14 22:37:20Z vingarzan $
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
 * CDiameterPeer Timer Process
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#ifndef __TIMERCDP_H
#define __TIMERCDP_H

#include "worker.h"

/** callback function for timer event */
typedef void (*callback_f)(time_t now,void *ptr);

/** timer element */
typedef struct _timer_cb_t{
	time_t expires;		/**< time of expiration */
	int one_time;		/**< if to trigger the event just one_time and then remove */
	int interval;		/**< original interval that this timer was set to expire in */
	callback_f cb;		/**< callback function to be called on timer expiration */
	void **ptr;			/**< generic parameter to call the callback with		*/
	
	struct _timer_cb_t *next;/**< next timer in the timer list */
	struct _timer_cb_t *prev;/**< previous timer in the timer list */	
} timer_cb_t;

/** timer list */
typedef struct {
	timer_cb_t *head;	/**< first element in the timer list */
	timer_cb_t *tail;	/**< last element in the timer list */
} timer_cb_list_t;

int add_timer(int expires_in,int one_time,callback_f cb,void *ptr);

void timer_cdp_init();

void timer_cdp_destroy();


void timer_process(int returns);



#endif

