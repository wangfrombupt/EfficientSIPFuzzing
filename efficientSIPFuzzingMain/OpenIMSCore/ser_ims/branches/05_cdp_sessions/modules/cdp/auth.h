/**
 * $Id$
 *   
 * Copyright (C) 2004-2007 FhG Fokus
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
#ifndef __DIAMETER_AUTH_SESSION_H
#define __DIAMETER_AUTH_SESSION_H


#include "diameter_api.h"
#include "utils.h"

/** Keep the session state */
#define SESSION_STATE_MAINTAINED      0
/** Don't keep the session state */
#define SESSION_NO_STATE_MAINTAINED   1

/** auth session states */
typedef enum {
	AUTH_ST_IDLE,
	AUTH_ST_PENDING,
	AUTH_ST_OPEN,
	AUTH_ST_DISCON
} auth_state;

/** auth session event */
typedef enum {
	AUTH_EV_START,
	AUTH_EV_SEND_REQ,
	AUTH_EV_RECV_ANS_SUCCESS,
	AUTH_EV_RECV_ANS_UNSUCCESS,
	AUTH_EV_STR,
	AUTH_EV_STA_SUCCESS,
	AUTH_EV_STA_UNSUCCESS
} auth_event;
	
/** structure for auth session */
typedef struct _auth_session {

	auth_state* st;		/** current state */
	
	str* sid; 			/** session id */
	str* fqdn;			/** FQDN of peer */
	str* call_id;       /** Call-ID combines sip dialog with diameter session */
	
	AAAMessage* req;
	AAAMessage* ans;
	
	/* TODO add 3 timers:
	 * 
	 * session timeout
	 * authorization-lifetime
	 * auth-grace-period		
	 */
	void (*sm_process)(struct _auth_session* auth, int ev, AAAMessage* req, AAAMessage* ans);
	
	struct _auth_session* prev;
	struct _auth_session* next;
	
} AAAAuthSession;

/** list of AAAAuthSessions */
typedef struct {
	AAAAuthSession *head;	/**< first AAAAuthSession in the list */	
	AAAAuthSession *tail; 	/**< last AAAAuthSession in the list */
} auth_session_list_t;

//void add_auth_session(AAAAuthSession *p);
//AAAAuthSession get_auth_session();

int auth_session_init();

/** exported modul methods */
AAAAuthSession* AAACreateAuthSession(str peer, str call_id, int session_state);
AAAAuthSession* AAAGetAuthSession(str call_id); 
void AAADropAuthSession(AAAAuthSession* auth);

#endif /*__DIAMTER_AUTH_SESSION_H*/
