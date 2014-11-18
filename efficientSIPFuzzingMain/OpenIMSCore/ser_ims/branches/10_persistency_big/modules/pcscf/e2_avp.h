/*
 * $Id: e2_avp.h 299 2007-05-31 18:19:30Z vingarzan $
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
 * P-CSCF Module - Cx AVP Operations  
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 * Copyright (C) 2005 FhG Fokus
 * 		
 */
 
#ifndef P_CSCF_e2_AVP_H
#define P_CSCF_e2_AVP_H

#include "../../sr_module.h"
#include "mod.h"
#include "../cdp/cdp_load.h"


/** NO DATA WILL BE DUPLICATED OR FREED - DO THAT AFTER SENDING THE MESSAGE!!! */

int e2_add_user_name(AAAMessage *uar,str private_identity);
int e2_add_public_identity(AAAMessage *msg,str data);
int e2_add_authorization_type(AAAMessage *msg,unsigned int data);
int e2_add_server_name(AAAMessage *msg,str data);
int e2_add_result_code(AAAMessage *msg,unsigned int data);
int e2_add_experimental_result_code(AAAMessage *msg,unsigned int data);
int e2_add_vendor_specific_appid(AAAMessage *msg,unsigned int vendor_id,	unsigned int auth_id,unsigned int acct_id);
int e2_add_auth_session_state(AAAMessage *msg,unsigned int data);	
int e2_add_destination_realm(AAAMessage *msg,str data);
int e2_add_g_unique_address(AAAMessage *msg, str ip,str realm);
int e2_add_app_identifier(AAAMessage *msg, str data); 

/* GET AVPS */
inline str e2_get_user_name(AAAMessage *msg);
inline str e2_get_terminal_type(AAAMessage *msg);
int e2_get_access_net(AAAMessage *msg, int *data);
int e2_get_location_info(AAAMessage *msg, str *data);
#endif /* P_CSCF_e2_AVP_H */
