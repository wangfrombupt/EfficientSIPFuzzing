/**
 * $Id: api_process.h 80 2007-01-02 15:09:41Z vingarzan $
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

#ifndef __CDP_API_PROCESS_H_
#define __CDP_API_PROCESS_H_

#include "peer.h"
#include "diameter.h"

/**
 * Request types for handler switching.
 */
enum handler_types {
	REQUEST_HANDLER=0, 	/**< the message received is a request */
	RESPONSE_HANDLER=1  /**< the message received is a response */
};

/**
 * Diameter message received handler list element.
 */ 
typedef struct handler_t{
	enum handler_types type;					/**< type of the handler */
	union {
		AAARequestHandler_f *requestHandler;	/**< request callback function */
		AAAResponseHandler_f *responseHandler;  /**< response callback function */
	} handler;									/**< union for handler callback function */
	void *param;								/**< transparent parameter to pass to callback */
	struct handler_t *next;				/**< next handler in the list */
	struct handler_t *prev;				/**< prev handler in the list */
} handler;
		
typedef struct handler_list_t{
	handler *head;				/**< first handler in the list */
	handler *tail;				/**< last handler in the list */
} handler_list;
		
int api_callback(peer *p,AAAMessage *msg,void* ptr);

#endif /*API_PROCESS_H_*/
