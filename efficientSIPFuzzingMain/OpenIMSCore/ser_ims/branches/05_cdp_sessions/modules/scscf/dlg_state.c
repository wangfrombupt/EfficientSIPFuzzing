/*
 * $Id: dlg_state.c 235 2007-04-18 11:28:10Z vingarzan $
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
 * Serving-CSCF - Dialog State Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *  \author Alberto Diez - Changes to handle release_call
 * 	
 */
 
#include <time.h>

#include "dlg_state.h"
#include "../tm/tm_load.h"
#include "../../mem/shm_mem.h"
#include "../../parser/parse_rr.h"

#include "sip.h"
#include "release_call.h"


extern struct tm_binds tmb;

int s_dialogs_hash_size;						/**< size of the dialog hash table 					*/
s_dialog_hash_slot *s_dialogs=0;				/**< the hash table									*/
extern int scscf_dialogs_expiration_time;		/**< default expiration time for dialogs			*/

extern str scscf_name_str;
extern str scscf_record_route_mo;				/**< the record route header for Mobile Originating */
extern str scscf_record_route_mt;				/**< the record route header for Mobile Terminating */
extern str scscf_record_route_mo_uri;			/**< just the record route uri for Mobile Originating */
extern str scscf_record_route_mt_uri;			/**< just the record route uri for Mobile Terminating */

time_t d_time_now;								/**< dialogs current time							*/




/**
 * Computes the hash for a string.
 * @param call_id - the string to compute for
 * @returns the hash % scscf_dialogs_hash_size
 */
inline unsigned int get_s_dialog_hash(str call_id)
{
	if (call_id.len==0) return 0;
#define h_inc h+=v^(v>>3)
	char* p;
	register unsigned v;
	register unsigned h;
  	
	h=0;
	for (p=call_id.s; p<=(call_id.s+call_id.len-4); p+=4){
		v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
		h_inc;
	}
	v=0;
	for (;p<(call_id.s+call_id.len); p++) {
		v<<=8;
		v+=*p;
	}
	h_inc;
	
	h=((h)+(h>>11))+((h>>13)+(h>>23));
	return (h)%s_dialogs_hash_size;
#undef h_inc 
}

/**
 * Initialize the S-CSCF dialogs registrar.
 * @param hash_size - size of the dialog hash table
 * @returns 1 if OK, 0 on error
 */
int s_dialogs_init(int hash_size)
{
	int i;
	
	s_dialogs_hash_size = hash_size;
	s_dialogs = shm_malloc(sizeof(s_dialog_hash_slot)*s_dialogs_hash_size);

	if (!s_dialogs) return 0;

	memset(s_dialogs,0,sizeof(s_dialog_hash_slot)*s_dialogs_hash_size);
	
	for(i=0;i<s_dialogs_hash_size;i++){
		s_dialogs[i].lock = lock_alloc();
		if (!s_dialogs[i].lock){
			LOG(L_ERR,"ERR:"M_NAME":d_hash_table_init(): Error creating lock\n");
			return 0;
		}
		s_dialogs[i].lock = lock_init(s_dialogs[i].lock);
	}
			
	return 1;
}

/**
 * Destroy the registrar
 */
void s_dialogs_destroy()
{
	int i;
	s_dialog *d,*nd;
	for(i=0;i<s_dialogs_hash_size;i++){
		d_lock(i);
			d = s_dialogs[i].head;
			while(d){
				nd = d->next;
				free_s_dialog(d);
				d = nd;
			}
		d_unlock(i);
		lock_dealloc(s_dialogs[i].lock);
	}
	shm_free(s_dialogs);
}

/**
 * Locks the required slot of the dialog hash table.
 * @param hash - index of the slot to lock
 */
inline void d_lock(unsigned int hash)
{
//	LOG(L_CRIT,"GET %d\n",hash);
	lock_get(s_dialogs[(hash)].lock);
//	LOG(L_CRIT,"GOT %d\n",hash);	
}

/**
 * UnLocks the required slot of the dialog hash table
 * @param hash - index of the slot to unlock
 */
inline void d_unlock(unsigned int hash)
{
	lock_release(s_dialogs[(hash)].lock);
//	LOG(L_CRIT,"RELEASED %d\n",hash);	
}

