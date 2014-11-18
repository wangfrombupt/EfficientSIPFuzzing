/*
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
 * \file release_call.c
 * 
 *	P-CSCF initiated call release (for confirmed dialogs and QoS relevant cases)
 * 
 *  \author Alberto Diez     albertowhiterabbit at yahoo dot es
 * 
 */


#include "release_call.h"

extern struct tm_binds tmb; 
extern dlg_func_t dialogb;	


extern str pcscf_record_route_mo_uri;
extern str pcscf_record_route_mt_uri;



/*only two reasons are possible*/
/*503 service unavailable and 488 not acceptable here*/
/*default is 503 because it fits both the timer and QoS related releases*/
static str reason_hdr_s={"Reason: SIP ;cause=",19};
static str reason_hdr_1={" ;text=\"",8};
static str reason_hdr_e={"\"\r\n",3};

//static int default_code=503;
//static str _503_text_s={"Service Unavailable",28};
static str _488_text_s={"Not Acceptable Here",19};

static str method_CANCEL_s={"CANCEL",6};
static str method_ACK_s={"ACK",3};
static str method_BYE_s={"BYE",3};


void alter_dialog_route_set(dlg_t *,enum p_dialog_direction);
int send_request(str ,str ,dlg_t *,transaction_cb , enum p_dialog_direction);
void confirmed_response(struct cell *,int ,struct tmcb_params *);


static str content_length_s={"Content-Length: 0\r\n",19};
/**
 * This functions sends BYE for a confirmed dialog
 * @param d - the p_dialog to end
 * @param reason - the Reason: header to include in the messages
 * @returns 0 on error 1 on success
 */
int release_call_confirmed(p_dialog *d, int reason_code, str reason_text)
{
	enum p_dialog_direction odir;
	p_dialog *o;
	str r;
	str hdrs={0,0};	
	char buf[256];
	
	LOG(L_INFO,"DBG:"M_NAME":release_call_confirmed(): Releasing call <%.*s> DIR[%d].\n",
		d->call_id.len,d->call_id.s,d->direction);
	
	r.len = snprintf(buf,256,"%.*s%d%.*s%.*s%.*s",
		reason_hdr_s.len,reason_hdr_s.s,
		reason_code,
		reason_hdr_1.len,reason_hdr_1.s,
		reason_text.len,reason_text.s,
		reason_hdr_e.len,reason_hdr_e.s);
	r.s = buf;

	hdrs.len = r.len+content_length_s.len;	
	hdrs.s = pkg_malloc(hdrs.len);
	if (!hdrs.s){
		LOG(L_INFO,"DBG:"M_NAME":release_call_confirmed(): Error allocating %d bytes.\n",hdrs.len);
		hdrs.len=0;
		goto error;
	}
	hdrs.len=0;	
	STR_APPEND(hdrs,r);
	STR_APPEND(hdrs,content_length_s);	
		
	/* get the dialog in the other direction to see if something going on there and mark as in releasing */
	switch (d->direction){
		case DLG_MOBILE_ORIGINATING:
			odir = DLG_MOBILE_TERMINATING;
			break;
		case DLG_MOBILE_TERMINATING:
			odir = DLG_MOBILE_ORIGINATING;
			break;
		default:
			odir = d->direction;
	}	
	
	o = get_p_dialog_dir_nolock(d->call_id,odir);
	if (o && !o->is_releasing) o->is_releasing = 1;
		
	d->is_releasing++;
		
	if (d->is_releasing>MAX_TIMES_TO_TRY_TO_RELEASE){
		LOG(L_ERR,"ERR:"M_NAME":release_call_confirmed(): had to delete silently dialog %.*s in direction %i\n",d->call_id.len,d->call_id.s,d->direction);
		del_p_dialog(d);
		goto error;
	}
	if (d->is_releasing==1) {	
		/*Before generating a request, we have to generate
		 * the route_set in the dlg , because the route set
		 * in the dialog is for the UAC everything which was in the 
		 * Record-Routes (including local address)*/
		alter_dialog_route_set(d->dialog_c,d->direction);		
		
		/*first generate the bye for called user*/
		/*then generate the bye for the calling user*/
		send_request(method_BYE_s,hdrs,d->dialog_c,confirmed_response,d->direction);
		send_request(method_BYE_s,hdrs,d->dialog_s,confirmed_response,d->direction);
		
		/*the dialog is droped by the callback-function when receives the two replies */
	}	 

	if (hdrs.s) pkg_free(hdrs.s);
	return 1;
error:
	if (hdrs.s) pkg_free(hdrs.s);
	return 0;	
}


