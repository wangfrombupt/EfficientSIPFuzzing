/**
 * $Id: scscf_list.c 583 2008-09-08 11:20:09Z vingarzan $
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
 * Interrogating-CSCF - Serving-CSCFs Lists
 * 
 * The I-CSCF performs a serial fork and tries to deliver the requests to the first
 * S-CSCF that responds, from a list.
 * 
 * The list is received from the HSS or created from the received (from HSS) list of
 * mandatory and optional capabilities for that particular user. This is often called
 * capabilities selection.
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include <values.h>
 
#include "../../mem/shm_mem.h"

#include "../../dset.h"
#include "../tm/tm_load.h"

#include "scscf_list.h"
#include "db.h" 
#include "sip.h"


extern struct tm_binds tmb;                             /**< Structure with pointers to tm funcs                */

scscf_capabilities *SCSCF_Capabilities=0;		/**< list of S-CSCFs and their capabilities */ 
int SCSCF_Capabilities_cnt=0;					/**< size of list of S-CSCFs and their capabilities */

int i_hash_size;					/**< size of the hash table for the S-CSCF lists 	*/
i_hash_slot *i_hash_table=0;		/**< the hash table for the S-CSCF lists				*/


/**
 * Refreshes the capabilities list reading them from the db.
 * Drops the old cache and queries the db
 * \todo - IMPLEMENT A WAY TO PUSH AN EXTERNAL EVENT FOR THIS
 * \todo - SOLVE THE LOCKING PROBLEM - THIS IS A WRITER
 * @returns 1 on success, 0 on failure
 */
int I_get_capabilities()
{
	int i,j,r;
	/* free the old cache */
	if (SCSCF_Capabilities!=0){
		for(i=0;i<SCSCF_Capabilities_cnt;i++){
			if (SCSCF_Capabilities[i].capabilities)
				shm_free(SCSCF_Capabilities[i].capabilities);
		}
		shm_free(SCSCF_Capabilities);
	}
	
	SCSCF_Capabilities_cnt = icscf_db_get_scscf(&SCSCF_Capabilities);
	
	r = icscf_db_get_capabilities(&SCSCF_Capabilities,SCSCF_Capabilities_cnt);

	LOG(L_DBG,"DBG:"M_NAME":------  S-CSCF Map with Capabilities  begin ------\n");
	if (SCSCF_Capabilities!=0){
		for(i=0;i<SCSCF_Capabilities_cnt;i++){
			LOG(L_DBG,"DBG:"M_NAME":S-CSCF [%d] <%.*s>\n",
				SCSCF_Capabilities[i].id_s_cscf,
				SCSCF_Capabilities[i].scscf_name.len,
				SCSCF_Capabilities[i].scscf_name.s);
			for(j=0;j<SCSCF_Capabilities[i].cnt;j++)
			LOG(L_DBG,"DBG:"M_NAME":       \t [%d]\n",
			 SCSCF_Capabilities[i].capabilities[j]);
		}
	}
	LOG(L_DBG,"DBG:"M_NAME":------  S-CSCF Map with Capabilities  end ------\n");
	
	return r;
}

/**
 * Returns the matching rank of a S-CSCF
 * \todo - optimize the search as O(n^2) is hardly desireable
 * @param c - the capabilities of the S-CSCF
 * @param m - mandatory capabilities list requested
 * @param mcnt - mandatory capabilities list size
 * @param o - optional capabilities list
 * @param ocnt - optional capabilities list sizeint I_get_capab_match(icscf_capabilities *c,int *m,int mcnt,int *o,int ocnt)
 * @returns - -1 if mandatory not satisfied, else count of matched optional capab
 */
int I_get_capab_match(scscf_capabilities *c,int *m,int mcnt,int *o,int ocnt)
{
	int r=0,i,j,t=0;
	for(i=0;i<mcnt;i++){
		t=0;
		for(j=0;j<c->cnt;j++)
			if (m[i]==c->capabilities[j]) {
				t=1;
				break;
			}
		if (!t) return -1;
	}
	for(i=0;i<ocnt;i++){
		for(j=0;j<c->cnt;j++)
			if (o[i]==c->capabilities[j]) r++;
	}	
	return r;
}

/**
 * Adds the name to the list starting at root, ordered by score.
 * Returns the new root
 */
