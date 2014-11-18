/**
 * $Id: config.h 522 2008-02-12 20:20:42Z vingarzan $
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
 * CDiameterPeer - start-up configuration
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
 
#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "utils.h"

/** Peer configuration. */
typedef struct{
	str fqdn;	/**< FQDN of the peer */
	str realm;	/**< Realm of the peer */
	int port;	/**< TCP port of the peer; the Diameter uri is then aaa://fqdn:port. */
} peer_config;


/** Acceptor socket configuration. */
typedef struct{
	int port;	/**< TCP port number to listen on */ 
	str bind;	/**< IP address to bind to (if null, then :: (0.0.0.0) - all) */
} acceptor_config;

typedef enum {
	DP_AUTHORIZATION,	/**< Authorization application */
	DP_ACCOUNTING		/**< Accounting application */
} app_type;

/** Application configuration. */
typedef struct {
	int id;			/**< integer id of the appication */
	int vendor;		/**< vendor id of the application */
	app_type type;			/**< type of the application */
} app_config;

/** Routing Table Entry */
typedef struct _routing_entry {
	str fqdn;				/**< FQDN of the server 				*/
	int metric;				/**< The metric of the route			*/
	struct _routing_entry *next;
} routing_entry;

/** Routing Table realm */
typedef struct _routing_realm {
	str realm;				/**< the realm to identify				*/
	routing_entry *routes;	/**< ordered list of routes				*/
	struct _routing_realm *next; /**< the next realm in the table	*/
} routing_realm;

/** Routing Table configuration */
typedef struct {
	routing_realm *realms;	/**< list of realms				 	*/
	routing_entry *routes;	/**< ordered list of default routes 	*/
} routing_table;

/** Full Diameter Peer configuration. */
typedef struct {
	str fqdn;					/**< own FQDN */
	str realm;					/**< own Realm */
	str identity;				/**< own diameter URI */
	int vendor_id;				/**< own vendorid */
	str product_name;			/**< own product name */
	int accept_unknown_peers;	/**< if to accept connections from peers that are not configured initially */
	int drop_unknown_peers;		/**< if to drop the peers that are not initially configured on disconnected;
									 usually, you want to do this, unless you want your list of peers to
									 grow and you want to try and connect back to everybody that connected 
									 to you before */
	int tc;						/**< Tc timer duration (30 seconds should be) */
	int workers;				/**< Number of worker-processes to fork */
	int queue_length;			/**< Length of the message queue; when it is filled, the server part will
									 block until workers will finish work on at least one item in the queue */
	
	peer_config *peers;			/**< list of peers */
	int peers_cnt;				/**< size of the list of peers */
	
	acceptor_config *acceptors;	/**< list of acceptors */
	int acceptors_cnt;			/**< size of the list of acceptors */
	
	app_config *applications;	/**< list of supporter applications */
	int applications_cnt;		/**< size of list of supporter applications*/
	
	routing_table *r_table;		/**< realm routing table */
} dp_config;


dp_config *new_dp_config();
routing_realm *new_routing_realm();
routing_entry *new_routing_entry();
void free_dp_config(dp_config *x);
void free_routing_realm(routing_realm *rr);
void free_routing_entry(routing_entry *re);
inline void log_dp_config(int level,dp_config *x);

dp_config* parse_dp_config(char* filename);

#endif /*__CONFIG_H_*/