/**
 * Callback function for BYE requests!
 * Identify the s_dialog, then see if one BYE has already been recieved
 * if yes drop it , if no, wait for the second
 */
void confirmed_response(struct cell *t,int type,struct tmcb_params *ps)
{		
	p_dialog *d;
	unsigned int hash;
	str call_id;
	
	enum p_dialog_direction dir;
	
	
	if (!ps->param) return;
	dir = *((enum p_dialog_direction *) *(ps->param));
	shm_free(*ps->param);
	*ps->param = 0;		
		
	//call_id = cscf_get_call_id(ps->rpl,0);
	call_id = t->callid;
  	call_id.s+=9;
  	call_id.len-=11;
	
	LOG(L_INFO,"DBG:"M_NAME":confirmed_response(): Received a BYE for a call release for <%.*s> DIR[%d].\n",
		call_id.len,call_id.s,dir);
	
	d = get_p_dialog_dir(call_id,dir);
	if (!d)	{
		LOG(L_ERR,"ERR:"M_NAME":confirmed_response(): Received a BYE for a call release but there is no dialog for <%.*s> DIR[%d].\n",
			call_id.len,call_id.s,dir);
		return;
	}
	
	if (ps->code>=200){
		if (d->state==DLG_STATE_TERMINATED_ONE_SIDE){
			hash=d->hash;
			del_p_dialog(d);
			d_unlock(hash);			 
		} else {
			hash=d->hash;
			d->state=DLG_STATE_TERMINATED_ONE_SIDE;
			d_unlock(hash);
		}		
	} 
}



/**
 * Function that releases a call in early or early200 situation
 * early200 is when the callee has already sent out 200 but that hasn't
 * arrived yet to the caller
 * @param d - p_dialog of the call
 * @situation - flag to distinguish between two situations
 * @return 0 on error 1 on success
 * 
 * \note This function shouldn't be called directly!
 * use release_call_early or release_call_early200 instead 
 * 
 * \note This function is full of tricks to fake states and
 * to decieve the transaction module so that it lets us
 * end a call in weird state
 * 
 * \note any move in the order of functions to clarify the structure
 * can lead to a crash in P-CSCF so watch out!
 */
int release_call_previous(p_dialog *d,enum release_call_situation situation,int reason_code,str reason_text)
{
	struct cell* t;
	p_dialog *o;
	enum p_dialog_direction odir;
	int i;
	str r;
	str hdrs={0,0};	
	char buf[256];
	
	LOG(L_INFO,"DBG:"M_NAME":release_call_previous(): Releasing call <%.*s> DIR[%d].\n",
		d->call_id.len,d->call_id.s,d->direction);
	
	r.len = snprintf(buf,256,"%.*s%d%.*s%.*s%.*s",
		reason_hdr_s.len,reason_hdr_s.s,
		reason_code,
		reason_hdr_1.len,reason_hdr_1.s,
		reason_text.len,reason_text.s,
		reason_hdr_e.len,reason_hdr_e.s);
	r.s = buf;

	hdrs.len = r.len+content_length_s.len;	
	hdrs.s = pkg_malloc(hdrs.len);
	if (!hdrs.s){
		LOG(L_INFO,"DBG:"M_NAME":release_call_previous(): Error allocating %d bytes.\n",hdrs.len);
		hdrs.len=0;
		goto error;
	}
	hdrs.len=0;
	STR_APPEND(hdrs,r);
	STR_APPEND(hdrs,content_length_s);	
													
	/* get the dialog in the other direction to see if something going on there and mark as in releasing */
	switch (d->direction){
		case DLG_MOBILE_ORIGINATING:
			odir = DLG_MOBILE_TERMINATING;
			break;
		case DLG_MOBILE_TERMINATING:
			odir = DLG_MOBILE_ORIGINATING;
			break;
		default:
			odir = d->direction;
	}	
	
	o = get_p_dialog_dir_nolock(d->call_id,odir);
	if (o && !o->is_releasing) o->is_releasing = 1;
		
	d->is_releasing++;
		
	if (d->is_releasing>MAX_TIMES_TO_TRY_TO_RELEASE){
		LOG(L_ERR,"ERR:"M_NAME":release_call_previous(): had to delete silently dialog %.*s in direction %i\n",d->call_id.len,d->call_id.s,d->direction);
		del_p_dialog(d);
		goto error;
	}
	
	alter_dialog_route_set(d->dialog_c,d->direction);
	
	d->state=DLG_STATE_TERMINATED_ONE_SIDE;
	/*this is just a trick to use the same callback function*/	
	
	/*trick or treat!*/
	d->dialog_c->state=DLG_CONFIRMED;
	
	if (situation == RELEASE_CALL_WEIRD){
		send_request(method_ACK_s,hdrs,d->dialog_c,0,0);
		send_request(method_BYE_s,hdrs,d->dialog_c,confirmed_response,d->direction);
		//d->dialog_c->state=DLG_EARLY;
	} else {/*(situation == RELEASE_CALL_EARLY)*/		
		send_request(method_CANCEL_s,hdrs,d->dialog_c,confirmed_response,d->direction);
		//d->dialog_c->state=DLG_EARLY;
	}

	/*i need the cell of the invite!!*/
	/*this is very experimental
	 * and very tricky too*/
	t=tmb.t_gett();
	
	if (t && t->uas.request) {
		/*first trick: i really want to get this reply sent even though we are onreply*/
		*tmb.route_mode=MODE_ONFAILURE;
		
		/*second trick .. i haven't recieve any response from the uac
		 * if i don't do this i get a cancel sent to the S-CSCF .. its not a big deal*/
		 /*if i cared about sip forking then probably i would not do that and let the 
		  * CANCEL go to the S-CSCF (reread specifications needed)*/
		for (i=0; i< t->nr_of_outgoings; i++)
			t->uac[i].last_received=99;
		/*t->uas.status=100;*/ /*no one cares about this*/
		/*now its safe to do this*/
		
		tmb.t_reply(t->uas.request,reason_code,reason_text.s);
		*tmb.route_mode=MODE_ONREPLY;
		tmb.t_release(t->uas.request);

		/*needed because if not i get last message retransmited... 
		 * probably there is a more logical way to do this.. but since i really
		 * want this transaction to end .. whats the point?*/
	}
	
	return 1;
error:
	if (hdrs.s) pkg_free(hdrs.s);
	return 0;	
}

