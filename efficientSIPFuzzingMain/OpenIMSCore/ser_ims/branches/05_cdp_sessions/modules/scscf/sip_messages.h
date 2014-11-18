/*
 * $Id: sip_messages.h 2 2006-11-14 22:37:20Z vingarzan $
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
 * Serving-CSCF - Response Messages Reason Phrases
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
  
#ifndef S_CSCF_SIP_MESSAGES_H_
#define S_CSCF_SIP_MESSAGES_H_


#define MSG_200_SAR_OK					"OK - SAR succesful and registrar saved"

#define MSG_401_CHALLENGE				"Unauthorized - Challenging the UE"

#define MSG_403_NO_PRIVATE				"Forbidden - Private identity not found (Authorization: username)"
#define MSG_403_NO_PUBLIC				"Forbidden - Public identity not found (To:)"
#define MSG_403_NO_NONCE				"Forbidden - Nonce not found (Authorization: nonce)"
#define MSG_403_UNKOWN_RC				"Forbidden - HSS responded with unknown Result Code"
#define MSG_403_UNKOWN_EXPERIMENTAL_RC	"Forbidden - HSS responded with unknown Experimental Result Code"
#define MSG_403_USER_UNKNOWN			"Forbidden - HSS User Unknown"
#define MSG_403_IDENTITIES_DONT_MATCH	"Forbidden - HSS Identities don't match"
#define MSG_403_AUTH_SCHEME_UNSOPPORTED "Forbidden - HSS Authentication Scheme Unsupported"
#define MSG_403_UNABLE_TO_COMPLY		"Forbidden - HSS Unable to comply"
#define MSG_403_NO_AUTH_DATA			"Forbidden - HSS returned no authentication vectors"

#define MSG_423_INTERVAL_TOO_BRIEF 		"Interval too brief"

#define MSG_480_HSS_ERROR 				"Temporarily unavailable - error retrieving av"
#define MSG_480_DIAMETER_ERROR			"Temporarily Unavailable - Diameter Cx interface failed"
#define MSG_480_DIAMETER_TIMEOUT		"Temporarily unavailable - TimeOut in MAR/A HSS"
#define MSG_480_DIAMETER_TIMEOUT_SAR	"Temporarily unavailable - TimeOut in SAR/A HSS"
#define MSG_480_DIAMETER_MISSING_AVP	"Temporarily unavailable - Missing AVP in UAA from HSS"

#define MSG_500_PACK_AV					"Server Internal Error - while packing auth vectors"
#define MSG_500_SAR_FAILED				"Server Internal Error - Server Assignment failed"
#define MSG_500_UPDATE_CONTACTS_FAILED	"Server Internal Error - Update Contacts failed"

#endif //S_CSCF_SIP_MESSAGES_H_