/**
 * Refresh the current dialog time.
 * @returns the current time
 */
inline int d_act_time()
{
	d_time_now=time(0);
	return d_time_now;
}


/**
 * Creates a new s_dialog structure.
 * Does not add the structure to the list
 * @param call_id - call_id of the dialog
 * @param aor - aor of the user
 * @param dir - the direction
 * @returns the new s_dialog* or NULL s_dialog
 */
s_dialog* new_s_dialog(str call_id,str aor, enum s_dialog_direction dir)
{
	s_dialog *d;
	
	d = shm_malloc(sizeof(s_dialog));
	if (!d) {
		LOG(L_ERR,"ERR:"M_NAME":new_s_dialog(): Unable to alloc %d bytes\n",
			sizeof(s_dialog));
		goto error;
	}
	memset(d,0,sizeof(s_dialog));
	
	d->hash = get_s_dialog_hash(call_id);		
	STR_SHM_DUP(d->call_id,call_id,"shm");
	STR_SHM_DUP(d->aor,aor,"shm");	
	d->direction = dir;
	d->is_releasing = 0;
	return d;
error:
	if (d){
		shm_free(d);		
	}
	return 0;
}

/**
 * Creates and adds a dialog to the hash table.
 * \note Locks the hash slot if OK! Call d_unlock(s_dialog->hash) when you are finished)
 * \note make sure that is_s_dialog(call_id) returns 0 or there will be unreachable duplicates!
 * @param call_id - call_id of the dialog
 * @param aor - aor of the user
 * @param dir - the direction
 * @returns the new s_dialog* or NULL s_dialog
 */
s_dialog* add_s_dialog(str call_id,str aor,enum s_dialog_direction dir)
{
	s_dialog *d;
	
	d = new_s_dialog(call_id,aor,dir);
	if (!d) return 0;		
	
	d_lock(d->hash);
		d->next = 0;
		d->prev = s_dialogs[d->hash].tail;
		if (d->prev) d->prev->next = d;
		s_dialogs[d->hash].tail = d;
		if (!s_dialogs[d->hash].head) s_dialogs[d->hash].head = d;

		return d;
}

/**
 * Finds out if a dialog is in the hash table.
 * @param call_id - call_id of the dialog
 * @param aor - aor of the user
 * @returns 1 if found, 0 if not found
 */
int is_s_dialog(str call_id,str aor)
{
	s_dialog *d=0;
	unsigned int hash = get_s_dialog_hash(call_id);

	d_lock(hash);
		d = s_dialogs[hash].head;
		while(d){
				if (d->aor.len == aor.len &&
				d->call_id.len == call_id.len &&
				strncasecmp(d->aor.s,aor.s,aor.len)==0 &&
				strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
					d_unlock(hash);
					return 1;
				}
			d = d->next;
		}
	d_unlock(hash);
	return 0;
}

/**
 * Finds out if a dialog is in the hash table.
 * @param call_id - call_id of the dialog
 * @param dir - the direction
 * @returns 1 if found, 0 if not found
 */
int is_s_dialog_dir(str call_id,enum s_dialog_direction dir)
{
	s_dialog *d=0;
	unsigned int hash = get_s_dialog_hash(call_id);

	d_lock(hash);
		d = s_dialogs[hash].head;
		while(d){
				if (d->direction == dir &&
				d->call_id.len == call_id.len &&
				strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
					d_unlock(hash);
					return 1;
				}
			d = d->next;
		}
	d_unlock(hash);
	return 0;
}

/**
 * Finds and returns a dialog from the hash table.
 * \note Locks the hash slot if ok! Call d_unlock(s_dialog->hash) when you are finished)
 * @param call_id - call_id of the dialog
 * @param aor - aor of the user
 * @returns the s_dialog* or NULL if not found
 */
s_dialog* get_s_dialog(str call_id,str aor)
{
	s_dialog *d=0;
	unsigned int hash = get_s_dialog_hash(call_id);

	d_lock(hash);
		d = s_dialogs[hash].head;
		while(d){
			if (d->aor.len == aor.len &&
				d->call_id.len == call_id.len &&
				strncasecmp(d->aor.s,aor.s,aor.len)==0 &&
				strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
					return d;
				}
			d = d->next;
		}
	d_unlock(hash);
	return 0;
}