/*Send cancel and 503 or 488 */
int release_call_early(p_dialog *d,int reason_code,str reason_text)
{
	/*CANCEL will be badly routed because the response hasn't being processed
	 * it will be sent to I-CSCF who will relay to S-CSCF and from there to term@P-CSCF*/
	return release_call_previous(d,RELEASE_CALL_EARLY,reason_code,reason_text);
}
/*send ACK,BYE and 503 or 488*/
int release_call_early200(p_dialog *d,int reason_code,str reason_text)
{
	return release_call_previous(d,RELEASE_CALL_WEIRD,reason_code,reason_text);
}

/**
 * Releases a dialog either confirmed or early
 * the dialog is given with a lock on the hash!
 * @param d - p_dialog to release
 * @param reason - Reason header to include in messages
 * @returns 0 on error , 1 on success, -1 if dialog should be deleted from outside
 */
 /*
  * Any dialogs in state over CONFIRMED are going to be treated by
  * release_call_confirmed,  its ok , but its a bit confusing because
  * some dialogs which were already treated by release_call_early may 
  * end in release_call_confirmed if there is a second call to release_call_p
  * and they are already in state DLG_STATE_TERMINATED_ONE_SIDE.. it doesn't really
  * matter because both do the same at that point!
  */
int release_call_p(p_dialog *d,int reason_code,str reason_text)
{		
	if (d->state>=DLG_STATE_CONFIRMED)
			return(release_call_confirmed(d,reason_code,reason_text));
	 else  
			return(release_call_early(d,reason_code,reason_text)); 
}


/**
 * Given a call-id, locate if its terminating,orginating or both
 * release the dialog involved and drop the dialog
 * @param callid - the Call-ID to release
 * @param reason - the Reason header to include in messages
 * @returns 0 on error, 1 on success  
 */ 
int release_call(str callid,int reason_code,str reason_text)
{
	p_dialog *d=0;
	unsigned int hash;
	int res = 0;
	
	d = get_p_dialog_dir(callid,DLG_MOBILE_ORIGINATING);
	if (d) {				
		hash = d->hash;
		if (release_call_p(d,reason_code,reason_text)>0) res = 1;		
		goto done;		
	}
	d = get_p_dialog_dir(callid,DLG_MOBILE_TERMINATING);
	if (d) {				
		hash = d->hash;
		if (release_call_p(d,reason_code,reason_text)>0) res = 1;		
		goto done;
	} 
	
	/*Neither ORGINATING nor TERMINATING is UNKNOWN!*/
	/*or doesn't exist*/
	/*drop it silently?*/
	/*treat it as ORIGINATING or TERMINATING?*/				
done:		
	if (d) d_unlock(hash);
	return 0;	
}



