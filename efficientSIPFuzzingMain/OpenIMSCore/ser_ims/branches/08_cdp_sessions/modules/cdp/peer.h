/**
 * $Id: peer.h 377 2007-07-05 15:51:24Z vingarzan $
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



#ifndef __PEER_H
#define __PEER_H


#include "utils.h"
#include "config.h"
#include <sys/types.h>


/** Peer states definition */
typedef enum {
	Closed 				= 0,	/**< Not connected */
	Wait_Conn_Ack		= 1,	/**< Connecting - waiting for Ack */
	Wait_I_CEA 			= 2,	/**< Connecting - waiting for Capabilities Exchange Answer */
	Wait_Conn_Ack_Elect	= 3,	/**< Connecting - Acknolegded and going for Election */
	Wait_Returns  		= 4,	/**< Connecting - done */
	R_Open 				= 5,	/**< Connected as receiver */
	I_Open 				= 6,	/**< Connected as initiator */
	Closing 			= 7		/**< Closing the connection */
} peer_state_t;


/** Peer events definition */
typedef enum {
	Start			= 101,	/**< Start connection attempt */
	Stop			= 102,	/**< Stop */
	Timeout			= 103,	/**< Time-out */
	Win_Election	= 104,	/**< Winning the election */
	R_Conn_CER		= 105,	/**< Receiver - Received connection Capabilities Exchange Request */
	I_Rcv_Conn_Ack 	= 106,	/**< Initiator - Received connection Ack */
	I_Rcv_Conn_NAck	= 107,	/**< Initiator - Received connection NAck */
	I_Rcv_CER		= 108,	/**< Initiator - Receiver Capabilities Exchange Request */
	I_Rcv_CEA		= 109,	/**< Initiator - Receiver Capabilities Exchange Answer */
	R_Rcv_CER		= 110,	/**< Receiver - Receiver Capabilities Exchange Request */
	R_Rcv_CEA		= 111,	/**< Receiver - Receiver Capabilities Exchange Answer */
	I_Rcv_Non_CEA	= 112,	/**< Initiator - Received non-Capabilities Exchange Answer */
	I_Rcv_DPR		= 113,	/**< Initiator - Received Disconnect Peer Request */
	I_Rcv_DPA		= 114,	/**< Initiator - Received Disconnect Peer Answer */
	R_Rcv_DPR		= 115,	/**< Receiver - Received Disconnect Peer Request */
	R_Rcv_DPA		= 116,	/**< Receiver - Received Disconnect Peer Answer */
	I_Rcv_DWR		= 117,	/**< Initiator - Received Diameter Watch-dog Request */
	I_Rcv_DWA		= 118,	/**< Initiator - Received Diameter Watch-dog Answer */
	R_Rcv_DWR		= 119,	/**< Receiver - Received Diameter Watch-dog Request */
	R_Rcv_DWA		= 120,	/**< Receiver - Received Diameter Watch-dog Answer */
	Send_Message	= 121,	/**< Send a message */
	I_Rcv_Message	= 122,	/**< Initiator - Received a message */
	R_Rcv_Message	= 123,	/**< Receiver - Received a message */
	I_Peer_Disc		= 124,	/**< Initiator - Peer disconnected */
	R_Peer_Disc		= 125	/**< Receiver - Peer disconnected */
} peer_event_t;

/** Peer data structure */
typedef struct _peer_t{
	str fqdn;				/**< FQDN of the peer */
	str realm;				/**< Realm of the peer */
	int port;				/**< TCP Port of the peer */
	
	app_config *applications;/**< list of supported applications */
	int applications_cnt;	/**< size of list of supporter applications*/
	
	gen_lock_t *lock;		/**< lock for operations with this peer */
	
	peer_state_t state;		/**< state of the peer */
	int I_sock;				/**< socket used as initiator */
	int R_sock;				/**< socket used as receiver */
	
	time_t activity;		/**< timestamp of last activity */
	int is_dynamic;			/**< whether this peer was accepted although it was not initially configured */
	int waitingDWA;			/**< if a Diameter Watch-dog Request was sent out and waiting for an answer */
	
	str send_pipe;			/**< pipe to send out messages */
	struct _peer_t *next;	/**< next peer in the peer list */
	struct _peer_t *prev;	/**< previous peer in the peer list */
} peer;

peer* new_peer(str fqdn,str realm,int port);
void free_peer(peer *x,int locked);

inline void touch_peer(peer *p);

#endif
