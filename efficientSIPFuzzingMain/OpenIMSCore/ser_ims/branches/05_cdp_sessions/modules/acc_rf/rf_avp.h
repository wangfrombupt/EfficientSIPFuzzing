/**
 * $Id$
 *  
 * Copyright (C) 2004-2007 FhG Fokus
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
 
#ifndef RF_AVP_H
#define RF_AVP_H

#include "../../sr_module.h"
#include "../cdp/cdp_load.h"

/** NO DATA WILL BE DUPLICATED OR FREED - DO THAT AFTER SENDING THE MESSAGE!!! */
int Rf_add_destination_realm(AAAMessage *msg, str data);
int Rf_add_acct_application_id(AAAMessage *msg, unsigned int data);
int Rf_add_accounting_record_type(AAAMessage *msg, unsigned int data);
int Rf_add_accounting_record_number(AAAMessage *msg, unsigned int data);
int Rf_add_service_context_id(AAAMessage *msg, str data);

int Rf_add_ims_information(AAA_AVP_LIST* outl, AAA_AVP_LIST* inl);
int Rf_add_service_information(AAAMessage *msg, AAA_AVP_LIST* list);

int Rf_add_role_of_node(AAA_AVP_LIST* list, unsigned int data);
int Rf_add_node_functionality(AAA_AVP_LIST* list, unsigned int data);

int Rf_add_ims_charging_id(AAA_AVP_LIST* list, str data);

int Rf_add_sip_method(AAA_AVP_LIST* list, str data);
int Rf_add_event_type(AAA_AVP_LIST* outl, AAA_AVP_LIST* inl);
int Rf_add_event(AAA_AVP_LIST* list, str data);
int Rf_add_expires(AAA_AVP_LIST* list, unsigned int data);
int Rf_add_user_session_id(AAA_AVP_LIST* list, str data);
int Rf_add_calling_party_address(AAA_AVP_LIST* list, str data);
int Rf_add_called_party_address(AAA_AVP_LIST* list, str data);

/* GET AVPS */
str Rf_get_session_id(AAAMessage *msg);
//str Cx_get_user_name(AAAMessage *msg);
//str Cx_get_public_identity(AAAMessage *msg);
//AAA_AVP* Cx_get_next_public_identity(AAAMessage *msg,AAA_AVP* pos,int avp_code,int vendor_id,const char *func);		
//str Cx_get_visited_network_id(AAAMessage *msg);
int Rf_get_result_code(AAAMessage *msg,int *data);
//int Cx_get_experimental_result_code(AAAMessage *msg, int *data);

	
//str Cx_get_destination_host(AAAMessage *msg);	
//str Cx_get_user_data(AAAMessage *msg);	
//str Cx_get_charging_info(AAAMessage *msg);



int Rf_get_service_context_id(AAAMessage *msg, int *data);
int Rf_get_accounting_record_type(AAAMessage *msg, int *data);

//str Rf_get_public_identity(AAAMessage *msg);
//AAA_AVP* Rf_get_next_public_identity(AAAMessage *msg,AAA_AVP* pos,int avp_code,int vendor_id,const char *func);		
//str Rf_get_visited_network_id(AAAMessage *msg);
//int Rf_get_experimental_result_code(AAAMessage *msg, int *data);
	
//str Rf_get_destination_host(AAAMessage *msg);	
//str Rf_get_user_data(AAAMessage *msg);	
//str Rf_get_charging_info(AAAMessage *msg);



#endif /* RF_AVP_H */
