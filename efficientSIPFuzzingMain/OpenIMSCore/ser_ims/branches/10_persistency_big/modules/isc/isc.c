/**
 * $Id: isc.c 490 2007-11-09 00:45:40Z vingarzan $
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
 * IMS Service Control - ISC Interface communication
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "../tm/tm_load.h"
#include "../../dset.h"

#include "isc.h"
#include "mod.h"
#include "mark.h"
#include "sip.h"
#include "../scscf/scscf_load.h"
#include "../../timer.h"
												/**< link to the stateless reply function in sl module	*/
extern struct tm_binds isc_tmb;		/**< Structure with pointers to tm funcs 		*/

extern str isc_my_uri;				/**< Uri of myself to loop the message in str	*/
extern str isc_my_uri_sip;			/**< Uri of myself to loop the message in str with leading "sip:" */
extern int isc_fr_timeout;			/**< default ISC response timeout in ms */
extern int isc_fr_inv_timeout;		/**< default ISC INVITE response timeout in ms */


/**
 *	Forwards the message to the application server.
 * - Marks the message
 * - fills routes
 * - replaces dst_uri
 * @param msg - the SIP message
 * @param m  - the isc_match that matched with info about where to forward it 
 * @param mark  - the isc_mark that should be used to mark the message
 * @returns #ISC_RETURN_TRUE if OK, #ISC_RETURN_ERROR if not
 */
int isc_forward( struct sip_msg *msg, isc_match *m,isc_mark *mark)
{
	struct cell *t;
	unsigned int hash,label;
	ticks_t fr_timeout,fr_inv_timeout;
	DBG( "DEBUG:"M_NAME":isc_forward(): marking for AS <%.*s>\n",
		m->server_name.len, m->server_name.s );

	isc_mark_set(msg,m,mark);
	/* change destination so it forwards to the app server */
	if (msg->dst_uri.s) pkg_free(msg->dst_uri.s);
	msg->dst_uri.s = pkg_malloc(m->server_name.len);
	if (!msg->dst_uri.s) {
		LOG(L_ERR,"ERR:"M_NAME":isc_forward(): error allocating %d bytes\n",m->server_name.len);
		return ISC_RETURN_ERROR;
	}
	msg->dst_uri.len = m->server_name.len;
	memcpy(msg->dst_uri.s,m->server_name.s,m->server_name.len);

	/* append branch if last trigger failed */
	if (*isc_tmb.route_mode == MODE_ONFAILURE) 
		append_branch(msg,msg->first_line.u.request.uri.s,msg->first_line.u.request.uri.len,
			msg->dst_uri.s,msg->dst_uri.len,0,0);
	
	/* set the timeout timers to a lower value */
	cscf_get_transaction(msg,&hash,&label);
	t = isc_tmb.t_gett();
	fr_timeout = t->fr_timeout;
	fr_inv_timeout = t->fr_inv_timeout;
	t->fr_timeout=S_TO_TICKS(isc_fr_timeout)/1000;
	t->fr_inv_timeout=S_TO_TICKS(isc_fr_inv_timeout)/1000;
	
	/* send it */
	isc_tmb.t_relay(msg,0,0);
	
	/* recover the timeouts */
	t->fr_timeout=fr_timeout;
	t->fr_inv_timeout=fr_inv_timeout;
	
	LOG(L_INFO,"INFO:"M_NAME">>       msg was fwded to AS\n");
	
//	LOG(L_INFO,"INFO:"M_NAME":isc_forward:   fw relayed with marking [%8X]\n",rc->mark.cnt);
//	DBG( "DEBUG:"M_NAME":isc_forward: Forward sending finished, resuming operation\n" );
	return ISC_RETURN_TRUE;
}
