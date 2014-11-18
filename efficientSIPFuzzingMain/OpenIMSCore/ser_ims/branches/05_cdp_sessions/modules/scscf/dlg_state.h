/*
 * $Id: dlg_state.h 232 2007-04-17 20:37:06Z vingarzan $
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
 */
 

#ifndef S_CSCF_DLG_STATE_H
#define S_CSCF_DLG_STATE_H

#include "../../sr_module.h"
#include "mod.h"
#include "../../locking.h"
#include "../tm/dlg.h"

/** Enumeration for known dialogs */
enum s_dialog_method {
	DLG_METHOD_OTHER=0,
	DLG_METHOD_INVITE=1,
	DLG_METHOD_SUBSCRIBE=2	
};

/** Enumeration for dialog states */
enum s_dialog_state {
	DLG_STATE_UNKNOWN=0,
	DLG_STATE_INITIAL=1,
	DLG_STATE_EARLY=2,
	DLG_STATE_CONFIRMED=3,
	DLG_STATE_TERMINATED_ONE_SIDE=4,
	DLG_STATE_TERMINATED=5	
};

/** Enumeration for dialog directions */
enum s_dialog_direction {
	DLG_MOBILE_ORIGINATING=0,
	DLG_MOBILE_TERMINATING=1,
	DLG_MOBILE_UNKNOWN=2
};

/** Structure for S-CSCF dialogs */
typedef struct _s_dialog {
	unsigned int hash;					/**< hash for the dialog 						*/
	str call_id;						/**< call-id for the dialog 					*/
	enum s_dialog_direction direction;	/**< direction									*/
	
	str aor;							/**< Public Identity of the user				*/
			
	enum s_dialog_method method;		/**< method of the initial request enumeration	*/
	str method_str;						/**< method of the initial request string 		*/
	int first_cseq;						/**< first (initial request) CSeq				*/
	int last_cseq;						/**< last seen CSeq								*/
	enum s_dialog_state state;			/**< state of the dialog						*/
	time_t expires;						/**< expiration time for the dialog				*/
	
	unsigned char is_releasing;			/**< weather this dialog is already being 
											released or not, or its peer, with count on 
											tries 										*/	
	dlg_t *dialog_c;					/**< dialog in direction to callee           	*/
	dlg_t *dialog_s;					/**< dialog in direction to caller 				*/
		
	struct _s_dialog *next;				/**< next dialog in this dialog hash slot 		*/
	struct _s_dialog *prev;				/**< previous dialog in this dialog hash slot	*/
} s_dialog;

/** Structure for a S-CSCF dialog hash slot */
typedef struct {
	s_dialog *head;						/**< first dialog in this dialog hash slot 		*/
	s_dialog *tail;						/**< last dialog in this dialog hash slot 		*/
	gen_lock_t *lock;					/**< slot lock 									*/	
} s_dialog_hash_slot;


inline unsigned int get_s_dialog_hash(str call_id);

int s_dialogs_init(int hash_size);

void s_dialogs_destroy();

inline void d_lock(unsigned int hash);
inline void d_unlock(unsigned int hash);


s_dialog* new_s_dialog(str call_id,str aor,enum s_dialog_direction dir);
s_dialog* add_s_dialog(str call_id,str aor,enum s_dialog_direction dir);
int is_s_dialog(str call_id,str aor);
int is_s_dialog_dir(str call_id,enum s_dialog_direction dir);
s_dialog* get_s_dialog(str call_id,str aor);
s_dialog* get_s_dialog_dir(str call_id,enum s_dialog_direction dir);
s_dialog* get_s_dialog_dir_nolock(str call_id,enum s_dialog_direction dir);
int terminate_s_dialog(s_dialog *d);
void del_s_dialog(s_dialog *d);
void free_s_dialog(s_dialog *d);
void print_s_dialogs(int log_level);
		


int S_is_in_dialog(struct sip_msg* msg, char* str1, char* str2);

int S_save_dialog(struct sip_msg* msg, char* str1, char* str2);

int S_update_dialog(struct sip_msg* msg, char* str1, char* str2);

int S_drop_dialog(struct sip_msg* msg, char* str1, char* str2);

int S_drop_all_dialogs(str aor);

int S_record_route(struct sip_msg *msg,char *user,char *str2);

int S_is_record_routed(struct sip_msg *msg,char *str1,char *str2);

void dialog_timer(unsigned int ticks, void* param);
		
#endif
