/**
 * $Id: location.h 423 2007-07-29 13:21:41Z vingarzan $
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
 * Interrogating-CSCF - Location Information Operations LIR/LIA
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#ifndef I_CSCF_LOCATION_H
#define I_CSCF_LOCATION_H

#include "../../sr_module.h"

#include "../cdp/cdp_load.h"
 
#include "scscf_list.h"
 
#define MSG_400 "Bad Request" 

#define MSG_403 "Forbidden"
#define MSG_403_UNKOWN_RC "Forbidden - HSS responded with unknown Result Code"
#define MSG_403_UNKOWN_EXPERIMENTAL_RC "Forbidden - HSS responded with unknown Experimental Result Code"

#define MSG_404_NOT_REGISTERED "Not Found - HSS Identity not registered"
#define MSG_480_NOT_REGISTERED "Temporarily Unavailable - HSS Identity not registered"

#define MSG_480_DIAMETER_ERROR "Temporarily Unavailable - Diameter Cx interface failed"
#define MSG_480_DIAMETER_TIMEOUT_LIA "Temporarily unavailable - TimeOut in LIR/A HSS"
#define MSG_480_DIAMETER_MISSING_AVP_LIA "Temporarily unavailable - Missing AVP in LIA from HSS"


#define MSG_604_USER_UNKNOWN "Does not exist anywhere - HSS User Unknown"

int I_LIR(struct sip_msg* msg, char* str1, char* str2);

int I_LIA(struct sip_msg* msg, AAAMessage** lia, int originating);

int I_originating(struct sip_msg *msg, char *str1, char *str2);

#endif /* I_CSCF_LOCATION_H */