/**
 * Finds and returns a dialog from the hash table.
 * \note Locks the hash slot if ok! Call d_unlock(s_dialog->hash) when you are finished)
 * @param call_id - call_id of the dialog
 * @param dir - the direction
 * @returns the s_dialog* or NULL if not found
 */
s_dialog* get_s_dialog_dir(str call_id,enum s_dialog_direction dir)
{
	s_dialog *d=0;
	unsigned int hash = get_s_dialog_hash(call_id);

	d_lock(hash);
		d = s_dialogs[hash].head;
		while(d){
			if (d->direction == dir &&
				d->call_id.len == call_id.len &&
				strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
					return d;
				}
			d = d->next;
		}
	d_unlock(hash);
	return 0;
}

/**
 * Finds and returns a dialog from the hash table.
 * \note the table should be locked already for the call_id in the parameter
 * @param call_id - call_id of the dialog
 * @param dir - the direction
 * @returns the s_dialog* or NULL if not found
 */
s_dialog* get_s_dialog_dir_nolock(str call_id,enum s_dialog_direction dir)
{
	s_dialog *d=0;
	unsigned int hash = get_s_dialog_hash(call_id);

	d = s_dialogs[hash].head;
	while(d){
		if (d->direction == dir &&
			d->call_id.len == call_id.len &&
			strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
				return d;
			}
		d = d->next;
	}
	return 0;
}

/** 
 * Terminates a dialog - called before del_s_dialog to send out terminatination messages.
 * @param d - the dialog to terminate
 * @returns - 1 if the requests were sent and the dialog will be deleted, 0 on error (you will have to delete the
 * dialog yourself!) 
 */
int terminate_s_dialog(s_dialog *d)
{
	switch (d->method){
		case DLG_METHOD_INVITE:
			if (release_call_s(d)==-1){
				//dialog has expired and not confirmed
				del_s_dialog(d);
			}
			return 1;
			break;
		case DLG_METHOD_SUBSCRIBE:
			LOG(L_ERR,"ERR:"M_NAME":terminate_s_dialog(): Not implemented yet for SUBSCRIBE dialogs!\n");
			return 0;
			break;
		default:
			LOG(L_ERR,"ERR:"M_NAME":terminate_s_dialog(): Not implemented yet for method[%d]!\n",d->method);
			return 0;
	}
}

/**
 * Deletes a dialog from the hash table
 * \note Must be called with a lock on the dialogs slot
 * @param d - the dialog to delete
 */
void del_s_dialog(s_dialog *d)
{
	LOG(L_INFO,"DBG:"M_NAME":del_s_dialog(): Deleting dialog <%.*s> DIR[%d]\n",d->call_id.len,d->call_id.s,d->direction);
	if (d->prev) d->prev->next = d->next;
	else s_dialogs[d->hash].head = d->next;
	if (d->next) d->next->prev = d->prev;
	else s_dialogs[d->hash].tail = d->prev;
	free_s_dialog(d);
}


/**
 * Frees a dialog.
 * @param d - the dialog to delete
 */
void free_s_dialog(s_dialog *d)
{
	if (!d) return;
	if (d->call_id.s) shm_free(d->call_id.s);
	if (d->aor.s) shm_free(d->aor.s);	
	if (d->method_str.s) shm_free(d->method_str.s);
	if (d->dialog_s) tmb.free_dlg(d->dialog_s);
	if (d->dialog_c) tmb.free_dlg(d->dialog_c);		
	shm_free(d);
}

/**
 * Prints the list of dialogs.
 * @param log_level - level to log at
 */
void print_s_dialogs(int log_level)
{
	s_dialog *d;
	int i;
	d_act_time();
	LOG(log_level,"INF:"M_NAME":----------  S-CSCF Dialog List begin --------------\n");
	for(i=0;i<s_dialogs_hash_size;i++){
		d_lock(i);
			d = s_dialogs[i].head;
			while(d){
				LOG(log_level,"INF:"M_NAME":[%4d] Call-ID:<%.*s>\t DIR[%d] AOR:<%.*s>\tMet:[%d]\tState:[%d] Exp:[%4d]\n",i,				
					d->call_id.len,d->call_id.s,d->direction,
					d->aor.len,d->aor.s,
					d->method,d->state,
					(int)(d->expires - d_time_now));
					
				d = d->next;
			} 		
		d_unlock(i);
	}
	LOG(log_level,"INF:"M_NAME":----------  S-CSCF Dialog List end   --------------\n");	
}



