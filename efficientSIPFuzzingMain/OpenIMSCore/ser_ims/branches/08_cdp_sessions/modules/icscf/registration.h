/**
 * $Id: registration.h 2 2006-11-14 22:37:20Z vingarzan $
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
 * Interrogating-CSCF - User-Authorization Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#ifndef I_CSCF_REGISTRATION_H
#define I_CSCF_REGISTRATION_H

#include "../../sr_module.h"

#include "../cdp/cdp_load.h"

#include "scscf_list.h"
 
#define MSG_400 "Bad Request" 
#define MSG_400_NO_PRIVATE "Bad Request - Private ID in Authorization / username token missing" 
#define MSG_400_NO_PUBLIC "Bad Request - Public ID in To header missing" 
#define MSG_400_NO_VISITED "Bad Request - P-Visited-Network-ID header missing" 

#define MSG_403_UNKOWN_RC "Forbidden - HSS responded with unknown Result Code"
#define MSG_403_UNKOWN_EXPERIMENTAL_RC "Forbidden - HSS responded with unknown Experimental Result Code"
#define MSG_403_USER_UNKNOWN "Forbidden - HSS User Unknown"
#define MSG_403_IDENTITIES_DONT_MATCH "Forbidden - HSS Identities don't match"
#define MSG_403_AUTHORIZATION_REJECTED "Forbidden - HSS Authorization Rejected"
#define MSG_403_ROAMING_NOT_ALLOWED "Forbidden - HSS Roaming not allowed"
#define MSG_403_IDENTITY_NOT_REGISTERED "Forbidden - HSS Identity not registered"
#define MSG_403_UNABLE_TO_COMPLY "Forbiddent - Hss Unable to comply"

#define MSG_480_DIAMETER_ERROR "Temporarily Unavailable - Diameter Cx interface failed"
#define MSG_480_DIAMETER_TIMEOUT "Temporarily unavailable - TimeOut in UAR/A HSS"
#define MSG_480_DIAMETER_MISSING_AVP "Temporarily unavailable - Missing AVP in UAA from HSS"

#define MSG_500_ERROR_SAVING_LIST "Server Error while saving S-CSCF list on I-CSCF"

#define MSG_600_FORWARDING_FAILED "Busy everywhere - Forwarding to S-CSCF failed"

#define MSG_600_EMPTY_LIST "Busy everywhere - Empty list of S-CSCFs"
#define MSG_600_EMPTY_SCSCF_NAME "Busy everywhere - Empty S-CSCF name as received in UAA"



int I_UAR(struct sip_msg* msg, char* str1, char* str2);

int I_UAA(struct sip_msg* msg, AAAMessage* uaa);


#endif /* I_CSCF_REGISTRATION_H */
