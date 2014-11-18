/**
 * $Id: mod.h 430 2007-08-01 13:18:42Z vingarzan $
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
 * \dir modules/pcscf
 *
 * This is the Proxy-CSCF module. For general documentation, look at \ref PCSCF.
 * 
 */
 
/**
 * \file modules/pcscf/Makefile 
 * Proxy-CSCF SER module Makefile
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/**
 * \file modules/pcscf/reginfo.dtd 
 * Proxy-CSCF SER module reginfo/xml DTD file for checking the payload of NOTIFY to reg
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/** 
 * \file modules/pcscf/ipsec_E_Drop.sh
 * Proxy-CSCF IPSec drop all Security-Associations on the UserEndpoint
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/** 
 * \file modules/pcscf/ipsec_E_Inc_Req.sh
 * Proxy-CSCF IPSec create Security-Association on the UserEndpoint for Incoming Requests
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/** 
 * \file modules/pcscf/ipsec_E_Inc_Rpl.sh
 * Proxy-CSCF IPSec create Security-Association on the UserEndpoint for Incoming Replies
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/** 
 * \file modules/pcscf/ipsec_E_Out_Req.sh
 * Proxy-CSCF IPSec create Security-Association on the UserEndpoint for Outgoing Requests
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/**
 * \file modules/pcscf/ipsec_E_Out_Rpl.sh
 * Proxy-CSCF IPSec create Security-Association on the UserEndpoint for Outgoing Replies
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/** 
 * \file modules/pcscf/ipsec_P_Drop.sh
 * Proxy-CSCF IPSec drop all Security-Associations on the Proxy-CSCF
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/** 
 * \file modules/pcscf/ipsec_P_Inc_Req.sh
 * Proxy-CSCF IPSec create Security-Association on the Proxy-CSCF for Incoming Requests
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/** 
 * \file modules/pcscf/ipsec_P_Inc_Rpl.sh
 * Proxy-CSCF IPSec create Security-Association on the Proxy-CSCF for Incoming Replies
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/** 
 * \file modules/pcscf/ipsec_P_Out_Req.sh
 * Proxy-CSCF IPSec create Security-Association on the Proxy-CSCF for Outgoing Requests
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
/**
 * \file modules/pcscf/ipsec_P_Out_Rpl.sh
 * Proxy-CSCF IPSec create Security-Association on the Proxy-CSCF for Outgoing Replies
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */ 
  
 
/** 
 * 
 * \page PCSCF The Proxy-CSCF Module (pcscf)
 *  \b Module \b Documentation
 *
 * [\subpage pcscf_overview]
 * [\subpage pcscf_code]
 * [\subpage pcscf_config]
 * [\subpage pcscf_example]
 * 
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de 
 * 
 * \section pcscf_overview Overview
 * 
 * This module is supposed to provide the functionality required for an Proxy-CSCF
 * 
 * 
 * \section pcscf_code Code Structure
 * 
 * \section pcscf_config Configuration and usage
 * 
 * For exported functions look at #pcscf_cmds.\n
 * For configuration parameters look at #pcscf_params.\n 
 * 
 *  
 * \section pcscf_example Example
 * And here is a real usage example:
 * - Proxy-CSCF configuration file \include pcscf.cfg
 * 
 */
 
/**
 * \file
 * 
 * Proxy-CSCF - SER module interface
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#ifndef P_CSCF_MOD_H
#define P_CSCF_MOD_H

#include "../../sr_module.h"

#define M_NAME "P-CSCF"

/** Return and break the execution of routng script */
#define CSCF_RETURN_BREAK	0 
/** Return true in the routing script */
#define CSCF_RETURN_TRUE	1
/** Return false in the routing script */
#define CSCF_RETURN_FALSE -1
/** Return error in the routing script */
#define CSCF_RETURN_ERROR -2

int P_trans_in_processing(struct sip_msg* msg, char* str1, char* str2);

#define STR_SHM_DUP(dest,src,txt)\
{\
	if ((src).len==0) {\
		(dest).s=0;\
		(dest).len=0;\
	}else {\
		(dest).s = shm_malloc((src).len);\
		if (!(dest).s){\
			LOG(L_ERR,"ERR:"M_NAME":"txt": Error allocating %d bytes\n",(src).len);\
			(dest).len = 0;\
			goto out_of_memory;\
		}else{\
			(dest).len = (src).len;\
			memcpy((dest).s,(src).s,(src).len);\
		}\
	}\
}

#define STR_PKG_DUP(dest,src,txt)\
{\
	if ((src).len==0) {\
		(dest).s=0;\
		(dest).len=0;\
	}else {\
		(dest).s = pkg_malloc((src).len);\
		if (!(dest).s){\
			LOG(L_ERR,"ERRL:"M_NAME":"txt": Error allocating %d bytes\n",(src).len);\
			(dest).len = 0;\
			goto out_of_memory;\
		}else{\
			(dest).len = (src).len;\
			memcpy((dest).s,(src).s,(src).len);\
		}\
	}\
}

#define STR_APPEND(dst,src)\
	{memcpy((dst).s+(dst).len,(src).s,(src).len);\
	(dst).len = (dst).len + (src).len;}

/* ANSI Terminal colors */
#define ANSI_GRAY		"\033[01;30m"
#define ANSI_BLINK_RED 	"\033[00;31m"
#define ANSI_RED 		"\033[01;31m"
#define ANSI_GREEN		"\033[01;32m"
#define ANSI_YELLOW 	"\033[01;33m"
#define ANSI_BLUE 		"\033[01;34m"
#define ANSI_MAGENTA	"\033[01;35m"
#define ANSI_CYAN		"\033[01;36m"
#define ANSI_WHITE		"\033[01;37m"

unsigned long (* get_tls_session_hash)(struct sip_msg *msg);

#endif /* P_CSCF_MOD_H */
