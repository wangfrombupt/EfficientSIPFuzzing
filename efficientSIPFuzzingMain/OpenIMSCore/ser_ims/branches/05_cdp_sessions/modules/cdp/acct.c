/**
 * $Id$
 *  
 * Copyright (C) 2004-2007 FhG Fokus
 * Copyright (C) 2007 PT Inovacao
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
 * acct.h, acct.c provides the accounting portion of Diameter based 
 * protocol.
 * 
 * \author Shengyao Chen shc -at- fokus dot fraunhofer dot de
 * \author Joao Filipe Placido joao-f-placido -at- ptinovacao dot pt
 */
#include "acct.h"
#include "acctstatemachine.h"
#include "time.h"

int acc_sessions_hash_size;						/**< size of the accounting session hash table 		*/
acc_session_hash_slot *acc_sessions=0;			/**< the hash table									*/


/**
 * Computes the hash for a string.
 * @param id - the string to compute for
 * @returns the hash % acc_sessions_hash_size
 */
inline unsigned int get_acc_session_hash(str* id)
{
	if (!id) return 0;
	if (!id->s) return 0;
	if (id->len==0) return 0;
#define h_inc h+=v^(v>>3)
	char* p;
	register unsigned v;
	register unsigned h;
  	
	h=0;
	for (p=id->s; p<=((id->s)+(id->len)-4); p+=4){
		v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
		h_inc;
	}
	v=0;
	for (;p<((id->s)+(id->len)); p++) {
		v<<=8;
		v+=*p;
	}
	h_inc;
	
	h=((h)+(h>>11))+((h>>13)+(h>>23));
	return (h)%acc_sessions_hash_size;
#undef h_inc 
}

/**
 * Initialize the accounting sessions hash table.
 * @param hash_size - size of the acc_sessions hash table
 * @returns 1 if OK, 0 on error
 */
int acc_sessions_init(int hash_size)
{
	int i;
	
	acc_sessions_hash_size = hash_size;
	acc_sessions = shm_malloc(sizeof(acc_session_hash_slot)*acc_sessions_hash_size);

	if (!acc_sessions) return 0;

	memset(acc_sessions,0,sizeof(acc_session_hash_slot)*acc_sessions_hash_size);
	
	for(i=0;i<acc_sessions_hash_size;i++){
		acc_sessions[i].lock = lock_alloc();
		if (!acc_sessions[i].lock){
			LOG(L_ERR,"ERR:acc_sessions_init(): Error creating lock\n");
			return 0;
		}
		acc_sessions[i].lock = lock_init(acc_sessions[i].lock);
	}
			
	return 1;
}

/**
 * Destroy the accounting sessions hash table.
 */
void acc_sessions_destroy()
{
	int i;
	AAAAcctSession *s,*ns;
	for(i=0;i<acc_sessions_hash_size;i++){
		s_lock(i);
			s = acc_sessions[i].head;
			while(s){
				ns = s->next;
				free_acc_session(s);
				s = ns;
			}
		s_unlock(i);
		lock_dealloc(acc_sessions[i].lock);
	}
	shm_free(acc_sessions);
}

/**
 * Locks the required slot of the accounting sessions hash table.
 * @param hash - index of the slot to lock
 */
inline void s_lock(unsigned int hash)
{
	LOG(L_CRIT,"GET %d\n",hash);
	lock_get(acc_sessions[(hash)].lock);
	LOG(L_CRIT,"GOT %d\n",hash);	
}

/**
 * UnLocks the required slot of the accounting sessions hash table
 * @param hash - index of the slot to unlock
 */
inline void s_unlock(unsigned int hash)
{
	lock_release(acc_sessions[(hash)].lock);
	LOG(L_CRIT,"RELEASED %d\n",hash);	
}



/**
 * Finds and returns an accounting session from the hash table.
 * \note Locks the hash slot if ok! Call s_unlock(acc_session->hash) when you are finished)
 * @param dlgid - the app-level id (e.g. SIP dialog)
 * @returns the acc_session* or NULL if not found
 */