/**
 * Returns the p_dialog_direction from the direction string.
 * @param direction - "orig" or "term"
 * @returns the p_dialog_direction if ok or #DLG_MOBILE_UNKNOWN if not found
 */
static inline enum s_dialog_direction get_dialog_direction(char *direction)
{
	switch(direction[0]){
		case 'o':
		case 'O':
		case '0':
			return DLG_MOBILE_ORIGINATING;
		case 't':
		case 'T':
		case '1':
			return DLG_MOBILE_TERMINATING;
		default:
			LOG(L_CRIT,"ERR:"M_NAME":get_dialog_direction(): Unknown direction %s",direction);
			return DLG_MOBILE_UNKNOWN;
	}
}
/**
 * Finds the AOR for a dialog
 * @param msg - the SIP message to add to
 * @param d - the dialog direction
 * @param aor - aor to fill
 * @returns 1 if found, 0 if not
 */
static inline int find_dialog_aor(struct sip_msg *msg,enum s_dialog_direction d,str *aor)
{
	if (msg->first_line.type!=SIP_REQUEST)
		return 0;
	switch(d){
		case DLG_MOBILE_ORIGINATING:
			*aor = cscf_get_asserted_identity(msg); 
			if (!aor->len) return 0;
			return 1;
			break;
		case DLG_MOBILE_TERMINATING:
			*aor = cscf_get_called_party_id(msg,0);	
			if (!aor->len) *aor = cscf_get_identity_from_ruri(msg);
			if (!aor->len) return 0;
			return 1;
			break;
		default:
			LOG(L_CRIT,"ERR:"M_NAME":find_dialog_aor(): Unknown direction %d",d);
			return 0;
	}
	return 1;
}
/**
 * Find the dialog direction from the first Route header.
 * @param msg - the SIP message
 */
static inline enum s_dialog_direction find_dialog_route_dir(struct sip_msg *msg)
{
	str r;	
	r = cscf_get_first_route(msg,0);
	
//	LOG(L_ERR,"DBG:"M_NAME":find_dialog_route_dir(): Route <%.*s>\n",r.len,r.s);
//		scscf_record_route_mo_uri.len,scscf_record_route_mo_uri.s,
//		scscf_record_route_mt_uri.len,scscf_record_route_mt_uri.s);
	
	if (!r.len) return DLG_MOBILE_UNKNOWN;
	
	if (r.len >= scscf_record_route_mo_uri.len &&
		strncasecmp(r.s,scscf_record_route_mo_uri.s,scscf_record_route_mo_uri.len)==0){
		return DLG_MOBILE_ORIGINATING;
	}
	if (r.len >= scscf_record_route_mt_uri.len &&
		strncasecmp(r.s,scscf_record_route_mt_uri.s,scscf_record_route_mt_uri.len)==0){
		return DLG_MOBILE_TERMINATING;
	}
	return DLG_MOBILE_UNKNOWN;
}

/**
 * Find out if a message is within a saved dialog.
 * @param msg - the SIP message
 * @param str1 - the direction of the dialog ("orig"/"term")
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if in, #CSCF_RETURN_FALSE else or #CSCF_RETURN_BREAK on error
 */
int S_is_in_dialog(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id;
	enum s_dialog_direction dir = get_dialog_direction(str1);
	enum s_dialog_direction dirmsg = find_dialog_route_dir(msg);
	

//	LOG(L_CRIT,"%d - %d\n",dir,dirmsg);
	if (dir!=dirmsg) return CSCF_RETURN_FALSE;				
			
//	print_s_dialogs(L_ERR);
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len){
		
		return CSCF_RETURN_FALSE;
	}
	
	if (is_s_dialog_dir(call_id,dir))
		return CSCF_RETURN_TRUE;
	else 
		return CSCF_RETURN_FALSE;
}