static inline scscf_entry* I_add_to_scscf_list(scscf_entry *root,str name,int score, int originating)
{
	scscf_entry *x,*i;

	//duplicate?
	for(i=root;i;i=i->next)
		if (name.len == i->scscf_name.len &&
			strncasecmp(name.s,i->scscf_name.s,name.len)==0)
				return root;

	x = new_scscf_entry(name,score, originating);
	if (!x) return root;
	
	if (!root){
		return x;
	}
	if (root->score < x->score){
		x->next = root;
		return x;
	}
	i = root;
	while(i->next && i->next->score > x->score)
		i = i->next;
	x->next = i->next;
	i->next = x;
	return root;
}

/**
 * Returns a list of S-CSCFs that we should try on, based on the
 * capabilities requested
 * \todo - order the list according to matched optionals - r
 * @param scscf_name - the first S-CSCF if specified
 * @param m - mandatory capabilities list
 * @param mcnt - mandatory capabilities list size
 * @param o - optional capabilities list
 * @param ocnt - optional capabilities list size
 * @param orig - indicates originating session case
 * @returns list of S-CSCFs, terminated with a str={0,0}
 */
scscf_entry* I_get_capab_ordered(str scscf_name,int *m,int mcnt,int *o,int ocnt, str *p, int pcnt,int orig)
{
	scscf_entry *list=0;
	int i,r;
	
	if (scscf_name.len) list = I_add_to_scscf_list(list,scscf_name,MAXINT, orig);

	for(i=0;i<pcnt;i++)
		list = I_add_to_scscf_list(list,p[i],MAXINT-i,orig);
		
	for(i=0;i<SCSCF_Capabilities_cnt;i++){
		r = I_get_capab_match(SCSCF_Capabilities+i,m,mcnt,o,ocnt);
		if (r!=-1){
			 list = I_add_to_scscf_list(list,SCSCF_Capabilities[i].scscf_name,r, orig);
			 LOG(L_DBG,"DBG:"M_NAME":I_get_capab_ordered: <%.*s> Added to the list, orig=%d\n",
			 	SCSCF_Capabilities[i].scscf_name.len,SCSCF_Capabilities[i].scscf_name.s, orig);
		}
	}
	return list;
}


/**
 * Computes the hash for a string.
 */
inline unsigned int get_call_id_hash(str callid,int hash_size)
{
#define h_inc h+=v^(v>>3)
   char* p;
   register unsigned v;
   register unsigned h;

   h=0;
   for (p=callid.s; p<=(callid.s+callid.len-4); p+=4){
       v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
       h_inc;
   }
   v=0;
   for (;p<(callid.s+callid.len); p++) {
       v<<=8;
       v+=*p;
   }
   h_inc;

   h=((h)+(h>>11))+((h>>13)+(h>>23));
   return (h)%hash_size;
#undef h_inc 
}

/**
 * Initialize the hash with S-CSCF lists
 */
int i_hash_table_init(int hash_size)
{
	int i;
	
	i_hash_size = hash_size;
	i_hash_table = shm_malloc(sizeof(i_hash_slot)*i_hash_size);

	if (!i_hash_table) return 0;
	
	memset(i_hash_table,0,sizeof(i_hash_slot)*i_hash_size);
	
	for(i=0;i<i_hash_size;i++){
		i_hash_table[i].lock = lock_alloc();
		if (!i_hash_table[i].lock){
			LOG(L_ERR,"ERR:"M_NAME":i_hash_table_init(): Error creating lock\n");
			return 0;
		}
		i_hash_table[i].lock = lock_init(i_hash_table[i].lock);
	}
			
	return 1;
}

/**
 * Destroy the hash with S-CSCF lists
 */
void i_hash_table_destroy()
{
	int i;
	scscf_list *sl,*nsl;
	for(i=0;i<i_hash_size;i++){
		i_lock(i);
			sl = i_hash_table[i].head;
			while(sl){
				nsl = sl->next;
				free_scscf_list(sl);
				sl = nsl;
			}
		i_unlock(i);
		lock_dealloc(i_hash_table[i].lock);
	}
	shm_free(i_hash_table);
}

/**
 * Locks the required part of hash with S-CSCF lists
 */
inline void i_lock(unsigned int hash)
{
//	LOG(L_CRIT,"GET %d\n",hash);
	lock_get(i_hash_table[(hash)].lock);
//	LOG(L_CRIT,"GOT %d\n",hash);	
}

/**
 * UnLocks the required part of hash with S-CSCF lists
 */
inline void i_unlock(unsigned int hash)
{
	lock_release(i_hash_table[(hash)].lock);
//	LOG(L_CRIT,"RELEASED %d\n",hash);	
}

