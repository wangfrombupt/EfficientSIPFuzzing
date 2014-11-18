/**
 * $Id: config.c 353 2007-06-28 14:27:06Z vingarzan $
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
 
#include "config.h"

/**
 * Create a new dp_config.
 */
inline dp_config *new_dp_config()
{
	dp_config *x;	
	x = shm_malloc(sizeof(dp_config));
	if (!x) {
		LOG_NO_MEM("shm",sizeof(dp_config));
		goto error;
	}	
	memset(x,0,sizeof(dp_config));
	return x;
error:
	LOG(L_ERR,"ERROR:%s(): failed to create new dp_config.\n",__FUNCTION__);	
	return 0;
}

/**
 * Create a new dp_config.
 */
inline routing_realm *new_routing_realm()
{
	routing_realm *x;	
	x = shm_malloc(sizeof(routing_realm));
	if (!x) {
		LOG_NO_MEM("shm",sizeof(routing_realm));
		goto error;
	}	
	memset(x,0,sizeof(routing_realm));
	return x;
error:
	LOG(L_ERR,"ERROR:%s(): failed to create new routing_realm.\n",__FUNCTION__);	
	return 0;
}

/**
 * Create a new dp_config.
 */
inline routing_entry *new_routing_entry()
{
	routing_entry *x;	
	x = shm_malloc(sizeof(routing_entry));
	if (!x) {
		LOG_NO_MEM("shm",sizeof(routing_entry));
		goto error;
	}	
	memset(x,0,sizeof(routing_entry));
	return x;
error:
	LOG(L_ERR,"ERROR:%s(): failed to create new routing_entry.\n",__FUNCTION__);	
	return 0;
}


/** 
 * Free the space claimed by a routing entry
 */
inline void free_routing_entry(routing_entry *re)
{
	if (!re) return;
	if (re->fqdn.s) shm_free(re->fqdn.s);
	shm_free(re);
}

/** 
 * Free the space claimed by a routing realm
 */
inline void free_routing_realm(routing_realm *rr)
{
	routing_entry *re,*ren;
	if (!rr) return;
	if (rr->realm.s) shm_free(rr->realm.s);
	for(re=rr->routes;re;re=ren){
		ren = re->next;
		free_routing_entry(re);
	}
	shm_free(rr);
}




/**
 * Frees the memory held by a dp_config.
 */
inline void free_dp_config(dp_config *x)
{
	int i;
	if (!x) return;
	if (x->fqdn.s) shm_free(x->fqdn.s);
	if (x->identity.s) shm_free(x->identity.s);
	if (x->realm.s) shm_free(x->realm.s);
	if (x->product_name.s) shm_free(x->product_name.s);
	if (x->peers) {
		for(i=0;i<x->peers_cnt;i++){
			if (x->peers[i].fqdn.s) shm_free(x->peers[i].fqdn.s);
			if (x->peers[i].realm.s) shm_free(x->peers[i].realm.s);
		}
		shm_free(x->peers);
	}
	if (x->acceptors) {
		for(i=0;i<x->acceptors_cnt;i++){
			if (x->acceptors[i].bind.s) shm_free(x->acceptors[i].bind.s);
		}		
		shm_free(x->acceptors);
	}
	if (x->applications) shm_free(x->applications);
	
	if (x->r_table) {
		routing_realm *rr,*rrn;
		routing_entry *re,*ren;
		for(rr=x->r_table->realms;rr;rr=rrn){
			rrn = rr->next;
			free_routing_realm(rr);
		}
		for(re=x->r_table->routes;re;re=ren){
			ren = re->next;
			free_routing_entry(re);
		}
		shm_free(x->r_table);
	}	
	shm_free(x);
}

/**
 * Log the dp_config to output, for debug purposes.
 */	
inline void log_dp_config(int level,dp_config *x)
{
	int i;
	LOG(level,"Diameter Peer Config:\n");
	LOG(level,"\tFQDN    : %.*s\n",x->fqdn.len,x->fqdn.s);
	LOG(level,"\tRealm   : %.*s\n",x->realm.len,x->realm.s);
	LOG(level,"\tVendorID: %d\n",x->vendor_id);
	LOG(level,"\tProdName: %.*s\n",x->product_name.len,x->product_name.s);
	LOG(level,"\tAcceptUn: [%c]\n",x->accept_unknown_peers?'X':' ');
	LOG(level,"\tDropUnkn: [%c]\n",x->drop_unknown_peers?'X':' ');
	LOG(level,"\tTc      : %d\n",x->tc);
	LOG(level,"\tWorkers : %d\n",x->workers);
	LOG(level,"\tQueueLen: %d\n",x->queue_length);
	LOG(level,"\tPeers : %d\n",x->peers_cnt);
	for(i=0;i<x->peers_cnt;i++)
		LOG(level,"\t\tFQDN:  %.*s \t Realm: %.*s \t Port: %d\n",
			x->peers[i].fqdn.len,x->peers[i].fqdn.s,
			x->peers[i].realm.len,x->peers[i].realm.s,
			x->peers[i].port);
	LOG(level,"\tAcceptors : %d\n",x->acceptors_cnt);
	for(i=0;i<x->acceptors_cnt;i++)
		LOG(level,"\t\tPort:  %d \t Bind: %.*s \n",
			x->acceptors[i].port,
			x->acceptors[i].bind.len,x->acceptors[i].bind.s);
	LOG(level,"\tApplications : %d\n",x->applications_cnt);
	for(i=0;i<x->applications_cnt;i++)
		LOG(level,"\t\t%s ID:  %d \t Vendor: %d \n",
			(x->applications[i].type==DP_AUTHORIZATION)?"Auth":"Acct",
			x->applications[i].id,
			x->applications[i].vendor);	
	if (x->r_table){
		routing_realm *rr;
		routing_entry *re;
		LOG(level,"\tRouting Table : \n");
		for(rr=x->r_table->realms;rr;rr=rr->next){
			LOG(level,"\t\tRealm: %.*s\n",
				rr->realm.len,rr->realm.s);
			for(re=rr->routes;re;re=re->next)		
				LOG(level,"\t\t\tRoute: [%4d] %.*s\n",
					re->metric,re->fqdn.len,re->fqdn.s);			
		}
		for(re=x->r_table->routes;re;re=re->next)		
			LOG(level,"\t\tDefaultRoute: [%4d] %.*s\n",
				re->metric,re->fqdn.len,re->fqdn.s);			
	}
	
}