str s_INVITE={"INVITE",6};
str s_SUBSCRIBE={"SUBSCRIBE",9};
/**
 * Return s_dialog_method for a method string.
 * @param method - the string containing the method
 * @returns the s_dialog_method corresponding if known or #DLG_METHOD_OTHER if not
 */
static enum s_dialog_method get_dialog_method(str method)
{
	if (method.len == s_INVITE.len &&
		strncasecmp(method.s,s_INVITE.s,s_INVITE.len)==0) return DLG_METHOD_INVITE;
	if (method.len == s_SUBSCRIBE.len &&
		strncasecmp(method.s,s_SUBSCRIBE.s,s_SUBSCRIBE.len)==0) return DLG_METHOD_SUBSCRIBE;
	return DLG_METHOD_OTHER;
}
	


/**
 * Saves a dialog.
 * @param msg - the initial request
 * @param str1 - direction - "orig" or "term"
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 */
int S_save_dialog(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id;
	s_dialog *d;
	str aor;
	str uri,tag;
	str ruri;	
	enum s_dialog_direction dir = get_dialog_direction(str1);
	
	if (!find_dialog_aor(msg,dir,&aor)){
		LOG(L_ERR,"ERR:"M_NAME":S_save_dialog(): Error retrieving %s contact\n",str1);
		return CSCF_RETURN_BREAK;
	}		
		
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;

	LOG(L_INFO,"DBG:"M_NAME":S_save_dialog(%s): Call-ID <%.*s>\n",str1,call_id.len,call_id.s);

	if (is_s_dialog(call_id,aor)){
		LOG(L_ERR,"ERR:"M_NAME":S_save_dialog: dialog already exists!\n");	
		return CSCF_RETURN_FALSE;
	}
	
	d = add_s_dialog(call_id,aor,dir);
	if (!d) return CSCF_RETURN_FALSE;

	d->method = get_dialog_method(msg->first_line.u.request.method);
	STR_SHM_DUP(d->method_str,msg->first_line.u.request.method,"shm");
	d->first_cseq = cscf_get_cseq(msg,0);
	d->last_cseq = d->first_cseq;
	d->state = DLG_STATE_INITIAL;
	d->expires = d_act_time()+60;
	
	cscf_get_from_tag(msg,&tag);
	cscf_get_from_uri(msg,&uri);
	ruri=cscf_get_identity_from_ruri(msg);
	 
	tmb.new_dlg_uac(&call_id,
						&tag,
						d->first_cseq,&uri,
						&ruri,
						&d->dialog_c);
	
	tmb.new_dlg_uas(msg,99,&d->dialog_s);

	d_unlock(d->hash);
	
//	print_s_dialogs(L_INFO);
	
	return CSCF_RETURN_TRUE;	
}


