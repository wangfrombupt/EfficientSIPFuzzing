/**
 * $Id: dlg_state.h 556 2008-04-30 09:15:31Z albertoberlios $
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
 * Proxy-CSCF - Dialog State
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 

#ifndef P_CSCF_DLG_STATE_H
#define P_CSCF_DLG_STATE_H

#include "../../sr_module.h"
#include "mod.h"
#include "../../locking.h"
#include "../tm/dlg.h"
#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"

enum p_dialog_method {
	DLG_METHOD_OTHER=0,
	DLG_METHOD_INVITE=1,
	DLG_METHOD_SUBSCRIBE=2	
};

/** The last dialog type */
#define DLG_METHOD_MAX DLG_METHOD_SUBSCRIBE

enum p_dialog_state {
	DLG_STATE_UNKNOWN=0,
	DLG_STATE_INITIAL=1,
	DLG_STATE_EARLY=2,
	DLG_STATE_CONFIRMED=3,
	DLG_STATE_TERMINATED_ONE_SIDE=4,
	DLG_STATE_TERMINATED=5	
};

enum p_dialog_direction {
	DLG_MOBILE_ORIGINATING=0,
	DLG_MOBILE_TERMINATING=1,
	DLG_MOBILE_UNKNOWN=2
};

typedef struct _p_dialog {
	unsigned int hash;
	str call_id;
	enum p_dialog_direction direction; /*Needed to locate dialogs easier in release_call*/
	
	str host;					/**< host of the UE						*/
	unsigned short port;					/**< port of the UE						*/
	char transport;				/**< transport for the UE				*/
	
	str *routes;
	unsigned short routes_cnt;
	
	enum p_dialog_method method;
	str method_str;
	int first_cseq;
	int last_cseq;
	enum p_dialog_state state;
	time_t expires;
	time_t lr_session_expires;  		/**< last remember request - session-expires header			*/
	str refresher;						/**< session refresher				*/
	unsigned char uac_supp_timer; 		/** < requester uac supports timer */
	
	unsigned char is_releasing;			/**< weather this dialog is already being 
	  										released or not, or its peer, with count on 
											tries 										*/
											
	AAASession *pcc_session;
													
	dlg_t *dialog_s;  /* dialog as UAS*/
	dlg_t *dialog_c;  /* dialog as UAC*/
			
	struct _p_dialog *next,*prev;	
} p_dialog;

typedef struct {
	p_dialog *head,*tail;
	gen_lock_t *lock;				/**< slot lock 					*/	
} p_dialog_hash_slot;


inline unsigned int get_p_dialog_hash(str call_id);

int p_dialogs_init(int hash_size);

void p_dialogs_destroy();

inline void d_lock(unsigned int hash);
inline void d_unlock(unsigned int hash);

inline int find_dialog_contact(struct sip_msg *msg,char *direction,str *host,int *port,int *transport);
p_dialog* new_p_dialog(str call_id,str host,int port, int transport);
p_dialog* add_p_dialog(str call_id,str host,int port, int transport);
int is_p_dialog(str call_id,str host,int port, int transport);
int is_p_dialog_dir(str call_id,enum p_dialog_direction dir);
p_dialog* get_p_dialog(str call_id,str host,int port, int transport);
p_dialog* get_p_dialog_dir(str call_id,enum p_dialog_direction dir);
p_dialog* get_p_dialog_dir_nolock(str call_id,enum p_dialog_direction dir);
int terminate_p_dialog(p_dialog *d);
void del_p_dialog(p_dialog *d);
void free_p_dialog(p_dialog *d);
void print_p_dialogs(int log_level);
		


int P_is_in_dialog(struct sip_msg* msg, char* str1, char* str2);

int P_save_dialog(struct sip_msg* msg, char* str1, char* str2);

int P_update_dialog(struct sip_msg* msg, char* str1, char* str2);

int P_drop_dialog(struct sip_msg* msg, char* str1, char* str2);

int P_drop_all_dialogs(str host,int port, int transport);

int P_follows_dialog_routes(struct sip_msg *msg,char *str1,char *str2);

int P_enforce_dialog_routes(struct sip_msg *msg,char *str1,char*str2);

int P_record_route(struct sip_msg *msg,char *str1,char *str2);

int P_check_session_expires(struct sip_msg* msg, char* str1, char* str2);

int P_422_session_expires(struct sip_msg* msg, char* str1, char* str2);

void dialog_timer(unsigned int ticks, void* param);
		
#endif
