/** 
 * $Id: worker.h 2 2006-11-14 22:37:20Z vingarzan $
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
 * CDiameterPeer Worker Procedures
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#ifndef __WORKER_H
#define __WORKER_H

#include "peer.h"
#include "diameter.h"
#include "utils.h"

/** function to be called on worker initialization */
typedef int (*worker_init_function)(int rank);

/** task element */ 
typedef struct _task_t {
	peer *p;			/**< peer that the message was received from */
	AAAMessage *msg;	/**< diameter message received */
} task_t;

/** task queue */
typedef struct {
	gen_lock_t *lock;	/**< lock for task queue operations */ 
	int start;			/**< start position in the queue array (index of oldest task) */
	int end;			/**< end position in the queue array (index of the youngest task) */
	int max;			/**< size of the queue array */
	task_t *queue;		/**< array holding the tasks */
	int empty;			/**< id of semaphore for signaling an empty queue */
	int full;			/**< id of semaphore for signaling an full queue */
} task_queue_t;

/** callback function to be called on message processing */
typedef int (*cdp_cb_f)(peer *p,AAAMessage *msg,void* ptr);

/** callback element for message processing */
typedef struct _cdp_cb_t{
	cdp_cb_f cb;				/**< callback function to be called on event */
	void **ptr;					/**< generic pointer to be passed to the callback */
	struct _cdp_cb_t *next; 	/**< next callback in the list */
	struct _cdp_cb_t *prev;		/**< previous callback in the list */
} cdp_cb_t;
	
/** list of callback elements for message processing */
typedef struct {
	cdp_cb_t *head;	/**< first element in the list */
	cdp_cb_t *tail; /**< last element in the list */
} cdp_cb_list_t;

void worker_init();
void worker_destroy();

int cb_add(cdp_cb_f cb,void *ptr);
void cb_remove(cdp_cb_t *cb);

int put_task(peer *p,AAAMessage *msg);
task_t take_task();


void worker_poison_queue();

void worker_process(int id);



#endif