/**
 * Updates a dialog.
 * If the initial request was:
 * - INVITE - refreshes the expiration or looks for the BYE and destroys the dialog 
 * if found
 * - SUBSCRIBE - looks for the Subscription-state in NOTIFY, refreshes the expiration 
 * and if terminated destroys the dialog
 * - When adding more dialogs, add the refreshal methods here or they will expire and will
 * be destroyed. Also add the termination to reduce the memory consumption and improve the
 * performance.
 * @param msg - the request/response
 * @param str1 - direction - "orig" or "term"
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 */
int S_update_dialog(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id;
	s_dialog *d;
	int response;
	int cseq;
	struct hdr_field *h=0;
	struct sip_msg *req=0;
	int expires=0;
	str totag;

	enum s_dialog_direction dir = get_dialog_direction(str1);
		
//	if (!find_dialog_aor(msg,str1,&aor)){
//		req = cscf_get_request_from_reply(msg);		
//		if (!find_dialog_aor(req,str1,&aor)){
//			LOG(L_ERR,"ERR:"M_NAME":S_update_dialog(%s): Error retrieving %s contact\n",str1,str1);
//			return CSCF_RETURN_BREAK;
//		}
//	}		
		
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;

	LOG(L_INFO,"DBG:"M_NAME":S_update_dialog(%s): Call-ID <%.*s>\n",str1,call_id.len,call_id.s);

	d = get_s_dialog_dir(call_id,dir);
//	if (!d && msg->first_line.type==SIP_REPLY){
//		/* Try to get the dialog from the request */
//		if (!req) req = cscf_get_request_from_reply(msg);		
//		if (!find_dialog_aor(req,str1,&aor)){
//			LOG(L_ERR,"ERR:"M_NAME":S_update_dialog(%s): Error retrieving %s contact\n",str1,str1);
//			return CSCF_RETURN_BREAK;
//		}		
//		d = get_s_dialog_dir(call_id,aor);		
//	}
	if (!d){
		LOG(L_INFO,"INFO:"M_NAME":S_update_dialog: dialog does not exists!\n");	
		return CSCF_RETURN_FALSE;
	}


	if (msg->first_line.type==SIP_REQUEST){
		/* Request */
		LOG(L_DBG,"DBG:"M_NAME":S_update_dialog(%s): Method <%.*s> \n",str1,
			msg->first_line.u.request.method.len,msg->first_line.u.request.method.s);
		cseq = cscf_get_cseq(msg,&h);
		if (cseq>d->last_cseq) d->last_cseq = cseq;
		d->expires = d_act_time()+scscf_dialogs_expiration_time;
	}else{
		/* Reply */
		response = msg->first_line.u.reply.statuscode;
		LOG(L_DBG,"DBG:"M_NAME":S_update_dialog(%s): <%d> \n",str1,response);
		cseq = cscf_get_cseq(msg,&h);
		if (cseq==0 || h==0) return CSCF_RETURN_FALSE;
		if (d->first_cseq==cseq && d->method_str.len == ((struct cseq_body *)h->parsed)->method.len &&
			strncasecmp(d->method_str.s,((struct cseq_body *)h->parsed)->method.s,d->method_str.len)==0 &&
			d->state < DLG_STATE_CONFIRMED){
			/* reply to initial request */
			if (response<200){
				d->state = DLG_STATE_EARLY;
				d->expires = d_act_time()+300;
			}else
			if (response>=200 && response<300){
				d->state = DLG_STATE_CONFIRMED;
				d->expires = d_act_time()+scscf_dialogs_expiration_time;
				/*I save the dialogs only here because
				 * i only want to release confirmed dialogs*/
				cscf_get_to_tag(msg,&totag);
				tmb.update_dlg_uas(d->dialog_s,response,&totag);
				tmb.dlg_response_uac(d->dialog_c,msg,IS_NOT_TARGET_REFRESH);				
			}else
				if (response>300){
					d->state = DLG_STATE_TERMINATED;
					d_unlock(d->hash);				
					return S_drop_dialog(msg,str1,str2);
				}				
		}else{
			/* reply to subsequent request */			
			if (!req) req = cscf_get_request_from_reply(msg);
			
			/* destroy dialogs on specific methods */
			switch (d->method){
				case DLG_METHOD_OTHER:							
					d->expires = d_act_time()+scscf_dialogs_expiration_time;
					break;
				case DLG_METHOD_INVITE:
					if (req && req->first_line.u.request.method.len==3 &&
						strncasecmp(req->first_line.u.request.method.s,"BYE",3)==0){
						d->state = DLG_STATE_TERMINATED;
						d_unlock(d->hash);				
						return S_drop_dialog(msg,str1,str2);
					}
					d->expires = d_act_time()+scscf_dialogs_expiration_time;
					break;
				case DLG_METHOD_SUBSCRIBE:
//					if (req && req->first_line.u.request.method.len==9 &&
//						strncasecmp(req->first_line.u.request.method.s,"SUBSCRIBE",9)==0 &&
//						cscf_get_expires_hdr(msg)==0){						
//						d->state = DLG_STATE_TERMINATED;
//						d_unlock(d->hash);				
//						return P_dros_dialog(msg,str1,str2);
//					}
					if (req && req->first_line.u.request.method.len==6 &&
						strncasecmp(req->first_line.u.request.method.s,"NOTIFY",6)==0){
						expires = cscf_get_subscription_state(req);
						if (expires==0){						
							d->state = DLG_STATE_TERMINATED;
							d_unlock(d->hash);				
							return S_drop_dialog(msg,str1,str2);
						}else if (expires>0){
							d->expires = expires;
						}
					}
					break;
			}
			if (cseq>d->last_cseq) d->last_cseq = cseq;						
		}
	}
	
	d_unlock(d->hash);
	
//	print_s_dialogs(L_INFO);
	
	return CSCF_RETURN_TRUE;	
}


