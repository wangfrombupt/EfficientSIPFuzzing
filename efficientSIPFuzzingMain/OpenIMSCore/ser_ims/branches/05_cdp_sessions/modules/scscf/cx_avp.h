/*
 * $Id: cx_avp.h 220 2007-04-05 19:26:00Z vingarzan $
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
 * I/S-CSCF Module - Cx AVP Operations  
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 * Copyright (C) 2005 FhG Fokus
 * 		
 */
 
#ifndef IS_CSCF_CX_AVP_H
#define IS_CSCF_CX_AVP_H

#include "../../sr_module.h"
#include "mod.h"
#include "../cdp/cdp_load.h"


/** NO DATA WILL BE DUPLICATED OR FREED - DO THAT AFTER SENDING THE MESSAGE!!! */

int Cx_add_user_name(AAAMessage *uar,str private_identity);
int Cx_add_public_identity(AAAMessage *msg,str data);
int Cx_add_visited_network_id(AAAMessage *msg,str data);
int Cx_add_authorization_type(AAAMessage *msg,unsigned int data);

int Cx_add_server_name(AAAMessage *msg,str data);
int Cx_add_sip_number_auth_items(AAAMessage *msg,unsigned int data);
int Cx_add_sip_auth_data_item_request(AAAMessage *msg,str auth_scheme,str auth);
int Cx_add_server_assignment_type(AAAMessage *msg,unsigned int data);
int Cx_add_userdata_available(AAAMessage *msg,unsigned int data);
int Cx_add_result_code(AAAMessage *msg,unsigned int data);
int Cx_add_experimental_result_code(AAAMessage *msg,unsigned int data);
int Cx_add_vendor_specific_appid(AAAMessage *msg,unsigned int vendor_id,
	unsigned int auth_id,unsigned int acct_id);
int Cx_add_auth_session_state(AAAMessage *msg,unsigned int data);	
int Cx_add_destination_realm(AAAMessage *msg,str data);

/* GET AVPS */

str Cx_get_session_id(AAAMessage *msg);
str Cx_get_user_name(AAAMessage *msg);
str Cx_get_public_identity(AAAMessage *msg);
AAA_AVP* Cx_get_next_public_identity(AAAMessage *msg,AAA_AVP* pos,int avp_code,int vendor_id,const char *func);		
str Cx_get_visited_network_id(AAAMessage *msg);
int Cx_get_authorization_type(AAAMessage *msg, int *data);
int Cx_get_server_assignment_type(AAAMessage *msg, int *data);
int Cx_get_userdata_available(AAAMessage *msg, int *data);
int Cx_get_result_code(AAAMessage *msg,int *data);
int Cx_get_experimental_result_code(AAAMessage *msg, int *data);
str Cx_get_server_name(AAAMessage *msg);
int Cx_get_capabilities(AAAMessage *msg,int **m,int *m_cnt,int **o,int *o_cnt);
int Cx_get_sip_number_auth_items(AAAMessage *msg,int *data);
int Cx_get_auth_data_item_request(AAAMessage *msg,
		 str *auth_scheme, str *authorization);
int Cx_get_auth_data_item_answer(AAAMessage *msg, AAA_AVP **auth_data,
	int *item_number,str *auth_scheme,str *authenticate,str *authorization,
	str *ck,str *ik,str *ip,str *ha1);
	
str Cx_get_destination_host(AAAMessage *msg);	
str Cx_get_user_data(AAAMessage *msg);	
str Cx_get_charging_info(AAAMessage *msg);

#endif /* IS_CSCF_CX_AVP_H */
