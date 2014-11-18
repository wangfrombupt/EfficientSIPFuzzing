/**
 * $Id: sip.h 317 2007-06-14 17:04:35Z vingarzan $
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
 * IMS Service Control - SIP Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#ifndef _ISC_SIP_H
#define _ISC_SIP_H

#include "../../data_lump_rpl.h"
//#include "../../fifo_server.h"
#include "../../parser/contact/parse_contact.h"
#include "../../parser/parse_uri.h"
#include "../../parser/parse_from.h"
#include "../../parser/parse_content.h"
#include "../../parser/parse_disposition.h"
#include "../../db/db.h"
#include "../tm/tm_load.h"

int str2cmp(str x,str y);

int str2icmp(str x,str y);

int isc_is_initial_request(struct sip_msg *msg);
int isc_is_register(struct sip_msg *msg);
	
int isc_get_originating_user( struct sip_msg * msg, str *uri );

int isc_is_registered(str *uri);

inline int isc_get_terminating_type(str *uri);

int isc_get_terminating_user( struct sip_msg * msg, str *uri );
	
int isc_get_expires(struct sip_msg *msg);

int cscf_get_transaction(struct sip_msg *msg, unsigned int *hash,unsigned int *label);

struct sip_msg* cscf_get_request_from_reply(struct sip_msg *reply);

// from scscf
int cscf_get_expires_hdr(struct sip_msg *msg);
int cscf_get_max_expires(struct sip_msg *msg);
contact_body_t *cscf_parse_contacts(struct sip_msg *msg);
str cscf_get_public_identity(struct sip_msg *msg);

// from pcscf
int cscf_get_first_p_associated_uri(struct sip_msg *msg,str *public_id);
str cscf_get_access_network_info(struct sip_msg *msg, struct hdr_field **h);
str cscf_get_visited_network_id(struct sip_msg *msg, struct hdr_field **h);
str cscf_get_charging_vector(struct sip_msg *msg, struct hdr_field **h);

str cscf_get_call_id(struct sip_msg *msg,struct hdr_field **hr);
int cscf_get_cseq(struct sip_msg *msg,struct hdr_field **hr);
#endif


