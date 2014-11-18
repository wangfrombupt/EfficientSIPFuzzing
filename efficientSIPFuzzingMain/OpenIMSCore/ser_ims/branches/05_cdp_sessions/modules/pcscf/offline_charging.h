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
#ifndef CSCF_OFFLINE_CHARGING_H
#define CSCF_OFFLINE_CHARGING_H

#include "../../sr_module.h"
#include "../cdp/cdp_load.h"

struct offline_charging_flag {
	unsigned int cf_ietf; 
	unsigned int cf_3gpp; 
};

/* avps may be created see 3GPP TS32.260 v7.2.0 table 6.3.2.1 */
#define CF_IETF_SESSION_ID						0x00000001
#define CF_IETF_ORIGIN_HOST						0x00000002
#define CF_IETF_ORIGIN_REALM					0x00000004
#define CF_IETF_DESTINATION_REALM				0x00000008
#define CF_IETF_ACCOUNTING_RECORD_TYPE			0x00000010
#define CF_IETF_ACCOUNTING_RECORD_NUMBER		0x00000020
#define CF_IETF_ACCT_APPLICATION_ID				0x00000040
#define CF_IETF_USER_NAME						0x00000080
#define CF_IETF_ACCT_INTERIM_INTERVAL			0x00000100
#define CF_IETF_ORIGIN_STATE_ID					0x00000200
#define CF_IETF_EVENT_TIME_STAMP				0x00000400
#define CF_IETF_PROXY_INFO						0x00000800
#define CF_IETF_ROUTE_RECORD					0x00001000
#define CF_IETF_SERVICE_CONTEXT_ID				0x00002000	

#define CF_3GPP_EVENT_TYPE						0x00000001
#define CF_3GPP_ROLE_OF_NODE					0x00000002
#define CF_3GPP_NODE_FUNCTIONALITY				0x00000004
#define CF_3GPP_USER_SESSION_ID					0x00000008
#define CF_3GPP_CALLING_PARTY_ADDRESS   		0x00000010
#define CF_3GPP_CALLED_PARTY_ADDRESS			0x00000020
#define CF_3GPP_REQUESTED_PARTY_ADDRESS			0x00000040
#define CF_3GPP_CALLED_ASSERTED_IDENTITIY		0x00000080
#define CF_3GPP_ASSOCIATED_URI					0x00000100
#define CF_3GPP_TIME_STAMPS						0x00000200
#define CF_3GPP_APPLICATION_SERVER_INFORMATION	0x00000400
#define CF_3GPP_INTER_OPERATOR_IDENTIFIER		0x00000800
#define CF_3GPP_IMS_CHARGING_IDENTIFIER			0x00001000
#define CF_3GPP_SDP_SESSION_DESCRIPTION			0x00002000
#define CF_3GPP_SDP_MEDIA_COMPONENT				0x00004000
#define CF_3GPP_GGSN_ADDRESS					0x00008000
#define CF_3GPP_SERVED_PARTY_IP_ADDRESS			0x00010000
#define CF_3GPP_AUTHORIZED_QOS					0x00020000
#define CF_3GPP_SERVER_CAPABILITIES				0x00040000
#define CF_3GPP_TRUNK_GROUP_ID					0x00080000
#define CF_3GPP_BEARER_SERVICE					0x00100000
#define CF_3GPP_SERVICE_ID						0x00200000
#define CF_3GPP_SERVICE_SPECIFIC_INFO			0x00400000
#define CF_3GPP_MESSAGE_BODIES					0x00800000
#define CF_3GPP_CAUSE_CODE						0x01000000
#define CF_3GPP_ACCESS_NETWORK_INFORMATION		0x02000000

#define CF_IETF_DEFAULT CF_IETF_SESSION_ID| \
						CF_IETF_ORIGIN_HOST| \
						CF_IETF_EVENT_TIME_STAMP

#define CF_3GPP_DEFAULT CF_3GPP_EVENT_TYPE| \
						CF_3GPP_ROLE_OF_NODE| \
						CF_3GPP_NODE_FUNCTIONALITY| \
						CF_3GPP_USER_SESSION_ID| \
						CF_3GPP_CALLING_PARTY_ADDRESS| \
						CF_3GPP_CALLED_PARTY_ADDRESS| \
						CF_3GPP_ASSOCIATED_URI| \
						CF_3GPP_INTER_OPERATOR_IDENTIFIER| \
						CF_3GPP_IMS_CHARGING_IDENTIFIER| \
						CF_3GPP_ACCESS_NETWORK_INFORMATION| \
						CF_3GPP_TIME_STAMPS
						  
						
int P_ACR_event(struct sip_msg*, char*, char*);
int P_ACR_start(struct sip_msg*, char*, char*);
int P_ACR_interim(struct sip_msg*, char*, char*);
int P_ACR_stop(struct sip_msg*, char*, char*);

int P_ACA_event(struct sip_msg* msg, AAAMessage* aca);
int P_ACA_start(struct sip_msg* msg, AAAMessage* aca);
int P_ACA_interim(struct sip_msg* msg, AAAMessage* aca);
int P_ACA_stop(struct sip_msg* msg, AAAMessage* aca);

#endif /*CSCF_OFFLINE_CHARGING_H*/
