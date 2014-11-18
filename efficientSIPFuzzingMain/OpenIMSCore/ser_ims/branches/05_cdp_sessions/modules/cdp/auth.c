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
#include <time.h>
 
#include "diameter_api.h"


// TODO AAAAuthSession should be saved in a hash table
auth_session_list_t* auth_list;

//gen_lock_t* auth_lock;          /**< lock for an auth session */
gen_lock_t* auth_list_lock;		/**< lock for the auth_list */




int auth_session_init()
{
	auth_list = shm_malloc(sizeof(auth_session_list_t));
	auth_list->head = NULL;
	auth_list->tail = NULL;
//	auth_lock = lock_alloc();
//	auth_lock = lock_init(auth_lock);
	auth_list_lock = lock_alloc();
	auth_list_lock = lock_init(auth_list_lock);
	return 1;
}




// TODO  auth_session_destroy()



void add_auth_session(AAAAuthSession* p)
{
	lock_get(auth_list_lock);
	if (!auth_list->head) {
		auth_list->head = p;
		auth_list->tail = p;
	} else { 
		p->prev = auth_list->tail;
		auth_list->tail->next = p;
		auth_list->tail = p;
	}
	lock_release(auth_list_lock);
}



AAAAuthSession* get_auth_session(str call_id)
{
	lock_get(auth_list_lock);
	AAAAuthSession* p = auth_list->head;
	while (p) {
		if (strncmp (p->call_id->s, call_id.s, call_id.len) == 0) {
			lock_release(auth_list_lock);
			return p;
		} else 
			p = p->next;
	}
	lock_release(auth_list_lock);
	return NULL;
}



void remove_auth_session(AAAAuthSession* p)
{
	lock_get(auth_list_lock);
	if ((!p->prev) && (!p->next)) {
		auth_list->head = NULL;
		auth_list->tail = NULL;
	} else if (!p->prev) {
		p->next->prev = NULL;
		auth_list->head = p->next;
	} else if (!p->next) {
		p->prev->next = NULL;
		auth_list->tail = p->prev;
	} else {
		p->prev->next = p->next;
		p->next->prev = p->prev;
	}
	lock_release(auth_list_lock);
}


void AAADropAuthSession(AAAAuthSession* auth)
{
	/*Maybe there is no authlist yet.. so lets create one*/
	if (auth_list==NULL)
			return;
		
	remove_auth_session(auth);
	shm_free(auth->st);
	AAADropSession(auth->sid);
	shm_free(auth->fqdn);
	shm_free(auth->call_id->s);
	shm_free(auth->call_id);
	shm_free(auth);
}



int length_auth_list()
{
	AAAAuthSession* p;
	int count = 0;
	
	lock_get(auth_list_lock);
	p = auth_list->head;
	while(p) {
		count++;
		p = p->next;
	}
	lock_release(auth_list_lock);
	return count;
}



/** stateless client state machine 
 * TODO
 */ 
void auth_sm_process_stateless(AAAAuthSession* auth, int ev, 
							   AAAMessage *req, AAAMessage* ans)
{
}


int get_result_code(AAAMessage* msg)
{
	if (!msg) goto error;
	AAA_AVP* rc = AAAFindMatchingAVP(msg, 0, AVP_Result_Code, 0, 0);
	if (!rc) goto error;
	return get_4bytes(rc->data.s);

error:
	LOG(L_INFO, "INF: no AAAMessage or Result Code not found\n");
	return 0;
}



/**
 * stateful client state machine
 * @param auth - AAAAuthSession which uses this state machine
 * @param ev   - Event
 * @param req  - AAAMessage
 */
