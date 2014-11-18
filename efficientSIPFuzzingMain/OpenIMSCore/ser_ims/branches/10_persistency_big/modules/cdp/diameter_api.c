/**
 * $Id: diameter_api.c 355 2007-06-28 15:41:19Z vingarzan $
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
 * CDiameterPeer - Diameter API interface
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */



#include "diameter_api.h"
#include "peer.h"
#include "peermanager.h"
#include "receiver.h"
#include "transaction.h"
#include "api_process.h"
#include "routing.h"

				/* TRANSACTIONS */
				
/**
 * Create a AAATransaction for the given request.
 * @param app_id - id of the request's application
 * @param cmd_code - request's code
 * @returns the AAATransaction*
 */				
AAATransaction *AAACreateTransaction(AAAApplicationId app_id,AAACommandCode cmd_code)
{
	AAATransaction *t;
	t = shm_malloc(sizeof(AAATransaction));
	if (!t) return 0;
	memset(t,0,sizeof(AAATransaction));	
	t->application_id=app_id;
	t->command_code=cmd_code;			
	return t;
}

/**
 * Free the memory allocated for the AAATransaction.
 * @param trans - the AAATransaction to be deallocated
 * @returns 1 on success, 0 on failure
 */
int AAADropTransaction(AAATransaction *trans)
{
	if (!trans) return 0;
//	LOG(L_ERR,"\nCALLED HERE %d %d\n",trans->done,trans->with_callback);
	shm_free(trans);
	return 1;
}


				/* CALLBACKS */

extern handler_list *handlers; 		/**< list of handlers */
extern gen_lock_t *handlers_lock;	/**< lock for list of handlers */

/**
 * Add a handler function for incoming requests.
 * @param f - the callback function
 * @param param - generic parameter to be used when calling the callback functions
 * @returns 1 on success, 0 on failure
 */
int AAAAddRequestHandler(AAARequestHandler_f *f,void *param)
{
	handler *h = shm_malloc(sizeof(handler));
	if (!h) {
		LOG(L_ERR,"ERR:AAAAddRequestHandler: error allocating %d bytes in shm\n",
			sizeof(handler));
		return 0;
	}
	h->type = REQUEST_HANDLER;
	h->handler.requestHandler = f;
	h->param = param;
	h->next = 0;
	lock_get(handlers_lock);
	h->prev = handlers->tail;
	if (handlers->tail) handlers->tail->next = h;
	handlers->tail = h;
	if (!handlers->head) handlers->head = h;
	lock_release(handlers_lock);
	return 1;
}

/**
 * Add a handler function for incoming responses.
 * @param f - the callback function
 * @param param - generic parameter to be used when calling the callback functions
 * @returns 1 on success, 0 on failure
 */
int AAAAddResponseHandler(AAAResponseHandler_f *f,void *param)
{
	handler *h = shm_malloc(sizeof(handler));
	if (!h) {
		LOG(L_ERR,"ERR:AAAAddResponseHandler: error allocating %d bytes in shm\n",
			sizeof(handler));
		return 0;
	}
	h->type = RESPONSE_HANDLER;
	h->handler.responseHandler = f;
	h->param = param;
	h->next = 0;
	lock_get(handlers_lock);
	h->prev = handlers->tail;
	if (handlers->tail) handlers->tail->next = h;
	handlers->tail = h;
	if (!handlers->head) handlers->head = h;
	lock_release(handlers_lock);
	return 1;
}


				/* MESSAGE SENDING */

/**
 * Send a AAAMessage asynchronously.
 * When the response is received, the callback_f(callback_param,...) is called.
 * @param message - the request to be sent
 * @param peer_id - FQDN of the peer to send
 * @param callback_f - callback to be called on transactional response or transaction timeout
 * @param callback_param - generic parameter to call the transactional callback function with
 * @returns 1 on success, 0 on failure 
 * \todo remove peer_id and add Realm routing
 */
AAAReturnCode AAASendMessage(	
		AAAMessage *message,
		AAATransactionCallback_f *callback_f,
		void *callback_param)
{
	peer *p;
	p = get_routing_peer(message);
	if (!p) {
		LOG(L_ERR,"ERROR:AAASendMessage(): Can't find a suitable connected peer in the routing table.\n");
		goto error;
	}
	if (p->state!=I_Open && p->state!=R_Open){
		LOG(L_ERR,"ERROR:AAASendMessage(): Peer not connected to %.*s\n",p->fqdn.len,p->fqdn.s);
		goto error;
	}
	/* only add transaction following when required */
	if (callback_f){
		if (is_req(message))
			add_trans(message,callback_f,callback_param,DP_TRANS_TIMEOUT,1);
		else
			LOG(L_ERR,"ERROR:AAASendMessage(): can't add transaction callback for answer.\n");
	}
		
	if (!peer_send_msg(p,message))
		goto error;
		
	return 1;
error:	
	AAAFreeMessage(&message);
	return 0;
}

/**
 * Send a AAAMessage asynchronously.
 * When the response is received, the callback_f(callback_param,...) is called.
 * @param message - the request to be sent
 * @param peer_id - FQDN of the peer to send
 * @param callback_f - callback to be called on transactional response or transaction timeout
 * @param callback_param - generic parameter to call the transactional callback function with
 * @returns 1 on success, 0 on failure 
 * \todo remove peer_id and add Realm routing
 */