/**
 * Drops and deletes a dialog.
 * @param msg - the request/response
 * @param str1 - direction - "orig" or "term"
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 */
int S_drop_dialog(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id;
	s_dialog *d;
	int hash;
//	struct sip_msg *req;
	enum s_dialog_direction dir = get_dialog_direction(str1);
	
	
//	if (!find_dialog_aor(msg,str1,&aor)){
//		LOG(L_ERR,"ERR:"M_NAME":S_is_in_dialog(): Error retrieving %s contact\n",str1);
//		return CSCF_RETURN_BREAK;
//	}		
		
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;

	LOG(L_INFO,"DBG:"M_NAME":S_drop_dialog(%s): Call-ID <%.*s> DIR[%d]\n",
		str1,call_id.len,call_id.s,
		dir);

	d = get_s_dialog_dir(call_id,dir);
//	if (!d && msg->first_line.type==SIP_REPLY){
//		/* Try to get the dialog from the request */
//		req = cscf_get_request_from_reply(msg);		
//		if (!find_dialog_aor(req,str1,&aor)){
//			LOG(L_ERR,"ERR:"M_NAME":S_update_dialog(%s): Error retrieving %s contact\n",str1,str1);
//			return CSCF_RETURN_BREAK;
//		}		
//		d = get_s_dialog(call_id,aor);		
//	}
	if (!d){
		LOG(L_ERR,"ERR:"M_NAME":S_drop_dialog: dialog does not exists!\n");	
		return CSCF_RETURN_FALSE;
	}

	hash = d->hash;
	
	del_s_dialog(d);
		
	d_unlock(hash);
	
//	print_s_dialogs(L_INFO);
	
	return CSCF_RETURN_TRUE;	
}

/**
 * Drop all dialogs belonging to one AOR.
 *  on deregistration for example.
 * @param aor - the public identity of the user
 * @returns the number of dialogs dropped 
 */
int S_drop_all_dialogs(str aor)
{
	s_dialog *d,*dn;
	int i,cnt=0;;
	
	LOG(L_DBG,"DBG:"M_NAME":S_drop_all_dialogs: Called for <%.*s>\n",aor.len,aor.s);

	for(i=0;i<s_dialogs_hash_size;i++){
		d_lock(i);
			d = s_dialogs[i].head;
			while(d){
				dn = d->next;
				if (d->aor.len == aor.len &&
					strncasecmp(d->aor.s,aor.s,aor.len)==0) {
					if (!terminate_s_dialog(d)) del_s_dialog(d);
					cnt++;
				}						
				d = dn;
			}
		d_unlock(i);
	}
	//print_s_dialogs(L_INFO);	
	return cnt;
}


static str s_record_route_s={"Record-Route: <",15};
static str s_record_route_e={";lr>\r\n",6};
/**
 * Record routes, with given user as parameter.
 * @param msg - the SIP message to add to
 * @param str1 - direction - "orig" or "term"
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error
 */ 
int S_record_route(struct sip_msg *msg,char *str1,char *str2)
{
	str rr;
	str u = {0,0},scheme={0,0},scscf={0,0};
	
	enum s_dialog_direction dir = get_dialog_direction(str1);
	
	switch (dir){
		case DLG_MOBILE_ORIGINATING:
			STR_PKG_DUP(rr,scscf_record_route_mo,"pkg");
			break;
		case DLG_MOBILE_TERMINATING:
			STR_PKG_DUP(rr,scscf_record_route_mt,"pkg");
			break;
		default:
			u.s = str1;
			u.len = strlen(str1);
			if (scscf_name_str.len>4 &&
				strncasecmp(scscf_name_str.s,"sip:",4)==0){
				scheme.s = scscf_name_str.s;
				scheme.len = 4;
			}else if (scscf_name_str.len>5 &&
				strncasecmp(scscf_name_str.s,"sips:",5)==0){
				scheme.s = scscf_name_str.s;
				scheme.len = 4;
			}
			scscf.s = scheme.s+scheme.len;
			scscf.len = scscf_name_str.len - scheme.len;
			
			rr.len = s_record_route_s.len+scheme.len+u.len+1+scscf.len+s_record_route_e.len;
			rr.s = pkg_malloc(rr.len);
			if (!rr.s){
				LOG(L_ERR,"ERR:"M_NAME":S_record_route: error allocating %d bytes!\n",rr.len);	
				return CSCF_RETURN_BREAK;
			}
			rr.len = 0;
			STR_APPEND(rr,s_record_route_s);
			STR_APPEND(rr,scheme);
			STR_APPEND(rr,u);
			rr.s[rr.len++]='@';
			STR_APPEND(rr,scscf);
			STR_APPEND(rr,s_record_route_e);					
	}
	
	if (cscf_add_header_first(msg,&rr,HDR_RECORDROUTE_T)) return CSCF_RETURN_TRUE;
	else{
		if (rr.s) pkg_free(rr.s);
		return CSCF_RETURN_BREAK;
	}
}

