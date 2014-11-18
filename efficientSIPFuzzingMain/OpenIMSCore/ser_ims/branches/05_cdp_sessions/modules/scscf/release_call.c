/*
 * $Id: release_call.c 220 2007-04-05 19:26:00Z vingarzan $
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
 *	S-CSCF initiated call release (for confirmed dialogs)
 * 
 *  \author Alberto Diez     albertowhiterabbit at yahoo dot es
 * 
 */



#include "release_call.h"
#include "sip.h"

extern struct tm_binds tmb; 
extern dlg_func_t dialogb;	

extern str scscf_record_route_mo_uri;
extern str scscf_record_route_mt_uri;




static str bye_s={"BYE",3};
str bye_headers_s={"Content-length: 0\r\n",19};
/**
 * This function sends a bye in the specified dialog 
 * the callback function and parameter are the specified arguments
 * @param d - the dlg_t to send it on
 * @param cbp the callback function
 * @returns 1 on success or 0 on failure
 */
int send_bye(dlg_t *d,transaction_cb cb,enum s_dialog_direction dir)
{
				
	if(d!=NULL)	{
		enum s_dialog_direction *cbp;
		cbp = shm_malloc(sizeof(enum s_dialog_direction));
		if (!cbp){
			LOG(L_ERR,"ERR:"M_NAME":send_bye(): error allocating %d bytes\n",sizeof(enum s_dialog_direction));
			return 0;
		}		
		*cbp = dir;
		dialogb.request_inside(&bye_s, &bye_headers_s, 0, d,cb,cbp);
		return 1;
	}	
	return 0;
}



/**
 * Callback function for BYE requests!
 * Identify the s_dialog, then see if one BYE has already been recieved
 * if yes drop it , if no, wait for the second
 */
void bye_response(struct cell *t,int type,struct tmcb_params *ps)
{		
	s_dialog *d;
	unsigned int hash;
	str call_id;
	
	enum s_dialog_direction dir;
	
	
	if (!ps->param) return;
	dir = *((enum s_dialog_direction *) *(ps->param));
	shm_free(*ps->param);
	*ps->param = 0;		

//	if (ps->rpl && ps->rpl!=(void*) 0xffffffff) call_id = cscf_get_call_id(ps->rpl,0);
//	else {
		call_id = t->callid;
		call_id.s+=9;
		call_id.len-=11;
//	}

	LOG(L_INFO,"DBG:"M_NAME":bye_response(): Received a %d response to BYE for a call release for <%.*s> DIR[%d].\n",
		ps->code, call_id.len,call_id.s,dir);
	
	d = get_s_dialog_dir(call_id,dir);
	if (!d)	{
		LOG(L_ERR,"ERR:"M_NAME":bye_response(): Received a BYE response for a call release but there is no dialog for <%.*s> DIR[%d].\n",
			call_id.len,call_id.s,dir);
		return;
	}
	
	if (ps->code>=200 && ps->code<=300)	{
		if (d->state==DLG_STATE_TERMINATED_ONE_SIDE){
			hash=d->hash;
			del_s_dialog(d);
			d_unlock(hash);			 
		} else {
			hash=d->hash;
			d->state=DLG_STATE_TERMINATED_ONE_SIDE;
			d_unlock(hash);
		}		
	} 
	else if(ps->code>300) {
		LOG(L_INFO,"INFO:"M_NAME":bye_response(): Received a %d response to BYE for a call release. Dialog is dropped anyway.\n",
			ps->code);
		hash=d->hash;
		del_s_dialog(d);
		d_unlock(hash);
	}	
}



/**
 * Alters the saved dlg_t routes for the dialog by removing
 * the first routes before myself.
 * This is requiered because half of the RRset is useless.
 * @param d - the dialog to modify the Record-Routes
 * @param dir - the direction
 */
void alter_dialog_route_set(dlg_t *d,enum s_dialog_direction dir)
{
	rr_t *r,*r_new;	
	str p; /*this is going to point to the scscf uri*/
			
	switch (dir) {
		case DLG_MOBILE_ORIGINATING:
			p = scscf_record_route_mo_uri;
			break;
		case DLG_MOBILE_TERMINATING:
			p = scscf_record_route_mt_uri;
			break;
		default:
			return;
	}
		
	for(r=d->route_set;r!=NULL;r=r->next) {
		if (r->nameaddr.uri.len>=p.len && 
			strncasecmp(r->nameaddr.uri.s,p.s,r->nameaddr.uri.len)==0) {
			r_new = r->next;
			r->next = NULL;
			shm_free_rr(&d->route_set);
			d->route_set = r_new;				
			return;
		}			
	}	
}


/**
 * Given a call-id, locate if its terminating,orginating or both
 * release the dialog involved and drop the dialog
 * @param callid - the Call-ID to release
 * @returns 0 on error, 1 on success  
 */ 
int release_call(str callid)
{
	s_dialog *d=0;
	unsigned int hash;
	int res = 0;
	
	d = get_s_dialog_dir(callid,DLG_MOBILE_ORIGINATING);
	if (d) {				
		hash = d->hash;
		if (release_call_s(d)>0) res = 1;		
		goto done;		
	}
	d = get_s_dialog_dir(callid,DLG_MOBILE_TERMINATING);
	if (d) {				
		hash = d->hash;
		if (release_call_s(d)>0) res = 1;		
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
 * Given an s_dialog, releases the call.
 * This function is already called with a lock in d
 * after returning d should be unlocked.
 * @param d - pointer to the s_dialog structure
 * @returns -1 if dialog the dialog is not confirmed, 0 on error or 1 on success
 */
int release_call_s(s_dialog *d)
{
	enum s_dialog_direction odir;
	s_dialog *o;

	LOG(L_INFO,"DBG:"M_NAME":release_call_s(): Releasing call <%.*s> DIR[%d].\n",
		d->call_id.len,d->call_id.s,d->direction);
					
	/* As for now, i'm only releasing confirmed dialogs */
	if (d->state < DLG_STATE_CONFIRMED){
		LOG(L_ERR,"ERR:"M_NAME":release_call_s(): Unable to release a non-confirmed dialog\n");		
		return -1;
	}
		
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
	
	o = get_s_dialog_dir_nolock(d->call_id,odir);
	if (o && !o->is_releasing) o->is_releasing = 1;
		
	d->is_releasing++;
		
	if (d->is_releasing>MAX_TIMES_TO_TRY_TO_RELEASE){
		LOG(L_ERR,"ERR:"M_NAME":release_call_s(): had to delete silently dialog %.*s in direction %i\n",d->call_id.len,d->call_id.s,d->direction);
		del_s_dialog(d);
		return 0;
	}
	if (d->is_releasing==1) {	
		/*Before generating a request, we have to generate
		 * the route_set in the dlg , because the route set
		 * in the dialog is for the UAC everything which was in the 
		 * Record-Routes (including local address)*/
		alter_dialog_route_set(d->dialog_c,d->direction);		
		
		/*first generate the bye for called user*/
		/*then generate the bye for the calling user*/
		send_bye(d->dialog_c,bye_response,d->direction);
		send_bye(d->dialog_s,bye_response,d->direction);
		
		/*the dialog is droped by the callback-function when recieves the two replies */
	}	 
	return 1;
}



