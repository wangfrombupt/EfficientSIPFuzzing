/*
 * $Id: registrar_storage.h 336 2007-06-21 23:07:09Z flp $
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
 * Proxy-CSCF - Registrar Storage
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#ifndef P_CSCF_REGISTRAR_STORAGE_H_
#define P_CSCF_REGISTRAR_STORAGE_H_

#include "../../sr_module.h"
#include "../../locking.h"

/** NAT address */
typedef struct _r_nat_dest{
	struct ip_addr nat_addr;	/**< address of the pin hole in the NAT	*/
	unsigned short nat_port;	/**< port of the pin hole in the NAT 	*/
}r_nat_dest;

/** Registrar Public Identity Structure */
typedef struct _r_public {
	str aor;					/**< the public identity 				*/
	char is_default;			/**< if this is the default id			*/			
	
	struct _r_public *next;		/**< next public identity for this contact */
	struct _r_public *prev; 	/**< previous public identity for this contact */
} r_public;

/** User Registration States */ 
enum Reg_States {
	NOT_REGISTERED=0,			/**< User not-registered */
	REGISTERED=1,				/**< User registered	 */
	REG_PENDING=-1,				/**< Registration pending*/
	DEREGISTERED=-2				/**< DeRegistered-IPSec on - used to relay last messages to freshly de-registered user*/
} ;

/** IPSec SA Information */
typedef struct _r_ipsec {
	int spi_uc;					/**< SPI Client to use					*/
	int spi_us;					/**< SPI Server to use					*/	
	int spi_pc;					/**< SPI Client to use					*/
	int spi_ps;					/**< SPI Server to use					*/
	unsigned short port_uc;		/**< Port UE Client						*/
	unsigned short port_us;		/**< Port UE Server						*/
	
	str ealg;					/**< Cypher Algorithm - ESP				*/
	str r_ealg;					/**< received Cypher Algorithm - ESP				*/
	str ck;						/**< Cypher Key							*/
	str alg;					/**< Integrity Algorithm - AH			*/
	str r_alg;				/**<received Integrity Algorithm - AH			*/
	str ik;						/**< Integrity Key						*/
} r_ipsec;

/** TLS SA Information */
typedef struct _r_tls {
	unsigned short port_tls;	/**< Port UE TLS						*/
	unsigned long session_hash;
} r_tls;

typedef enum _r_sec_type {
	SEC_NONE	=0,
	SEC_IPSEC	=1,
	SEC_TLS		=2,
} r_security_type;

typedef struct _r_security {
	str sec_header;				/**< Security Header value 				*/
	r_security_type type;		/**< Type of security in use			*/ 
	union {
		r_ipsec *ipsec;			/**< IPSec SA information, if any		*/
		r_tls *tls;				/**< TLS SA information, if any 		*/
	} data;
	float q;
} r_security;

/** Registrar Contact Structure */
typedef struct _r_contact {
	unsigned int hash;			/**< the hash value 					*/
	
	str host;					/**< host of the UE						*/
	unsigned short port;		/**< port of the UE						*/
	char transport;				/**< transport for the UE				*/
			
	r_security *security_temp;  /**< Security-Client Information		*/
	r_security *security;	    /**< Security-Client Information		*/
	
	str uri;					/**< uri of contact						*/		
	
	enum Reg_States reg_state;	/**< registration state					*/
	time_t expires;				/**< time of expiration					*/
	
	unsigned short service_route_cnt;/**< size of the above vector		*/
	str *service_route;			/**< service route entries				*/

	r_nat_dest * pinhole;		/**< address of the receive				*/ 
	
	r_public *head;				/**< first (and default) public identity*/
	r_public *tail;				/**< last public identity				*/

	struct _r_contact *next;	/**< next contact in this hash slot 	*/
	struct _r_contact *prev;	/**< previous contact in this hash slot	*/
} r_contact;

/** Registrar Slot Structure */
typedef struct {
	r_contact *head;			/**< first contact in the slot			*/
	r_contact *tail;			/**< last contact in the slot			*/
	gen_lock_t *lock;			/**< slot lock 							*/
} r_hash_slot;


void r_act_time();
inline int r_valid_contact(r_contact *c);
inline int r_reg_contact(r_contact *c);


int r_storage_init(int hash_size);
void r_storage_destroy();

inline void r_lock(unsigned int hash);
inline void r_unlock(unsigned int hash);

unsigned int get_contact_hash(str aor,int port,int transport,int hash_size);

r_public* new_r_public(str aor, int is_default);
r_public* get_r_public(r_contact *c, str aor);
r_public* add_r_public(r_contact *c,str aor,int is_default);
r_public* update_r_public(r_contact *c,str aor,int *is_default);
void del_r_public(r_contact *c,r_public *p);
void free_r_public(r_public *p);

r_ipsec* new_r_ipsec(int spi_uc,int spi_us,int spi_pc,int spi_ps,int port_uc,int port_us,
	str ealg_setkey,str r_ealg, str ck_esp,str alg_setkey,str r_alg, str ik_esp);
void free_r_ipsec(r_ipsec *ipsec);

r_tls* new_r_tls(int port_tls, unsigned long session_hash);
void free_r_tls(r_tls *tls);

r_security *new_r_security(str sec_header,r_security_type type,float q);
void free_r_security(r_security *s);

r_contact* new_r_contact(str host,int port,int transport,str uri,enum Reg_States reg_state,int expires,
	str *service_route,int service_route_cnt);	
r_contact* get_r_contact(str host,int port,int transport);
r_contact* add_r_contact(str host,int port,int transport,str uri,
	enum Reg_States reg_state,int expires,str *service_route,int service_route_cnt, r_nat_dest * pinhole);
r_contact* update_r_contact(str host,int port,int transport,
	str *uri,enum Reg_States  *reg_state,int *expires,str **service_route,int *service_route_cnt, r_nat_dest ** pinhole);
r_contact* update_r_contact_sec(str host,int port,int transport,
	str *uri,enum Reg_States *reg_state,int *expires,
	r_security *s);
void del_r_contact(r_contact *c);
void free_r_contact(r_contact *c);

r_nat_dest * get_r_nat_pinhole(str host, int port, int transport);



void print_r(int log_level);



#endif //P_CSCF_REGISTRAR_STORAGE_H_
