/*
 * $Id: registrar.h 398 2007-07-20 16:37:54Z placido $
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
 * Serving-CSCF - Registrar-Related Operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

  
#ifndef S_CSCF_REGISTRAR_H_
#define S_CSCF_REGISTRAR_H_

#include "../../sr_module.h"

/** User Not Registered */
#define IMS_USER_NOT_REGISTERED 0
/** User registered */
#define IMS_USER_REGISTERED 1
/** User unregistered (not registered but with services for unregistered state) */
#define IMS_USER_UNREGISTERED -1
/** User de-registered */
#define IMS_USER_DEREGISTERED -2

void registrar_timer(unsigned int ticks, void* param);

int S_assign_server(struct sip_msg *msg,char *str1,char *str2 );

int S_assign_server_unreg(struct sip_msg *msg,char *str1,char *str2 );

int S_update_contacts(struct sip_msg *msg,char *str1,char *str2);

int SAR(struct sip_msg *msg, str realm,str public_identity, str private_identity,
				int assignment_type,int data_available);

int save_location(struct sip_msg *msg,int assignment_type,str *xml,str *ccf1,str *ccf2,str *ecf1,str *ecf2);


int S_lookup(struct sip_msg *msg,char *str1,char *str2);


int r_is_registered_id(str public_identity);
int r_is_not_registered_id(str public_identity);
int r_is_unregistered_id(str public_identity);

int S_is_not_registered(struct sip_msg *msg,char *str1,char *str2);

int S_term_registered(struct sip_msg *msg,char *str1,char *str2);
int S_term_not_registered(struct sip_msg *msg,char *str1,char *str2);
int S_term_unregistered(struct sip_msg *msg,char *str1,char *str2);


int S_orig_registered(struct sip_msg *msg,char *str1,char *str2);
int S_orig_not_registered(struct sip_msg *msg,char *str1,char *str2);
int S_orig_unregistered(struct sip_msg *msg,char *str1,char *str2);



int S_mobile_originating(struct sip_msg *msg,char *str1,char *str2);

int S_originating_barred(struct sip_msg *msg,char *str1,char *str2);

int S_terminating_barred(struct sip_msg *msg,char *str1,char *str2);

int S_add_p_asserted_identity(struct sip_msg *msg,char *str1,char *str2);

#endif //S_CSCF_REGISTRAR_H_
