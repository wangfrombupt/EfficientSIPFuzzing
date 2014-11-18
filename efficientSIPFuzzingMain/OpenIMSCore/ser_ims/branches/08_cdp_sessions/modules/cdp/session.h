/**
 * $Id: session.h 511 2008-01-02 16:12:26Z albertoberlios $
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
 * CDiameterPeer Session Handling 
 * 
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * \author Shengyao Chen shc -at- fokus dot fraunhofer dot de
 * \author Joao Filipe Placido joao-f-placido -at- ptinovacao dot pt
 * 
 */
#ifndef __DIAMETER_SESSION_H
#define __DIAMETER_SESSION_H


//#include "diameter_api.h"
#include "utils.h"


/** Function for callback on session events: timeout, etc. */
typedef void (AAASessionCallback_f)(int event,void *param,void *session);

/** Types of sessions */
typedef enum {
	UNKNOWN_SESSION			= 0x0000,
	
	AUTH_CLIENT_STATELESS	= 0x0100,
	AUTH_SERVER_STATELESS	= 0x0101,
	AUTH_CLIENT_STATEFULL	= 0x0110,
	AUTH_SERVER_STATEFULL	= 0x0111,
	
	ACCT_CLIENT				= 0x1000,
	ACCT_SERVER_STATELESS	= 0x1001,
	ACCT_SERVER_STATEFULL	= 0x1011
		
} cdp_session_type_t;







/** auth session states */
typedef enum {
	AUTH_ST_IDLE,
	AUTH_ST_PENDING,
	AUTH_ST_OPEN,
	AUTH_ST_DISCON
} cdp_auth_state;

/** auth session event */
typedef enum {
	AUTH_EV_START,
	AUTH_EV_SEND_REQ,
	AUTH_EV_SEND_ANS,
	AUTH_EV_SEND_ANS_SUCCESS,
	AUTH_EV_SEND_ANS_UNSUCCESS,
	AUTH_EV_RECV_ASR,
	AUTH_EV_RECV_REQ,
	AUTH_EV_RECV_ANS,
	AUTH_EV_RECV_ANS_SUCCESS,
	AUTH_EV_RECV_ANS_UNSUCCESS,
	AUTH_EV_SEND_ASR,
	AUTH_EV_SEND_ASA_SUCCESS,
	AUTH_EV_SEND_ASA_UNSUCCESS,
	AUTH_EV_SEND_STA,
	AUTH_EV_RECV_ASA,
	AUTH_EV_RECV_ASA_SUCCESS,
	AUTH_EV_RECV_ASA_UNSUCCESS,
	AUTH_EV_RECV_STA,
	AUTH_EV_RECV_STR,
	AUTH_EV_SESSION_TIMEOUT,
	AUTH_EV_SERVICE_TERMINATED,
	AUTH_EV_SESSION_GRACE_TIMEOUT,
} cdp_auth_event;



	
/** structure for auth session */
typedef struct _cdp_auth_session_t {
	cdp_auth_state state;	/**< current state */
	
	time_t timeout;			/**< absolute time for timeout  */
	time_t lifetime;		/**< absolute time for lifetime */
	time_t grace_period;	/**< grace_period in seconds 	*/ 
		
	void* generic_data;			
} cdp_auth_session_t;



/** Accounting states definition */
typedef enum {
	ACC_ST_IDLE			= 0,	/**< Idle */
	ACC_ST_PENDING_S	= 1,	/**< Pending Session */
	ACC_ST_PENDING_E	= 2,	/**< Pending Event */
	ACC_ST_PENDING_B	= 3,	/**< Pending Buffered */
	ACC_ST_OPEN	  		= 4,	/**< Open */
	ACC_ST_PENDING_I	= 5,	/**< Pending Interim */
	ACC_ST_PENDING_L	= 6		/**< PendingL - sent accounting stop */
} cdp_acc_state_t;


/** Accounting events definition */
typedef enum {
	ACC_EV_START					= 101,	/**< Client or device "requests access" (SIP session establishment) */
	ACC_EV_EVENT					= 102,	/**< Client or device requests a one-time service (e.g. SIP MESSAGE) */
	ACC_EV_BUFFEREDSTART			= 103,	/**< Records in storage */
	ACC_EV_RCV_SUC_ACA_START		= 104,	/**< Successful accounting start answer received */
	ACC_EV_SNDFAIL					= 105,	/**< Failure to send */
	ACC_EV_RCV_FAILED_ACA_START		= 106,	/**< Failed accounting start answer received */
	ACC_EV_STOP						= 107,	/**< User service terminated */
	ACC_EV_INTERIM					= 108,	/**< Interim interval elapses */
	ACC_EV_RCV_SUC_ACA_INTERIM		= 109,	/**< Successful accounting interim answer received */
	ACC_EV_RCV_FAILED_ACA_INTERIM	=110,	/**< Failed accounting interim answer received */
	ACC_EV_RCV_SUC_ACA_EVENT		= 111,	/**< Successful accounting event answer received */
	ACC_EV_RCV_FAILED_ACA_EVENT		= 112,	/**< Failed accounting event answer received */
	ACC_EV_RCV_SUC_ACA_STOP			= 113,	/**< Successful accounting stop answer received */
	ACC_EV_RCV_FAILED_ACA_STOP		= 114,	/**< Failed accounting stop answer received */
} cdp_acc_event_t;


/** Structure for accounting sessions */
typedef struct _acc_session {

	cdp_acc_state_t state;						/**< current state */
	
	str dlgid;       						/**< application-level identifier, combines application session (e.g. SIP dialog) or event with diameter accounting session */
	
	unsigned int acct_record_number; 		/**< number of last accounting record within this session */
	time_t aii;	 							/**< expiration of Acct-Interim-Interval (seconds) */
	time_t timeout;							/**< session timeout (seconds) */
	
	
	void* generic_data;			
	
} cdp_acc_session_t;




/** Structure for session identification */
typedef struct _cdp_session_t {
	unsigned int hash;
	str id;                             /**< session-ID as string */
	unsigned int application_id;		/**< specific application id associated with this session */	
	cdp_session_type_t type;
	
	union {
		cdp_auth_session_t auth;
		cdp_acc_session_t acc;
		void *generic_data;
	} u;
	 
	AAASessionCallback_f *cb;			/**< session callback function */
	void *cb_param;						/**< session callback generic parameter */
	 
	struct _cdp_session_t *next,*prev; 	
} cdp_session_t;

/** Session list structure */
typedef struct _cdp_session_list_t {		
	gen_lock_t *lock;				/**< lock for list operations */
	cdp_session_t *head,*tail;		/**< first, last sessions in the list */ 
} cdp_session_list_t;

typedef cdp_session_t AAASession;


int sessions_init(int hash_size);
int sessions_destroy();
void session_timer(time_t now, void* ptr);

cdp_session_t* get_session(str id);

void sessions_lock(unsigned int hash);
void sessions_unlock(unsigned int hash);


#endif
