/** 
 * $Id: transaction.h 2 2006-11-14 22:37:20Z vingarzan $
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
#ifndef __TRANSACTION_H_
#define __TRANSACTION_H_

#include <time.h>
#include "utils.h"
#include "diameter.h"
#include "diameter_api.h"

/** Diameter Transaction representation */
typedef struct _cdp_trans_t{
	AAAMsgIdentifier endtoendid;	/**< End-to-end id of the messages */
	AAAMsgIdentifier hopbyhopid;	/**< Hop-by-hop id of the messages */
	AAATransactionCallback_f *cb;	/**< transactional callback function */
	void **ptr;						/**< generic pointer to pass to the callback */
	AAAMessage *ans;				/**< answer for the transaction */
	time_t expires;					/**< time of expiration, when a time-out event will happen */
	int auto_drop;					/**< if to drop automatically the transaction on event or to let the app do it later */
	struct _cdp_trans_t *next;		/**< the next transaction in the transaction list */
	struct _cdp_trans_t *prev;		/**< the previous transaction in the transaction list */
} cdp_trans_t;

/** Diameter Transaction list */
typedef struct {		
	gen_lock_t *lock;				/**< lock for list operations */
	cdp_trans_t *head,*tail;		/**< first, last transactions in the list */ 
} cdp_trans_list_t;

int trans_init();

inline cdp_trans_t* add_trans(AAAMessage *msg,AAATransactionCallback_f *cb, void *ptr,int timeout,int auto_drop);
void del_trans(AAAMessage *msg);
inline cdp_trans_t* take_trans(AAAMessage *msg);
inline void free_trans(cdp_trans_t *x);

void trans_timer(time_t now, void* ptr);



#endif /*TRANSACTION_H_*/
