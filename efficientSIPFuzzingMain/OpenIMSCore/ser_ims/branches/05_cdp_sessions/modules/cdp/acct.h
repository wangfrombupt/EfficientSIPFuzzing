#ifndef __DIAMETER_BASE_ACCT_H
#define __DIAMETER_BASE_ACCT_H

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


#include "utils.h"
#include "diameter_api.h"

/* Command code used in the accounting portion of Diameter base protocol. */
#define Code_AC 	271
//#define ACA 	271



/** Accounting states definition */
typedef enum {
	ACC_ST_IDLE			= 0,	/**< Idle */
	ACC_ST_PENDING_S	= 1,	/**< Pending Session */
	ACC_ST_PENDING_E	= 2,	/**< Pending Event */
	ACC_ST_PENDING_B	= 3,	/**< Pending Buffered */
	ACC_ST_OPEN	  		= 4,	/**< Open */
	ACC_ST_PENDING_I	= 5,	/**< Pending Interim */
	ACC_ST_PENDING_L	= 6		/**< PendingL - sent accounting stop */
} acc_state_t;


/** Accounting events definition */
typedef enum {
	ACC_EV_START				= 101,	/**< Client or device "requests access" (SIP session establishment) */
	ACC_EV_EVENT				= 102,	/**< Client or device requests a one-time service (e.g. SIP MESSAGE) */
	ACC_EV_BUFFEREDSTART		= 103,	/**< Records in storage */
	ACC_EV_RCV_SUC_ACA_START	= 104,	/**< Successful accounting start answer received */
	ACC_EV_SNDFAIL				= 105,	/**< Failure to send */
	ACC_EV_RCV_FAILED_ACA_START	= 106,	/**< Failed accounting start answer received */
	ACC_EV_STOP					= 107,	/**< User service terminated */
	ACC_EV_INTERIM				= 108,	/**< Interim interval elapses */
	ACC_EV_RCV_SUC_ACA_INTERIM	= 109,	/**< Successful accounting interim answer received */
	ACC_EV_RCV_FAILED_ACA_INTERIM=110,	/**< Failed accounting interim answer received */
	ACC_EV_RCV_SUC_ACA_EVENT	= 111,	/**< Successful accounting event answer received */
	ACC_EV_RCV_FAILED_ACA_EVENT	= 112,	/**< Failed accounting event answer received */
	ACC_EV_RCV_SUC_ACA_STOP		= 113,	/**< Successful accounting stop answer received */
	ACC_EV_RCV_FAILED_ACA_STOP	= 114,	/**< Failed accounting stop answer received */
} acc_event_t;



/** Structure for accounting sessions */
typedef struct _acc_session {
	unsigned int hash;						/**< hash for the accounting session 			*/
	
	str* sID;								/**< session id */
	str* peer_fqdn;							/**< FQDN of peer */
	str* dlgid;       						/**< application-level identifier, combines application session (e.g. SIP dialog) or event with diameter accounting session */
	
	acc_state_t state;						/**< current state */
	unsigned int acct_record_number; 		/**< number of last accounting record within this session */
	unsigned int aii;	 					/**< expiration of Acct-Interim-Interval (seconds) */
	unsigned int timeout;					/**< session timeout (seconds) */
	
	AAAMessage* interim_acr;				/**< interim ACR to send every aii seconds */
	
	struct _acc_session *next;				/**< next accounting session in this hash slot 		*/
	struct _acc_session *prev;				/**< previous accounting session in this hash slot	*/
} AAAAcctSession;


/** Structure for an accounting session hash slot */
typedef struct {
	AAAAcctSession *head;						/**< first accounting session in this hash slot */
	AAAAcctSession *tail;						/**< last accounting session in this hash slot 	*/
	gen_lock_t *lock;					/**< slot lock 									*/	
} acc_session_hash_slot;



inline unsigned int get_acc_session_hash(str* id);

int acc_sessions_init(int hash_size);

void acc_sessions_destroy();

inline void s_lock(unsigned int hash);
inline void s_unlock(unsigned int hash);



void free_acc_session(AAAAcctSession *s);

/** exported funcs */
AAAAcctSession* AAACreateAcctSession(str* peer, str* dlgid);
AAAAcctSession* AAAGetAcctSession(str *dlgid); 
void AAADropAcctSession(AAAAcctSession* s);
AAAMessage* AAAAcctCliEvent(AAAMessage* acr, str* dlgid, str* peer_fqdn);
AAAMessage* AAAAcctCliStart(AAAMessage* acr, str* dlgid, str* peer_fqdn, AAAAcctSession *s);
AAAMessage* AAAAcctCliStop(AAAMessage* acr, str* peer_fqdn, AAAAcctSession *s);
AAAMessage* AAAAcctCliInterim(AAAMessage* acr, str* peer_fqdn, AAAAcctSession *s);

#endif /*__DIAMETER_BASE_ACCT_H*/