AAAAcctSession* get_acc_session(str* dlgid)
{
	AAAAcctSession *s=0;
	unsigned int hash = get_acc_session_hash(dlgid);

	s_lock(hash);
		s = acc_sessions[hash].head;
		while(s){
			if (s->dlgid->len == dlgid->len &&
				strncasecmp(s->dlgid->s,dlgid->s,dlgid->len)==0) {
					return s;
				}
			s = s->next;
		}
	s_unlock(hash);
	return 0;
}





/**
 * Creates a new AAAAcctSession structure.
 * Does not add the structure to the list
 * @param dlgid - application-level id
 * @returns the new AAAAcctSession* or NULL
 */
AAAAcctSession* new_acc_session(str* dlgid)
{
	AAAAcctSession *s;
	
	s = shm_malloc(sizeof(AAAAcctSession));
	if (!s) {
		LOG(L_ERR,"ERR:new_acc_session(): Unable to alloc %d bytes\n",
			sizeof(AAAAcctSession));
		goto error;
	}
	memset(s,0,sizeof(AAAAcctSession));
	
	s->hash = get_acc_session_hash(dlgid);
	
	return s;
error:
	if (s){
		shm_free(s);		
	}
	return 0;
}


/**
 * Creates and adds an accounting session to the hash table.
 * \note Locks the hash slot if OK! Call s_unlock(acc_session->hash) when you are finished)
 * @param s - AAAAcctSession to add
 * @returns the new AAAAcctSession* or NULL
 */
AAAAcctSession* add_acc_session(AAAAcctSession* s)
{
	s_lock(s->hash);
		s->next = 0;
		s->prev = acc_sessions[s->hash].tail;
		if (s->prev) s->prev->next = s;
		acc_sessions[s->hash].tail = s;
		if (!acc_sessions[s->hash].head) acc_sessions[s->hash].head = s;

		return s;
}


/**
 * Deletes an acc_session from the hash table
 * \note Must be called with a lock on the acc_session hash slot
 * @param s - the AAAAcctSession to delete
 */
void del_acc_session(AAAAcctSession *s)
{
	LOG(L_INFO,"DBG:del_acc_session(): Deleting AAAAcctSession <%.*s>\n",s->sID->len, s->sID->s);
	if (s->prev) s->prev->next = s->next;
	else acc_sessions[s->hash].head = s->next;
	if (s->next) s->next->prev = s->prev;
	else acc_sessions[s->hash].tail = s->prev;
	free_acc_session(s);
}

/**
 * Frees an accounting session.
 * @param s - the accounting session to free
 */
void free_acc_session(AAAAcctSession *s)
{
	LOG(L_DBG,"DBG:free_acc_session()");
	if (!s) return;
	if (s->sID->s) shm_free(s->sID->s);
	
	// TODO: free interim_acr, peer_fqdn, sID, dlgid, and peer_fqdn.s, dlgid.s
	// TODO: remove interim timer...
	
	shm_free(s);
}




/****************************** API FUNCTIONS ********************************/
/**
 * Creates an AAAAcctSession.
 * @param peer - accounting server FQDN.
 * @param dlgid - application-level id of accountable event or session
 * @returns the new AAAAcctSession*
 */
