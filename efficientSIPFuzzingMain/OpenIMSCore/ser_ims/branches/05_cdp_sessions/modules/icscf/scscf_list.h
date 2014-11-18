/**
 * $Id: scscf_list.h 2 2006-11-14 22:37:20Z vingarzan $
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
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 

#ifndef I_CSCF_SCSCF_LIST_H
#define I_CSCF_SCSCF_LIST_H

#include "../../sr_module.h"
#include "mod.h"

/** S-CSCF list element */ 
typedef struct _scscf_list {
	str scscf_name;	/**< SIP URI of the S-CSCF */
	int score;		/**< score of the match */
	
	struct _scscf_list *next; /**< next S-CSCF in the list */
} scscf_list;

/** S-CSCF list */
typedef struct _s_list {
	str call_id;			/**< Call-Id from the request */
	scscf_list *list;		/**< S-CSCF list */
	
	struct _s_list *next;	/**< Next S-CSCF list in the hash slot */
	struct _s_list *prev;	/**< Previous S-CSCF list in the hash slot */
} s_list;

/** hash slot for S-CSCF lists */
typedef struct {
	s_list *head;					/**< first S-CSCF list in this slot */
	s_list *tail;					/**< last S-CSCF list in this slot */
	gen_lock_t *lock;				/**< slot lock 					*/	
} i_hash_slot;


/** S-CSCF with attached capabilities */
typedef struct _scscf_capabilities {
	int id_s_cscf;					/**< S-CSCF id in the DB */
	str scscf_name;					/**< S-CSCF SIP URI */
	int *capabilities;				/**< S-CSCF array of capabilities*/
	int cnt;						/**< size of S-CSCF array of capabilities*/
} scscf_capabilities;


int I_get_capabilities();

scscf_list* I_get_capab_ordered(str scscf_name,int *m,int mcnt,int *o,int ocnt);


inline unsigned int get_call_id_hash(str callid,int hash_size);

int i_hash_table_init(int hash_size);

void i_hash_table_destroy();

inline void i_lock(unsigned int hash);
inline void i_unlock(unsigned int hash);


s_list* new_s_list(str callid,scscf_list *sl);
int add_s_list(str call_id,scscf_list *sl);
int is_s_list(str call_id);
str take_s_list(str call_id);
void del_s_list(str call_id);
void free_s_list(s_list *sl);
void print_s_list(int log_level);
		
		
int I_trans_in_processing(struct sip_msg* msg, char* str1, char* str2);
int I_scscf_select(struct sip_msg* msg, char* str1, char* str2);
int I_scscf_drop(struct sip_msg* msg, char* str1, char* str2);
		
#endif