scscf_entry* new_scscf_entry(str name, int score, int orig)
{
	scscf_entry *x=0;
	x = shm_malloc(sizeof(scscf_entry));
	if (!x) {	
		LOG(L_ERR,"ERR:"M_NAME":new_scscf_entry: Error allocating %d bytes\n",
			sizeof(scscf_entry));
		return 0;
	}
	/* duplicate always the scscf_name because of possible list reloads and scscf_name coming in LIA/UAA */
	if (orig) x->scscf_name.s = shm_malloc(name.len+5);
	else x->scscf_name.s = shm_malloc(name.len);
	if (!x->scscf_name.s){	
		LOG(L_ERR,"ERR:"M_NAME":new_scscf_entry: Error allocating %d bytes\n",
			orig?name.len+5:name.len);
		shm_free(x);
		return 0;
	}
	memcpy(x->scscf_name.s,name.s,name.len);
	x->scscf_name.len = name.len;
	if (orig) {
		memcpy(x->scscf_name.s+name.len, ";orig", 5);
		x->scscf_name.len += 5;
	}
	
	LOG(L_INFO,"INFO:"M_NAME":new_scscf_entry:  <%.*s>\n",x->scscf_name.len,x->scscf_name.s);
	
	x->score = score;
	x->next = 0;
	return x;
}


scscf_list* new_scscf_list(str call_id,scscf_entry *sl)
{
	scscf_list *l;
	
	l = shm_malloc(sizeof(scscf_list));
	if (!l) {
		LOG(L_ERR,"ERR:"M_NAME":new_scscf_list(): Unable to alloc %d bytes\n",
			sizeof(scscf_list));
		goto error;
	}
	memset(l,0,sizeof(scscf_list));
	
	STR_SHM_DUP(l->call_id,call_id,"shm");
	l->list = sl;
				
	return l;
error:
out_of_memory:
	if (l){
		shm_free(l);		
	}
	return 0;
}

int add_scscf_list(str call_id,scscf_entry *sl)
{
	scscf_list *l;
	unsigned int hash = get_call_id_hash(call_id,i_hash_size);
	
	l = new_scscf_list(call_id,sl);
	if (!l) return 0;		
	
	i_lock(hash);
	l->prev = 0;
	l->next = i_hash_table[hash].head;
	if (l->next) l->next->prev = l;
	i_hash_table[hash].head = l;
	if (!i_hash_table[hash].tail) i_hash_table[hash].tail = l;
	i_unlock(hash);	
	
	return 1;
}

int is_scscf_list(str call_id)
{
	scscf_list *l=0;
	unsigned int hash = get_call_id_hash(call_id,i_hash_size);

	i_lock(hash);
	l = i_hash_table[hash].head;
	while(l){
		if (l->call_id.len == call_id.len &&
			strncasecmp(l->call_id.s,call_id.s,call_id.len)==0) {
				i_unlock(hash);
				return 1;
			}
		l = l->next;
	}
	i_unlock(hash);
	return 0;
}

/**
 * Takes on S-CSCF name for the respective Call-ID from the respective name list.
 * Should shm_free the result.s when no longer needed.
 * @param call_id - the id of the call
 * @returns the shm_malloced S-CSCF name if found or empty string if list is empty or does not exists 
 */  
str take_scscf_entry(str call_id)
{
	str scscf={0,0};
	scscf_list *l=0;
	scscf_entry *sl;
	unsigned int hash = get_call_id_hash(call_id,i_hash_size);

	i_lock(hash);
	l = i_hash_table[hash].head;
	while(l){
		if (l->call_id.len == call_id.len &&
			strncasecmp(l->call_id.s,call_id.s,call_id.len)==0) {
				if (l->list){
					scscf = l->list->scscf_name;
					sl = l->list->next;
					shm_free(l->list);
					l->list = sl;
				}
				break;
			}
		l = l->next;
	}
	i_unlock(hash);
	return scscf;
}

void del_scscf_list(str call_id)
{
	scscf_list *l=0;
	unsigned int hash = get_call_id_hash(call_id,i_hash_size);

	i_lock(hash);
	l = i_hash_table[hash].head;
	while(l){
		if (l->call_id.len == call_id.len &&
			strncasecmp(l->call_id.s,call_id.s,call_id.len)==0) {
				if (l->prev) l->prev->next = l->next;
				else i_hash_table[hash].head = l->next;
				if (l->next) l->next->prev = l->prev;
				else i_hash_table[hash].tail = l->prev;					
				i_unlock(hash);
				free_scscf_list(l);
				return;
			}
		l = l->next;
	}
	i_unlock(hash);
}

void free_scscf_list(scscf_list *sl)
{
	scscf_entry *i;
	if (!sl) return;
	if (sl->call_id.s) shm_free(sl->call_id.s);
	while (sl->list) {
		i = sl->list->next;
		if (sl->list->scscf_name.s) shm_free(sl->list->scscf_name.s);
		shm_free(sl->list);
		sl->list = i;
	}
	shm_free(sl);
}