AAAAcctSession* AAACreateAcctSession(str* peer, str* dlgid)
{
	AAAAcctSession* s = NULL;
	
	LOG(L_INFO, "INF: AAACreateAcctSession\n");
		
	s = new_acc_session(dlgid);
	if (!s) return 0;		
	 
	//s = get_acc_session(dlgid);
	//if(s) goto error;
	
	s->state = ACC_ST_IDLE;

	s->peer_fqdn = shm_malloc(sizeof(str));
	s->peer_fqdn->len = peer->len;
	s->peer_fqdn->s = peer->s;

	s->dlgid = shm_malloc(sizeof(str));
	s->dlgid->len = dlgid->len;
	s->dlgid->s = dlgid->s;
	LOG(L_INFO, "s->dlgid: %.*s\n", s->dlgid->len, s->dlgid->s);
	
	s->sID = shm_malloc(sizeof(str));
	s->sID->len = 0;
	s->sID->s = 0;
	
	s->timeout = time(0) + 60; // TODO: use parameter "session_timeout"
	s->aii=0;
	
	/* generates a new session-ID */
	//if (generate_sessionID( s->sID, 0 )!=1) goto error;
	if (generate_sessionID( s->sID, 0, dlgid )!=1) goto error; // TODO: check pad length...
	LOG(L_INFO, "s->sID: %.*s\n", s->sID->len, s->sID->s);
	
	LOG(L_INFO, "s->hash: %d\n", s->hash);
	
	s->prev = NULL;
	s->next = NULL;
	
	// TODO: include pointer to sm_process function in acc_session
	//		to allow accounting client SM and server SM?
	/*
	if () {
		s->sm_process = acc_cli_sm_process;
	} else {
		s->sm_process = acc_serv_sm_process;
	}*/
		
	add_acc_session(s);
	s_unlock(s->hash);
	
	return s;

error:
	if (s) 
		LOG(L_ERR, "ERR: AAACreateAccSession: AAAAcctSession exists\n");
	else 	
		LOG(L_ERR, "ERR: AAACreateAccSession: Error on new AAAAcctSession \
			generation\n");
	
	return NULL;
}


/**
 * Get an AAAAcctSession based on the application-level id.
 * @param dlgid app-level id 
 * @returns the AAAAcctSession* if found, else NULL
 */
AAAAcctSession* AAAGetAcctSession(str *dlgid) 
{
	AAAAcctSession* s;
	
	LOG(L_INFO, "INF: AAAGetAcctSession\n");
	s = get_acc_session(dlgid);
	if (!s) {
		LOG(L_ERR, "ERR: AAAGetAcctSession: AAAAcctSession does not exist\n");
		return 0;
	}
	unsigned int hash = get_acc_session_hash(dlgid);
	s_unlock(hash);
	
	return s;
}

/**
 * Drop a given AAAAcctSession.
 * @param s the accounting session to drop
 */
void AAADropAcctSession(AAAAcctSession* s) {
	LOG(L_INFO, "INF: AAADropAcctSession\n");
	if (s) {
		AAAAcctSession *s1 = get_acc_session(s->dlgid);
		if (!s1) {
			LOG(L_ERR, "ERR: AAADropAcctSession: AAAAcctSession does not exist\n");
			return;
		}
		unsigned int hash = get_acc_session_hash(s1->dlgid);
		//del_acc_session(s);
		s_unlock(hash);
	}
}



/**
 * Accounting client: sends an ACR Event and returns answer
 * @param acr ACR with EVENT_RECORD Accounting-Record-Type
 * @param dlgid app-level session/dialog id
 * @param peer_fqdn FQDN of diameter peer to send message to
 * @return answer (ACA) or NULL if sending failed
 */
AAAMessage* AAAAcctCliEvent(AAAMessage* acr, str* dlgid, str* peer_fqdn) {
	// TODO: check acr has Accounting-Record-Type with EVENT_RECORD value
	
	AAAMessage* aca = 0;
	AAA_AVP      *avp;
	
	AAAAcctSession* s = AAACreateAcctSession(peer_fqdn, dlgid); 	
	if (!s) return 0;
	
	// add Session-Id AVP
	avp = AAACreateAVP( 263, 0, 0, s->sID->s, s->sID->len,
			AVP_DUPLICATE_DATA);
	if ( !avp || AAAAddAVPToMessage(acr,avp,0)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERROR:AAAAcctCliEvent: cannot create/add Session-Id avp\n");
		if (avp) AAAFreeAVP( &avp );
		goto error;
	}
	acr->sessionId = avp;
	
	acct_cli_sm_process(s, ACC_EV_EVENT, acr, aca, 0);
	
	AAADropAcctSession(s); // TODO: only drop if received successfull answer, otherwise need to buffer and follow Accounting client state machine...
	
	return aca;
error:
	return 0;
}

