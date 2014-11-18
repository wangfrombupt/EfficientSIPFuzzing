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
 * \dir modules/isc
 *
 * This is the IMS Service Control module. For general documentation, look at \ref ISC
 * 
 */
 
 /**
  *  \file modules/isc/Makefile 
  * IMS Service Control SER module Makefile
  * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
  */
 
/** \page ISC The IMS Service Control Module (isc)
 *  \b Module \b Documentation
 * 
 * [\subpage isc_overview]
 * [\subpage isc_code]
 * [\subpage isc_config]
 * [\subpage isc_example]
 * 
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *  
 * \section isc_overview Overview
 * 
 * This module is supposed to provide support for the ISC interface between the 
 * Serving-CSCF and the Aplication Servers 
 * 
 * To use you need the \ref SCSCF loaded. This is because this module uses the registrar
 * in there for Initial Filter Criteria storage.
 * 
 * 
 * \section isc_code Code Structure
 * The functionality exported to the SER routing script is defined in mod.c . The IFC
 * checking logic is in checker.c. isc.c defines the ISC interface processing and mark.c 
 * the message marking mechanism. For generic SIP processing functions look into sip.c.
 * 
 * \section isc_config Configuration and usage
 * 
 * For exported functions look at #isc_cmds.\n
 * For configuration parameters look at #isc_params.\n 
 * 
 * 
 * The messages forwarded to the Application Server have the following marking:
\code
Route: <AS>, <sip:ifcmark@[isc_my_uri];lr;s=xxx;h=xxx;d=xxx>
\endcode
 * The message is "loose-routed" to the AS. If the AS responds with a request, the 2nd
 * route header value will contain all the required state information for identification 
 * and IFC matching resume.
 * 
 * For detailed information on marking take a look at mark.c.
 * 
 * 
 * To check if a message is matching the next unchecked IFC you should do the following for
 * checking triggers for the originating user:
 * \code
{  
    ...
	if (ISC_match_filter("orig")){
		t_on_reply("ISC_Orig_reply");
        t_on_failure("ISC_Orig_failure");
		# here the message matched and was forwarded to the AS already and we can exit
        exit;
    }
    # here the message did not match any (more) IFCs
    ...
}    
 \endcode
 * Then to catch a response:		  
 \code
onreply_route[ISC_Orig_reply]
{
    log(1,">>       ISC_Orig_reply\n");
    #if you would like to do some processing on responses from AS, do it here
    break;
}

failure_route[ISC_Orig_failure]
{
    log(1,">>       ISC_Orig_failure\n");
    if (!t_check_status("408")){ 
        #if you would like to do some processing on responses from AS, do it here   
        break;
    }
    if (ISC_is_session_continued()){
		#here the AS failed to respond and we should continue processing 
        break;      
    }else{
    	#here the AS failed to respond and the session should be terminated
        t_reply("555","AS failed to respond");
    }
}
\endcode
 *
 * Or, if the AS responded with the request modified, or with another request and
 * it wishes to resume the IFC triggering session, the following should be applied to
 * identify this case:
\code
	...
	if (ISC_from_AS("orig")){
		#do resume of processing - probably call ISC_match_filter("orig")
	}
	...
\endcode	
 * 
 * 
 * 
 *  
 * \section isc_example Example
 * And here is a real usage example:
 * - Serving-CSCF configuration file with ISC support \include scscf.cfg
 * 

 */
 
/**
 * \file
 * 
 * IMS Service Control - SER module interface
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#ifndef _ISC__H
#define _ISC__H

#include "../../sr_module.h"

#define M_NAME "ISC"

/** SER routing script return and break execution */
#define ISC_RETURN_BREAK	 0
/** SER routing script return true */
#define ISC_RETURN_TRUE		 1
/** SER routing script return false */
#define ISC_RETURN_FALSE	-1
/** SER routing script return error */
#define ISC_RETURN_ERROR 	-2

/** Message was forwarded to AS */
#define ISC_MSG_FORWARDED 0
/** Message was not forwarded to AS */
#define ISC_MSG_NOT_FORWARDED 1
/** Message is not a request */
#define ISC_NOT_A_REQUEST 2

/* Various constants */
/** User Not Registered */
#define IMS_USER_NOT_REGISTERED 0
/** User registered */
#define IMS_USER_REGISTERED 1
/** User unregistered (not registered but with services for unregistered state) */
#define IMS_USER_UNREGISTERED -1

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

/** Direction of the dialog */
enum dialog_direction {
	DLG_MOBILE_ORIGINATING=0,	/** Originating */
	DLG_MOBILE_TERMINATING=1,	/** Terminating */
	DLG_MOBILE_UNKNOWN=2		/** Unknown 	*/
};

#endif