/**
 * Check if we already did record-route
 * @param msg - the SIP message to add to
 * @param str1 - direction - "orig" or "term"
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error
 */
int S_is_record_routed(struct sip_msg *msg,char *str1,char *str2)
{
	str rr;
	str u = {0,0},scheme={0,0},scscf={0,0};
	struct hdr_field *hdr=0;
	rr_t *rr_s;
	
	enum s_dialog_direction dir = get_dialog_direction(str1);
	
	switch (dir){
		case DLG_MOBILE_ORIGINATING:
			STR_PKG_DUP(rr,scscf_record_route_mo,"pkg");
			break;
		case DLG_MOBILE_TERMINATING:
			STR_PKG_DUP(rr,scscf_record_route_mt,"pkg");
			break;
		default:
			u.s = str1;
			u.len = strlen(str1);
			if (scscf_name_str.len>4 &&
				strncasecmp(scscf_name_str.s,"sip:",4)==0){
				scheme.s = scscf_name_str.s;
				scheme.len = 4;
			}else if (scscf_name_str.len>5 &&
				strncasecmp(scscf_name_str.s,"sips:",5)==0){
				scheme.s = scscf_name_str.s;
				scheme.len = 4;
			}
			scscf.s = scheme.s+scheme.len;
			scscf.len = scscf_name_str.len - scheme.len;
			
			rr.len = scheme.len+u.len+1+scscf.len;
			rr.s = pkg_malloc(rr.len);
			if (!rr.s){
				LOG(L_ERR,"ERR:"M_NAME":S_record_route: error allocating %d bytes!\n",rr.len);	
				return CSCF_RETURN_BREAK;
			}
			rr.len = 0;
			STR_APPEND(rr,scheme);
			STR_APPEND(rr,u);
			rr.s[rr.len++]='@';
			STR_APPEND(rr,scscf);
	}

	for(hdr = cscf_get_next_record_route(msg,(struct hdr_field*) 0); hdr ; hdr = cscf_get_next_record_route(msg,hdr)){
		for (rr_s = (rr_t *)hdr->parsed;rr_s; rr_s = rr_s->next)
			if (rr_s->nameaddr.uri.len == rr.len &&
				strncasecmp(rr_s->nameaddr.uri.s,rr.s,rr.len)==0){
					pkg_free(rr.s);
					return CSCF_RETURN_TRUE;
				}
	}

	pkg_free(rr.s);
	return CSCF_RETURN_FALSE;
}



/**
 * The dialog timer looks for expires dialogs and removes them
 * @param ticks - the current time
 * @param param - pointer to the dialogs list
 */
void dialog_timer(unsigned int ticks, void* param)
{
	s_dialog *d,*dn;
	int i;
	
	LOG(L_DBG,"DBG:"M_NAME":dialog_timer: Called at %d\n",ticks);
	if (!s_dialogs) s_dialogs = (s_dialog_hash_slot*)param;

	d_act_time();
	
	for(i=0;i<s_dialogs_hash_size;i++){
		d_lock(i);
			d = s_dialogs[i].head;
			while(d){
				dn = d->next;
				if (d->expires<=d_time_now) {
					if (!terminate_s_dialog(d)) del_s_dialog(d);
				}						
				d = dn;
			}
		d_unlock(i);
	}
	print_s_dialogs(L_INFO);
}