/**
 * Releases a call from the on reply route block
 * called with any reply to an INVITE
 * useful in cases of rejecting a call when you are processing the SDP
 * or handling QoS things
 * @msg  - the sip message being processed
 * @str1 - the first parameter  "orig" or "term"
 * @str2 - [optional] the Reason header that you want to go to the messages 
 * @returns - TRUE on success or FALSE on misscall and BREAK on error
 */
int P_release_call_onreply(struct sip_msg *msg,char *str1,char *str2)
{
	enum p_dialog_direction dir;
	p_dialog *d=NULL;
	str callid;
	struct hdr_field *h1;
	str reason={NULL,0};
	
	if (str2) {
		reason.s=str2;
		reason.len=strlen(str2);
	} else
		reason = _488_text_s;
		
	
	dir= (str1[0]=='o' || str1[0]=='O' || str1[0]=='0')? DLG_MOBILE_ORIGINATING : DLG_MOBILE_TERMINATING;
	
	if (msg->first_line.type== SIP_REQUEST)
	{
		LOG(L_ERR,"ERR: P_release_call_on_reply called with a request\n");
		return CSCF_RETURN_FALSE;
	}
	
	callid=cscf_get_call_id(msg,&h1);
	if (is_p_dialog_dir(callid,dir)) {
		d=get_p_dialog_dir(callid,dir);
		if (msg->first_line.u.reply.statuscode > 199)
		{
			release_call_previous(d,RELEASE_CALL_WEIRD,488,reason);
			d_unlock(d->hash);
			return CSCF_RETURN_TRUE;
		} else {
			release_call_previous(d,RELEASE_CALL_EARLY,488,reason);
			d_unlock(d->hash);
			return CSCF_RETURN_TRUE;
		}
	} else {
		LOG(L_ERR,"ERR:"M_NAME "P_release_call_onreply :  unable to find dialog\n");
		return CSCF_RETURN_BREAK;
	}
	
}
/**
 * Sends the request on the dialog with the direction given as a parameter
 * @param method - the SIP method of the request
 * @param reqbuf - Any headers to be included in the request
 * @param d - the dlg_t in which to send the request
 * @cb - the callback function that will handle the replies for this request
 * @dir - the direction of the dialog_p to include it as a parameter in the callback
 * @returns 0 on error or 1 on success
*/
int send_request(str method,str reqbuf,dlg_t *d,transaction_cb cb, enum p_dialog_direction dir)
{
	if((d!=NULL) && (method.s!=NULL))
	{
		enum p_dialog_direction *cbp=NULL;
		
		if (method.len !=3 || memcmp(method.s,"ACK",3)!=0)
		{
			/*In case of ACK i don't want to even send the direction
			 * moreover as the memory is freed in the callback function
			 * and the ACK is never replied it would be a bug if i did*/
			cbp = shm_malloc(sizeof(enum p_dialog_direction));
			if (!cbp){
				LOG(L_ERR,"ERR:"M_NAME":send_request(): error allocating %d bytes\n",sizeof(enum p_dialog_direction));
				return 0;
			}
			*cbp=dir;
		}	
		dialogb.request_inside(&method,&reqbuf,NULL, d,cb,cbp);
		return 1;
	}
	
	return 0;
}



/**
 * Alters the saved dlg_t routes for the dialog by removing
 * the first routes before myself.
 * This is requiered because half of the RRset is useless.
 * @param d - the dialog to modify the Record-Routes
 * @param dir - the direction
 */
void alter_dialog_route_set(dlg_t *d,enum p_dialog_direction dir)
{
	rr_t *r,*r_new;	
	str p; /*this is going to point to the scscf uri*/
		
	
	
	switch (dir) {
		case DLG_MOBILE_ORIGINATING:
			p = pcscf_record_route_mo_uri;
			break;
		case DLG_MOBILE_TERMINATING:
			p = pcscf_record_route_mt_uri;
			break;
		default:
			return;
	}
	//LOG(L_CRIT,"Looking for <%.*s> in\n",p.len,p.s);
	//for(r=d->route_set;r!=NULL;r=r->next) 
	//	LOG(L_CRIT,"<%.*s>\n",r->nameaddr.uri.len,r->nameaddr.uri.s);
		
	for(r=d->route_set;r!=NULL;r=r->next) {
		if (r->nameaddr.uri.len>=p.len && 
			strncasecmp(r->nameaddr.uri.s,p.s,r->nameaddr.uri.len)==0)
			{
				r_new=r->next;
				r->next=NULL;
				shm_free_rr(&d->route_set);
				d->route_set = r_new;
  	            return;	
			}
					
	}	
}		