void print_scscf_list(int log_level)
{
	scscf_list *l;
	int i;
	scscf_entry *sl;
	LOG(log_level,"INF:"M_NAME":----------  S-CSCF Lists begin --------------\n");
	for(i=0;i<i_hash_size;i++){
		i_lock(i);
			l = i_hash_table[i].head;
			while(l){
				LOG(log_level,"INF:"M_NAME":[%4d] Call-ID: <%.*s> \n",i,
					l->call_id.len,l->call_id.s);
				sl = l->list;
				while(sl){
					LOG(log_level,"INF:"M_NAME":         Score:[%4d] S-CSCF: <%.*s> \n",
						sl->score,
						sl->scscf_name.len,sl->scscf_name.s);					
					sl = sl->next;
				}
				l = l->next;
			} 		
		i_unlock(i);
	}
	LOG(log_level,"INF:"M_NAME":----------  S-CSCF Lists end   --------------\n");
	
}

int I_trans_in_processing(struct sip_msg* msg, char* str1, char* str2)
{
        unsigned int hash, label;
        if (tmb.t_get_trans_ident(msg,&hash,&label)<0)
            return CSCF_RETURN_FALSE;
        return CSCF_RETURN_TRUE;
}

static str route_hdr_s={"Route: <",8};
static str route_hdr_e={">\r\n",3};

int I_scscf_select(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id,scscf_name={0,0};
	struct sip_msg *req;
	int result;
	str hdr={0,0};

	//print_scscf_list(L_ERR);
		
	call_id = cscf_get_call_id(msg,0);
	LOG(L_DBG,"DBG:"M_NAME":I_scscf_select(): <%.*s>\n",call_id.len,call_id.s);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;
	
	scscf_name = take_scscf_entry(call_id);
	if (!scscf_name.len){
		I_scscf_drop(msg,str1,str2);
		cscf_reply_transactional(msg,600,MSG_600_FORWARDING_FAILED);			
		return CSCF_RETURN_BREAK;
	}
	
	if (msg->first_line.u.request.method.len==8 &&
		strncasecmp(msg->first_line.u.request.method.s,"REGISTER",8)==0) {
		/* REGISTER fwding */			
		if (str1&&str1[0]=='0'){
			/* first time */	
			//LOG(L_CRIT,"rewrite uri\n");
			if (rewrite_uri(msg, &(scscf_name)) < 0) {
				LOG(L_ERR,"ERR:"M_NAME":I_UAR_forward: Unable to Rewrite URI\n");
				result = CSCF_RETURN_FALSE;
			}else
				result = CSCF_RETURN_TRUE;
		}else{
			/* subsequent */
			//LOG(L_CRIT,"append branch\n");
			req = msg;//cscf_get_request_from_reply(msg);
			append_branch(req,scscf_name.s,scscf_name.len,0,0,0,0);
			result = CSCF_RETURN_TRUE;
		}
	}else{
		/* Another request */
		result = CSCF_RETURN_TRUE;
		
		hdr.len = route_hdr_s.len+scscf_name.len+route_hdr_e.len;
		hdr.s = pkg_malloc(hdr.len);
		if (!hdr.s){
			LOG(L_ERR,"ERR:"M_NAME":Mw_REQUEST_forward: Error allocating %d bytes\n",
				hdr.len);
			result = CSCF_RETURN_TRUE;
		}
		hdr.len=0;
		STR_APPEND(hdr,route_hdr_s);
		STR_APPEND(hdr,scscf_name);
		STR_APPEND(hdr,route_hdr_e);
	
		if (!cscf_add_header_first(msg,&hdr,HDR_ROUTE_T)){
			pkg_free(hdr.s);
			result = CSCF_RETURN_TRUE;
		}
		
		if (msg->dst_uri.s) pkg_free(msg->dst_uri.s);	
		STR_PKG_DUP(msg->dst_uri,scscf_name,"pkg");
	}

	if (scscf_name.s) shm_free(scscf_name.s);
	return result;
out_of_memory:	
	if (scscf_name.s) shm_free(scscf_name.s);
	return CSCF_RETURN_ERROR;
}

int I_scscf_drop(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id;
	//print_scscf_list(L_ERR);
	call_id = cscf_get_call_id(msg,0);
	LOG(L_DBG,"DBG:"M_NAME":I_scscf_drop(): <%.*s>\n",call_id.len,call_id.s);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;
	
	del_scscf_list(call_id);
	return CSCF_RETURN_TRUE;
}