void auth_sm_process_stateful(AAAAuthSession* auth, int ev, 
							  AAAMessage* req, AAAMessage* ans)
{
	int rc;
	
	LOG(L_INFO, "in auth_sm_process_stateful\n");
	switch (*auth->st) {
		case AUTH_ST_IDLE:
			LOG(L_INFO, "INF: auth_sm_process_stateful: IDLE\n");
			switch (ev) {
				case AUTH_EV_SEND_REQ:
					LOG(L_INFO, "INF: send AAR\n");
					*auth->st = AUTH_ST_PENDING;
					*ans = *AAASendRecvMessage(req, auth->fqdn);

					rc = get_result_code(ans);
					if (rc == AAA_SUCCESS) {
						LOG(L_INFO, "INF: receive AAA success\n");
						auth_sm_process_stateful(auth, AUTH_EV_RECV_ANS_SUCCESS,
												 NULL, NULL);
					} else {
						LOG(L_INFO, "INF: receive AAA unsuccess\n");
						auth_sm_process_stateful(auth, 
												 AUTH_EV_RECV_ANS_UNSUCCESS,
												 NULL, NULL);
						//AAAPrintMessage(ans);
					}
					break;	
			}
			break;
		
		case AUTH_ST_PENDING:
			LOG(L_INFO, "INF: auth_sm_process_stateful: PENDING\n");
			switch (ev) {
				case AUTH_EV_RECV_ANS_SUCCESS:
					*auth->st = AUTH_ST_OPEN;
					break;
				case AUTH_EV_RECV_ANS_UNSUCCESS:
					*auth->st = AUTH_ST_IDLE;
					LOG(L_INFO, "INF: terminate auth session\n");
					AAADropAuthSession(auth);
					LOG(L_INFO, "active session number: %d\n", length_auth_list());
					break;
			}
			break;
		
		case AUTH_ST_OPEN:
			LOG(L_INFO, "INF: auth_sm_process_stateful: OPEN\n");
			switch (ev) {
				case AUTH_EV_STR:
					LOG(L_INFO, "send STR\n");
					*auth->st = AUTH_ST_DISCON;
					ans = AAASendRecvMessage(req, auth->fqdn);
					if (ans) {
						LOG(L_INFO, "receive STA successful\n");
						auth_sm_process_stateful(auth, AUTH_EV_STA_SUCCESS, 
												 NULL, NULL);
					} else {
						LOG(L_INFO, "receive STA unsuccessful\n");
						auth_sm_process_stateful(auth, AUTH_EV_STA_UNSUCCESS,
												 NULL, NULL);
					}
					break;
			}
			break;
		
		case AUTH_ST_DISCON:
			LOG(L_INFO, "INF: auth_sm_process_stateful: DISCON\n");
			switch(ev) {
				case AUTH_EV_STA_SUCCESS:
					break;
				case AUTH_EV_STA_UNSUCCESS:
					*auth->st = AUTH_ST_IDLE;
					LOG(L_INFO, "INF: terminate auth session\n");
					AAADropAuthSession(auth);
					LOG(L_INFO, "active session number: %d\n", length_auth_list());
					break;
			}	
			break;
	}
}




/****************************** API FUNCTIONS ********************************/
/**
 * Creates an AAAAuthSession.
 * @param peer - PDF address that PCSCF is conntected to.
 * @param state - stateful / stateless 
 * @returns the new AAAAuthSession
 */
AAAAuthSession* AAACreateAuthSession(str peer, str call_id, int state)
{
	AAAAuthSession* auth = NULL;
	/*Maybe there is no authlist yet.. so lets create one*/
	if (auth_list==NULL)
		auth_session_init();
	 LOG(L_INFO,"INF: AAACreateAuthSession\n");
	// first check if the session already exists
	auth = get_auth_session(call_id);
	 
	if(auth!=NULL) return auth;
	// P-CSCF breaks randomly on this line 
	auth = shm_malloc(sizeof(AAAAuthSession));
	 
	auth->st = shm_malloc(sizeof(auth_state));
	*(auth->st) = AUTH_ST_IDLE;
	 
	auth->fqdn = shm_malloc(sizeof(str));
	auth->fqdn->len = peer.len;
	auth->fqdn->s = peer.s;

	auth->call_id = shm_malloc(sizeof(str));
	auth->call_id->len = call_id.len;
	if (call_id.len)
	{
		auth->call_id->s=shm_malloc(call_id.len);
		if (!auth->call_id->s)
		{
			LOG(L_ERR,"ERR: unable to allocate memory\n");
		}	
	}
	strncpy(auth->call_id->s,call_id.s,call_id.len);
	//print(auth->call_id);
	
	auth->sid = shm_malloc(sizeof(str));
	auth->sid->len = 0;
	auth->sid->s = 0;
	/* generates a new session-ID */
	if (generate_sessionID( auth->sid, 0, NULL )!=1) goto error;
	//print(auth->sid);
	
	auth->prev = NULL;
	auth->next = NULL;
	
	if (state == SESSION_STATE_MAINTAINED) {
		auth->sm_process = auth_sm_process_stateful;
	} else {
		auth->sm_process = auth_sm_process_stateless;
	}
		
	add_auth_session(auth);
	
	return auth;

error:
	if (auth!=NULL) 
		LOG(L_ERR, "ERR: AAACreateAuthSession: AAAAuthSession exists\n");
	else 	
		LOG(L_ERR, "ERR: AAACreateAuthSession: Error on new AAAAuthSession \
			generation\n");
	
	return NULL;
}



/**
 * Get an AAAAuthSession based on the Call-ID in a SIP message.
 * @param call_id Call-ID in SIP message
 *  
 */
AAAAuthSession* AAAGetAuthSession(str call_id) 
{
	AAAAuthSession* auth;
	/*Maybe there is no authlist yet.. so lets create one*/
	if (auth_list==NULL)
		auth_session_init();
			LOG(L_INFO, "INF: AAAGetAuthSession\n");
	auth = get_auth_session(call_id);
	if (!auth) LOG(L_ERR, "ERR: AAAGetAuthSession: AAAAuthSession does not exist\n");
	
	return auth;
}
