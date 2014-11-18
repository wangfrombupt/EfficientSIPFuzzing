/**
 * $Id: peer.c 2 2006-11-14 22:37:20Z vingarzan $
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
 * CDiameterPeer Peer related functionality 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include <time.h>

#include "peer.h"
#include "diameter.h"

/**
 * Create a new peer.
 * - All the memory from parameters is duplicated.
 * @param fqdn - FQDN of the peer
 * @param realm - Realm of the peer
 * @param port - port of the peer to connect to
 * @returns the new peer* if ok, NULL on error
 */
peer* new_peer(str fqdn,str realm,int port)
{
	peer *x;
	x = shm_malloc(sizeof(peer));
	if (!x){
		LOG_NO_MEM("shm",sizeof(peer));
		goto error;
	}
	memset(x,0,sizeof(peer));
	shm_str_dup(x->fqdn,fqdn);
	if (!x->fqdn.s) goto error;	
	shm_str_dup(x->realm,realm);
	if (!x->realm.s) goto error;	
	x->port = port;
	x->lock = lock_alloc();
	x->lock = lock_init(x->lock);
		
	x->state = Closed;

	x->I_sock = -1;
	x->R_sock = -1;

	x->activity = time(0)-500;	
	
	x->next = 0;
	x->prev = 0;
	
	return x;
error:
	return 0;
}

/**
 * Frees the memory taken by a peer structure.
 * @param x - the peer to free
 * @param locked - if the caller of this function already acquired the lock on this peer
 */
void free_peer(peer *x,int locked)
{
	if (!x) return;
	if (!locked) lock_get(x->lock);
	if (x->fqdn.s) shm_free(x->fqdn.s);
	if (x->realm.s) shm_free(x->realm.s);	
	lock_destroy(x->lock);
	lock_dealloc((void*)x->lock);
	shm_free(x);
}

/**
 * "Touches" the peer by updating the last activity time to the current time.
 * @param p - which peer to touch
 */
inline void touch_peer(peer *p)
{
	p->activity = time(0);
}