/**
 * Accounting client sends an ACR Start and returns answer
 * @param acr ACR with START_RECORD Accounting-Record-Type
 * @param dlgid id of app-level session
 * @param peer_fqdn FQDN of diameter peer
 * @param s pointer to newly created session
 * @return answer (ACA) or NULL if sending failed
 */
AAAMessage* AAAAcctCliStart(AAAMessage* acr, str* dlgid, str* peer_fqdn, AAAAcctSession *s) {
	// TODO: check acr has Accounting-Record-Type with START_RECORD value
	AAAMessage* aca = 0;
	AAA_AVP      *avp;
	
	s = AAACreateAcctSession(peer_fqdn, dlgid);
	
	// add Session-Id AVP
	avp = AAACreateAVP( 263, 0, 0, s->sID->s, s->sID->len,
			AVP_DUPLICATE_DATA);
	if ( !avp || AAAAddAVPToMessage(acr,avp,0)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERROR:AAAAcctCliEvent: cannot create/add Session-Id avp\n");
		if (avp) AAAFreeAVP( &avp );
		goto error;
	}
	acr->sessionId = avp;
	
	acct_cli_sm_process(s, ACC_EV_START, acr, aca, 0);
	
	return aca;
error:
	return 0;
}


/**
 * Accounting client sends an ACR Interim, updates session timeout and returns answer
 * @param acr ACR with INTERIM_RECORD Accounting-Record-Type
 * @param peer_fqdn FQDN of diameter peer
 * @param s pointer to existing session
 * @return answer (ACA) or NULL if sending failed
 */
AAAMessage* AAAAcctCliInterim(AAAMessage* acr, str* peer_fqdn, AAAAcctSession *s) {
	// TODO: check acr has Accounting-Record-Type with INTERIM_RECORD value
	AAAMessage* aca = 0;
	AAA_AVP      *avp;
	
	// add Session-Id AVP
	avp = AAACreateAVP( 263, 0, 0, s->sID->s, s->sID->len,
			AVP_DUPLICATE_DATA);
	if ( !avp || AAAAddAVPToMessage(acr,avp,0)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERROR:AAAAcctCliEvent: cannot create/add Session-Id avp\n");
		if (avp) AAAFreeAVP( &avp );
		goto error;
	}
	acr->sessionId = avp;
	
//	s->timeout += ; // TODO: update session timeout
	
	acct_cli_sm_process(s, ACC_EV_INTERIM, acr, aca, 0);
	
	return aca;
error:
	return 0;
}


/**
 * Accounting client sends an ACR Stop and returns answer
 * @param acr ACR with STOP_RECORD Accounting-Record-Type
 * @param peer_fqdn FQDN of diameter peer
 * @param s pointer to existing session
 * @return answer (ACA) or NULL if sending failed
 */
AAAMessage* AAAAcctCliStop(AAAMessage* acr, str* peer_fqdn, AAAAcctSession *s) {
	// TODO: check acr has Accounting-Record-Type with STOP_RECORD value
	AAAMessage* aca = 0;
	AAA_AVP      *avp;
	
	// add Session-Id AVP
	avp = AAACreateAVP( 263, 0, 0, s->sID->s, s->sID->len,
			AVP_DUPLICATE_DATA);
	if ( !avp || AAAAddAVPToMessage(acr,avp,0)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERROR:AAAAcctCliEvent: cannot create/add Session-Id avp\n");
		if (avp) AAAFreeAVP( &avp );
		goto error;
	}
	acr->sessionId = avp;
	
	acct_cli_sm_process(s, ACC_EV_STOP, acr, aca, 0);
	
	return aca;
error:
	return 0;
}

/**
 * The interim interval timer sends an interim ACR
 * @param ticks - the current time
 * @param param - pointer to the AAAAcctSession
*/
void interim_interval_timer(time_t now, void *ptr) {
	// TODO
}


