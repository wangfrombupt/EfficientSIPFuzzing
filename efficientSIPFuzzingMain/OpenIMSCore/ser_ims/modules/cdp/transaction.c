/** 
 * $Id: transaction.c 2 2006-11-14 22:37:20Z vingarzan $
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
 * CDiameterPeer Transaction Procedures
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#include "transaction.h"

#include "timer.h"
#include "globals.h"


cdp_trans_list_t *trans_list=0;		/**< list of transactions */

/**
 * Initializes the transaction structure.
 * Also adds a timer callback for checking the transaction statuses
 * @returns 1 if success or 0 on error
 */
int trans_init()
{
	trans_list = shm_malloc(sizeof(cdp_trans_list_t));
	if (!trans_list){
		LOG_NO_MEM("shm",sizeof(cdp_trans_list_t));
		return 0;
	}
	trans_list->head = 0;
	trans_list->tail = 0;
	trans_list->lock = lock_alloc();
	trans_list->lock = lock_init(trans_list->lock);

	add_timer(1,0,trans_timer,0);
	return 1;
}

/**
 * Create and add a transaction to the transaction list.
 * @param msg - the message that this related to
 * @param cb - callback to be called on response or time-out
 * @param ptr - generic pointer to pass to the callback on call
 * @param timeout - timeout time in seconds
 * @param auto_drop - whether to auto drop the transaction on event, or let the application do it later
 * @returns the created cdp_trans_t* or NULL on error 
 */
inline cdp_trans_t* add_trans(AAAMessage *msg,AAATransactionCallback_f *cb, void *ptr,int timeout,int auto_drop)
{
	cdp_trans_t *x;
	x = shm_malloc(sizeof(cdp_trans_t));
	if (!x) {
		LOG_NO_MEM("shm",sizeof(cdp_trans_t));
		return 0;
	}
	x->ptr = shm_malloc(sizeof(void*));
	if (!x->ptr) {
		LOG_NO_MEM("shm",sizeof(void*));
		shm_free(x);
		return 0;
	}
	x->endtoendid = msg->endtoendId;
	x->hopbyhopid = msg->hopbyhopId;
	x->cb = cb;
	*(x->ptr) = ptr;
	x->expires = timeout + time(0);
	x->auto_drop = auto_drop;
	x->next = 0;

	lock_get(trans_list->lock);
	x->prev = trans_list->tail;
	if (trans_list->tail) trans_list->tail->next = x;
	trans_list->tail = x;
	if (!trans_list->head) trans_list->head = x;
	lock_release(trans_list->lock);
	return x;
}

/**
 * Remove from the list and deallocate a transaction.
 * @param msg - the message that relates to that particular transaction
 */
inline void del_trans(AAAMessage *msg)
{
	cdp_trans_t *x;
	lock_get(trans_list->lock);
	x = trans_list->head;
	while(x&& x->endtoendid!=msg->endtoendId && x->hopbyhopid!=msg->hopbyhopId) x = x->next;
	if (x){
		if (x->prev) x->prev->next = x->next;
		else trans_list->head = x->next;
		if (x->next) x->next->prev = x->prev;
		else trans_list->tail = x->prev;
		free_trans(x);
	}
	lock_release(trans_list->lock);
}

/**
 * Return and remove the transaction from the transaction list.
 * @param msg - the message that this transaction relates to
 * @returns the cdp_trans_t* if found or NULL if not
 */
inline cdp_trans_t* take_trans(AAAMessage *msg)
{
	cdp_trans_t *x;
	lock_get(trans_list->lock);
	x = trans_list->head;
	while(x&& x->endtoendid!=msg->endtoendId && x->hopbyhopid!=msg->hopbyhopId) x = x->next;
	if (x){
		if (x->prev) x->prev->next = x->next;
		else trans_list->head = x->next;
		if (x->next) x->next->prev = x->prev;
		else trans_list->tail = x->prev;
	}
	lock_release(trans_list->lock);
	return x;
}

/**
 * Deallocate the memory taken by a transaction.
 * @param x - the transaction to deallocate
 */
inline void free_trans(cdp_trans_t *x)
{
	if (x->ptr) shm_free(x->ptr);
	shm_free(x);
}

/**
 * Timer callback for checking the transaction status.
 * @param now - time of call
 * @param ptr - generic pointer, passed to the transactional callbacks
 */
void trans_timer(time_t now, void* ptr)
{
	cdp_trans_t *x,*n;	
	LOG(L_DBG,"DBG:trans_timer(): taking care of diameter transactions...\n");
	lock_get(trans_list->lock);
	x = trans_list->head;
	while(x)
	{
		if (now>x->expires){
			x->ans = 0;
			if (x->cb){
				(x->cb)(1,*(x->ptr),0);
			}
			n = x->next;
			
			if (x->prev) x->prev->next = x->next;
			else trans_list->head = x->next;
			if (x->next) x->next->prev = x->prev;
			else trans_list->tail = x->prev;
			if (x->auto_drop) free_trans(x);
			
			x = n;
		} else 
			x = x->next;
	}
	lock_release(trans_list->lock);
}

