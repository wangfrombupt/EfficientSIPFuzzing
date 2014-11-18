/**
 * $Id: api_process.c 80 2007-01-02 15:09:41Z vingarzan $
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
 * CDiameterPeer API processor
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 #include "api_process.h"
 #include "transaction.h"
 #include "receiver.h"
 


handler_list *handlers = 0; /**< list of handlers */
gen_lock_t *handlers_lock;	/**< lock for list of handlers */

/**
 * This callback is added as an internal message listener and used to process
 * transaction requests. 
 * - first it calls all the registered handlers for requests and responses
 * - then it calls the transactio handler
 * @param p - peer that this message came from
 * @param msg - the diameter message
 * @param ptr - not used anymore
 * @returns 1 always
 */ 
int api_callback(peer *p,AAAMessage *msg,void* ptr)
{
	cdp_trans_t *t;
	int auto_drop;	
	handler *h;
	enum handler_types type;
	AAAMessage *rsp;
	if (is_req(msg)) type = REQUEST_HANDLER;
	else type=RESPONSE_HANDLER;

	lock_get(handlers_lock);
		for(h=handlers->head;h;h=h->next){
			if (h->type==type){
				if (h->type == REQUEST_HANDLER) {					
					rsp = (h->handler.requestHandler)(msg,h->param);
					if (rsp) peer_send_msg(p,rsp);
				}
				else (h->handler.responseHandler)(msg,h->param);
			}
		}		
	lock_release(handlers_lock);
	
	if (!is_req(msg)){		
		/* take care of transactional callback if any */
		t = take_trans(msg);
		if (t){
			t->ans = msg;
			auto_drop = t->auto_drop;
			if (t->cb){
				(t->cb)(0,*(t->ptr),msg);
			}
			if (auto_drop) free_trans(t);
		}
	}
	return 1;
}