/**
 * The acc_session timer looks for expired sessions and removes them
 * @param ticks - the current time
 * @param param - pointer to the sessions hashtable
 */
void acc_session_timer(time_t ticks, void* param)
{
	int i=0;
	AAAAcctSession *s = 0;
	
	LOG(L_DBG,"DEBUG:acc_session_timer: running acc_session_timer...\n");
	
	for(i=0;i<acc_sessions_hash_size;i++){
		s_lock(i);
		s = acc_sessions[i].head;
		while(s){
			// check acc session expiration
			if (s->timeout < ticks) {
				LOG(L_DBG,"DEBUG:acc_session_timer: dropping session with dlgid %.*s\n", s->dlgid->len, s->dlgid->s);
				AAADropAcctSession(s);
			}
			s = s->next;
		}
		s_unlock(i);
	}
}


//static inline int acct_add_avp(AAAMessage *m,char *d,int len,int avp_code,
//	int flags,int vendorid,int data_do,const char *func)
//{
//	AAA_AVP *avp;
//	if (vendorid!=0) flags |= AAA_AVP_FLAG_VENDOR_SPECIFIC;
//	avp = cdpb.AAACreateAVP(avp_code,flags,vendorid,d,len,data_do);
//	if (!avp) {
//		//LOG(L_ERR,"ERR:"M_NAME":%s: Failed creating avp\n",func);
//		return 0;
//	}
//	if (cdpb.AAAAddAVPToMessage(m,avp,m->avpList.tail)!=AAA_ERR_SUCCESS) {
//		//LOG(L_ERR,"ERR:"M_NAME":%s: Failed adding avp to message\n",func);
//		cdpb.AAAFreeAVP(&avp);
//		return 0;
//	}
//	return 1;
//}
//
///* add Accounting-Record-Type AVP */
//int acct_add_accounting_record_type(AAAMessage* msg, unsigned int data)
//{
//	char x[4];
//	set_4bytes(x,data);
//	
//	return
//	acct_add_avp(msg, x, 4,
//		AVP_Accounting_Record_Type,
//		AAA_AVP_FLAG_MANDATORY,
//		0,
//		AVP_DUPLICATE_DATA,
//		__FUNCTION__);
//}
//
///* add Accounting-Record-Number AVP */
//int acct_add_accounting_record_number(AAAMessage* msg, unsigned int data)
//{
//	char x[4];
//	set_4bytes(x,data);
//	
//	return
//	acct_add_avp(msg, x, 4,
//		AVP_Accounting_Record_Number,
//		AAA_AVP_FLAG_MANDATORY,
//		0,
//		AVP_DUPLICATE_DATA,
//		__FUNCTION__);
//}

/**
 * Create an ACR based on sessionId and accounting type. 
 * 
 * @param type 	- accounting type: start, interim, stop, event  
 * @returns the created ACR message, if error return NULL
 */
//AAAMessage* ACR_create(AAASessionId sessId, unsigned int type, struct cdp_binds* cdpb)
//{
//	AAAMessage* acr = NULL;
//	
//	AAASessionId sid = {0,0};
//	/*
//	 * if type == event 
//	 */
//	if (type == AAA_ACCT_EVENT) {
//	 	sessId = cdpb->AAACreateSession();
//		//acr = cdpb.AAACreateRequest(IMS_Rf, ACR, Flag_Proxyable, &sessId);
//		/* orgin host realm, dest host realm are added by peer */ 
//		//if (!acct_add_accounting_record_type(acr, type)) goto error;
//		//if (!acct_add_accounting_record_number(acr, 0)) goto error;
//	 }
//	 return acr;
//
//	/* if type == start, create a new accounting session. */
//	/* if type == interim, sid cannot be NULL, new ACR is created based
//	 * 		      on this session.
//	 * if type == stop, sid cannot be NULL, new ACR is created based on 
//	 * 			  this session, and the session is closed. 
//	 */
//	
//	
//
//	// TODO not forget to call this line: cdpb.AAADropSession(&sessId);
//error: return NULL; 	
//}





