/**
 * $Id: isc.c 50 2006-12-04 19:20:29Z vingarzan $
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


extern struct tm_binds isc_tmb;		/**< Structure with pointers to tm funcs 		*/

extern str isc_my_uri;				/**< Uri of myself to loop the message in str	*/
extern str isc_my_uri_sip;			/**< Uri of myself to loop the message in str with leading "sip:" */


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
	unsigned int a,b;
	isc_mark *m2;
//	struct cell *t;
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

	/* append branch */
	//TODO - if 2nd fwd fails, just for subsequent fwds, not for first
//	append_branch(msg,msg->first_line.u.request.uri.s,msg->first_line.u.request.uri.len,
//		msg->dst_uri.s,msg->dst_uri.len,0,0);

	/* send it */
	isc_tmb.t_relay(msg,0,0);
//	/* decrease the reply timers to compensate*/
//	t = isc_tmb.t_gett();
//	if (t){
//		t->fr_timeout = 5000;
//		t->fr_inv_timeout = 5000;
//	}
	
	/* register callbacks for failure/response */	
	cscf_get_transaction(msg,&a,&b);
	m2 = isc_mark_dup(mark);
	isc_tmb.register_tmcb(msg,0,TMCB_RESPONSE_IN|TMCB_ON_FAILURE,isc_failure,(void*)m2);	
//	isc_tmb.register_tmcb(msg,0,TMCB_RESPONSE_IN,isc_response,(void*)m2);

	
//	LOG(L_INFO,"INFO:"M_NAME":isc_forward:   fw relayed with marking [%8X]\n",rc->mark.cnt);
//	DBG( "DEBUG:"M_NAME":isc_forward: Forward sending finished, resuming operation\n" );
	return ISC_RETURN_TRUE;
}

/**
 *	 Callback function to deal with failure to contact AS.
 *		- Registered by isc_forward(struct sip_msg*,int status, isc_match *m)
 *		- If the session should continue then the checking is resumed and the
 *		message is forwarded as needed
 *		- If the session should be terminated it send a final response to the client
 * @param t - tm transaction
 * @param type - type of event
 * @param ps - generic parameter for the callback. ps->param should contain the isc_mark 
 */
void isc_failure(struct cell *t,int type,struct tmcb_params *ps)
{
	isc_mark *mark;
	
	if (!(ps->param)){
		LOG( L_INFO,"INFO:"M_NAME":ifc:isc_failure reached but already treated... so skipped.\n");
		return;
	}
	/* Get the running check attached */
	mark = (isc_mark*) *ps->param;
	if (!mark){
		LOG( L_INFO,"INFO:"M_NAME":ifc:isc_failure reached but already treated... so skipped.\n");
		return;
	}
	LOG(L_INFO,"INFO:"M_NAME":isc_failure: reached with s=%d;h=%d;d=%d\n",
		mark->skip,mark->handling,mark->direction);	

	/* non 408 */
	if (ps->code!=408 && ps->code!=480) {
		if (ps->code>=200){
			LOG( L_INFO,"INFO:"M_NAME":isc_failure: reached with reply %d. Aborting as AS responded.\n",ps->code);
			*ps->param=0;
			isc_mark_free(mark);		
		}
		return;
	}

	/* 408 */
	*ps->param=0;
			
	if (mark->handling==IFC_SESSION_TERMINATED) {
		/* Terminate the session */
		DBG( "DEBUG:"M_NAME":isc_failure: Terminating session.\n" );
		//ps->req->dst_uri.len=0;
		isc_mark_free(mark);
		isc_tmb.t_reply(ps->req,IFC_AS_UNAVAILABLE_STATUS_CODE,
			"AS Contacting Failed - iFC terminated dialog");
		LOG(L_INFO,"INFO:"M_NAME":isc_failure: Responding with %d "
			"to URI: %.*s\n",IFC_AS_UNAVAILABLE_STATUS_CODE,
			ps->req->first_line.u.request.uri.len,
			ps->req->first_line.u.request.uri.s);
	}else{
		/* do the bootstrap as REQUEST received exactly the same + mark from AS */
		isc_bootstrap(ps->req,mark);
		isc_mark_free(mark);
		
	}
	DBG("DEBUG:"M_NAME":isc_failure sucesssfuly completed\n" );
}