AAAReturnCode AAASendMessageToPeer(	
		AAAMessage *message,
		str *peer_id, 
		AAATransactionCallback_f *callback_f,
		void *callback_param)
{
	peer *p;
	p = get_peer_by_fqdn(peer_id);
	if (!p) {
		LOG(L_ERR,"ERROR:AAASendMessageToPeer(): Peer unknown %.*s\n",peer_id->len,peer_id->s);
		goto error;
	}
	if (p->state!=I_Open && p->state!=R_Open){
		LOG(L_ERR,"ERROR:AAASendMessageToPeer(): Peer not connected to %.*s\n",peer_id->len,peer_id->s);
		goto error;
	}
	/* only add transaction following when required */
	if (callback_f){
		if (is_req(message))
			add_trans(message,callback_f,callback_param,DP_TRANS_TIMEOUT,1);
		else
			LOG(L_ERR,"ERROR:AAASendMessageToPeer(): can't add transaction callback for answer.\n");
	}
		
	if (!peer_send_msg(p,message))
		goto error;
		
	return 1;
error:	
	AAAFreeMessage(&message);
	return 0;
}


/**
 * Generic callback used by AAASendRecvMessage() to block until a transactional response
 * is received.
 * The AAASendRecvMessage() is basically a AAASendMessage() that has a callback 
 * (this function) that blocks until a transactional response or timeout is received and 
 * then it returns that.
 *  
 * @param is_timeout - if this is a time-out or response event
 * @param param - generic parameter to call the transactional callback function with
 * @param ans - the answer for the callback
 */
void sendrecv_cb(int is_timeout,void *param,AAAMessage *ans)
{
	lock_release((gen_lock_t*)param);
}

/**
 * Send a AAAMessage synchronously.
 * This blocks until a response is received or a transactional time-out happens. 
 * @param message - the request to be sent
 * @param peer_id - FQDN of the peer to send
 * @returns 1 on success, 0 on failure 
 * \todo remove peer_id and add Realm routing
 * \todo replace the busy-waiting lock in here with one that does not consume CPU
 */
AAAMessage* AAASendRecvMessage(AAAMessage *message)
{
	peer *p;
	gen_lock_t *lock;
	cdp_trans_t *t;
	AAAMessage *ans;
	
	p = get_routing_peer(message);
	if (!p) {
		LOG(L_ERR,"ERROR:AAASendRecvMessage(): Can't find a suitable connected peer in the routing table.\n");
		goto error;
	}
	if (p->state!=I_Open && p->state!=R_Open){
		LOG(L_ERR,"ERROR:AAASendRecvMessage(): Peer not connected to %.*s\n",p->fqdn.len,p->fqdn.s);
		goto error;
	}
	
	
	if (is_req(message)){
		lock = lock_alloc();
		lock = lock_init(lock);
		lock_get(lock);
		t = add_trans(message,sendrecv_cb,(void*)lock,DP_TRANS_TIMEOUT,0);

		if (!peer_send_msg(p,message)) {
			lock_destroy(lock);
			lock_dealloc((void*)lock);	
			goto error;
		}

		/* block until callback is executed */
		lock_get(lock);		
		lock_destroy(lock);
		lock_dealloc((void*)lock);
		ans = t->ans;
		free_trans(t);
		return ans;
	}
	else
	{
		LOG(L_ERR,"ERROR:AAASendRecvMessage(): can't add wait for answer to answer.\n");
		goto error;
	}

		
error:	
	AAAFreeMessage(&message);
	return 0;
}

/**
 * Send a AAAMessage synchronously.
 * This blocks until a response is received or a transactional time-out happens. 
 * @param message - the request to be sent
 * @param peer_id - FQDN of the peer to send
 * @returns 1 on success, 0 on failure 
 * \todo remove peer_id and add Realm routing
 * \todo replace the busy-waiting lock in here with one that does not consume CPU
 */
AAAMessage* AAASendRecvMessageToPeer(AAAMessage *message, str *peer_id)
{
	peer *p;
	gen_lock_t *lock;
	cdp_trans_t *t;
	AAAMessage *ans;
	
	p = get_peer_by_fqdn(peer_id);
	if (!p) {
		LOG(L_ERR,"ERROR:AAASendRecvMessageToPeer(): Peer unknown %.*s\n",peer_id->len,peer_id->s);
		goto error;
	}
	if (p->state!=I_Open && p->state!=R_Open){
		LOG(L_ERR,"ERROR:AAASendRecvMessageToPeer(): Peer not connected to %.*s\n",peer_id->len,peer_id->s);
		goto error;
	}
	
	
	if (is_req(message)){
		lock = lock_alloc();
		lock = lock_init(lock);
		lock_get(lock);
		t = add_trans(message,sendrecv_cb,(void*)lock,DP_TRANS_TIMEOUT,0);

		if (!peer_send_msg(p,message)) {
			lock_destroy(lock);
			lock_dealloc((void*)lock);	
			goto error;
		}

		/* block until callback is executed */
		lock_get(lock);		
		lock_destroy(lock);
		lock_dealloc((void*)lock);
		ans = t->ans;
		free_trans(t);
		return ans;
	}
	else
	{
		LOG(L_ERR,"ERROR:AAASendRecvMessageToPeer(): can't add wait for answer to answer.\n");
		goto error;
	}

		
error:	
	AAAFreeMessage(&message);
	return 0;
}



