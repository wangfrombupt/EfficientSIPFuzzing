/*
 * $Id: registration.h 569 2008-06-24 19:38:57Z vingarzan $
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
 * Serving-CSCF - Registration Related Operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#ifndef S_CSCF_REGISTRATION_H_
#define S_CSCF_REGISTRATION_H_

#include "mod.h"
#include "../../locking.h"

#define NONCE_LEN 16
#define RAND_LEN  16

unsigned char get_algorithm_type(str algorithm);
unsigned char get_auth_scheme_type(str algorithm);

int S_add_path_service_routes(struct sip_msg *msg,char *str1,char *str2 );

int S_add_allow(struct sip_msg *msg,char *str1,char *str2 );

int S_add_service_route(struct sip_msg *msg,char *str1,char *str2 );

int S_add_p_charging_function_addresses(struct sip_msg *msg,char *str1,char *str2 );

int S_check_visited_network_id(struct sip_msg *msg,char *str1,char *str2 );

int S_REGISTER_reply(struct sip_msg *msg, int code,  char *text);

int S_is_integrity_protected(struct sip_msg *msg,char *str1,char *str2 );

int S_is_authorized(struct sip_msg *msg,char *str1,char *str2 );

int S_challenge(struct sip_msg *msg,char *str1,char *str2 );


enum authorization_types {
	AUTH_UNKNOWN			= 0,
/* 3GPP */	
	AUTH_AKAV1_MD5			= 1,
	AUTH_AKAV2_MD5			= 2,
	AUTH_EARLY_IMS			= 3,
/* FOKUS */
	AUTH_MD5				= 4,
/* CableLabs */	
	AUTH_DIGEST				= 5,
/* TISPAN */	
	AUTH_HTTP_DIGEST_MD5	= 6,	
	AUTH_NASS_BUNDLED		= 7
};

#define AUTH_TYPE_MAX AUTH_NASS_BUNDLED

/** Enumeration for the Authorization Vector status */
enum auth_vector_status {
	AUTH_VECTOR_UNUSED = 0,
	AUTH_VECTOR_SENT = 1,
	AUTH_VECTOR_USED = 2,
	AUTH_VECTOR_USELESS = 3
} ;


/** Authorization Vector storage structure */
typedef struct _auth_vector {
	int item_number;	/**< index of the auth vector		*/
	unsigned char type;	/**< type of authentication vector 	*/
	str authenticate;	/**< challenge (rand|autn in AKA)	*/
	str authorization; 	/**< expected response				*/
	str ck;				/**< Cypher Key						*/
	str ik;				/**< Integrity Key					*/
	time_t expires;/**< expires in (after it is sent)	*/
	
	enum auth_vector_status status;/**< current status		*/
	struct _auth_vector *next;/**< next av in the list		*/
	struct _auth_vector *prev;/**< previous av in the list	*/
} auth_vector;



/** Set of auth_vectors used by a private id */
typedef struct _auth_userdata{
	unsigned int hash;		/**< hash of the auth data		*/
	str private_identity;	/**< authorization username		*/
	str public_identity;	/**< public identity linked to	*/
	time_t expires;			/**< expires in					*/
	
	auth_vector *head;		/**< first auth vector in list	*/
	auth_vector *tail;		/**< last auth vector in list	*/
	
	struct _auth_userdata *next;/**< next element in list	*/
	struct _auth_userdata *prev;/**< previous element in list*/
} auth_userdata;

/** Authorization user data hash slot */
typedef struct {
	auth_userdata *head;				/**< first in the slot			*/ 
	auth_userdata *tail;				/**< last in the slot			*/
	gen_lock_t *lock;			/**< slot lock 							*/	
} auth_hash_slot_t;






int pack_challenge(struct sip_msg *msg,str realm,auth_vector *av);

int S_MAR(struct sip_msg *msg, str public_identity, str private_identity,
					int count,str auth_scheme,str nonce,str auts,str server_name,str realm);


/*
 * Storage of authentication vectors
 */

inline void auth_data_lock(unsigned int hash);
inline void auth_data_unlock(unsigned int hash);
 
int auth_data_init(int size);

void auth_data_destroy();

auth_vector *new_auth_vector(int item_number,str auth_scheme,str authenticate,
			str authorization,str ck,str ik);
void free_auth_vector(auth_vector *av);

auth_userdata *new_auth_userdata(str private_identity,str public_identity);
void free_auth_userdata(auth_userdata *aud);					

inline unsigned int get_hash_auth(str private_identity,str public_identity);

int add_auth_vector(str private_identity,str public_identity,auth_vector *av);
auth_vector* get_auth_vector(str private_identity,str public_identity,int status,str *nonce,unsigned int *hash);

int drop_auth_userdata(str private_identity,str public_identity);

inline void start_reg_await_timer(auth_vector *av);

void reg_await_timer(unsigned int ticks, void* param);



#endif //S_CSCF_REGISTRATION_H_