///**
// *	 Callback function to deal with response from AS
// *		- Registered by ifc_isc_forward(struct sip_msg*,ifc_running_check*)
// *		- The checking is resumed and the message is forwarded as needed
// *		- If the session should be terminated it send a final response to the client
// */
//void isc_response(struct cell *t,int type,struct tmcb_params *ps)
//{
//	isc_mark *mark;
//	if (!(ps->param)){
//		LOG( L_INFO,"INFO:"M_NAME":isc_response: reached but already treated... so skipped.\n");
//		return;
//	}
//	/* Get the running check attached */
//	mark = (isc_mark*) *ps->param;
//
//	LOG(L_CRIT,"INFO:"M_NAME":isc_response: reached with s=%d;h=%d;d=%d\n",
//		mark->skip,mark->handling,mark->direction);	
//
//	if (ps->code<200){
//		/* Informational only */
//		/* Don't destroy the checker but proxy the reply */
//		LOG(L_INFO,"INFO:"M_NAME":"M_NAME":isc_response: - Informational response received\n");
//		DBG( "DEBUG:"M_NAME":isc_response: Proxying response to caller.\n" );
//		/* respond with informational response */
//		isc_tmb.t_relay(ps->rpl,0,0);
//		LOG(L_INFO,"INFO:"M_NAME":isc_response: Informational Response <%d - %.*s> sent\n",
//			ps->rpl->first_line.u.reply.statuscode,
//			ps->rpl->first_line.u.reply.reason.len,
//			ps->rpl->first_line.u.reply.reason.s);
//	}else{		
//		/* Final response */
//		*ps->param=0;
//		isc_mark_free(mark);
//		/* Destroy the checker and proxy the final response */
//		LOG(L_INFO,"INFO:"M_NAME":isc_response: - Final response received\n");
//		/* Proxy the response back */
//		
//		//ifc_mark_set(ps->req,&rc->mark);
//		DBG( "DEBUG:"M_NAME":isc_response: Proxying response to caller.\n" );
//		/* respond with final response */
//		isc_tmb.t_relay(ps->rpl,0,0);
//		LOG(L_INFO,"INFO:"M_NAME":isc_response: Response <%d - %.*s> sent\n",
//			ps->rpl->first_line.u.reply.statuscode,
//			ps->rpl->first_line.u.reply.reason.len,
//			ps->rpl->first_line.u.reply.reason.s);
//	}
//	DBG("DEBUG:"M_NAME":isc_response sucesssfuly completed\n" );
//}


/**
 *	Forwards the message back to the itself.
 *		- Marks the message - as the AS responded but did not change the message
 * 		- replaces dst_uri
 *
 * @param msg - the SIP message
 * @param mark  - the isc_mark that should be used to mark the message
 * @returns #ISC_RETURN_TRUE if OK, #ISC_RETURN_ERROR if not
 */
int isc_bootstrap( struct sip_msg *msg, isc_mark *mark)
{
//	struct cell *t;
	LOG(L_INFO,"DBG:"M_NAME":isc_bootstrap(): marking for bootstrap \n");

	isc_mark_set(msg,0,mark);
	/* change destination so it forwards to the app server */
	if (msg->dst_uri.s) pkg_free(msg->dst_uri.s);
	msg->dst_uri.s = pkg_malloc(isc_my_uri_sip.len);
	if (!msg->dst_uri.s) {
		LOG(L_ERR,"ERR:"M_NAME":isc_forward(): error allocating %d bytes\n",isc_my_uri_sip.len);
		return ISC_RETURN_ERROR;
	}
	msg->dst_uri.len = isc_my_uri_sip.len;
	memcpy(msg->dst_uri.s,isc_my_uri_sip.s,isc_my_uri_sip.len);	

	/* append branch */
	append_branch(msg,msg->first_line.u.request.uri.s,msg->first_line.u.request.uri.len,
		msg->dst_uri.s,msg->dst_uri.len,0,0);

//	/* increase the reply timers to compensate for bootstraping time */
//	t = isc_tmb.t_gett();
//	if (t){
//		t->fr_timeout = 7000;
//		t->fr_inv_timeout = 7000;
//	}
	/* send it */
	isc_tmb.t_relay(msg,0,0);
	
	return ISC_RETURN_TRUE;
}
