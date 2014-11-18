/**
 * $Id: mod.h 155 2007-02-27 10:22:27Z vingarzan $
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
 * \dir modules/icscf
 *
 * This is the Interrogating-CSCF module. For general documentation, look at \ref ICSCF
 * 
 */
 
 /**
  *  \file modules/icscf/Makefile 
  * Interrogating-CSCF SER module Makefile
  * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
  */
 
/** 
 * \page ICSCF The Interrogating-CSCF Module (icscf)
 *  \b Module \b Documentation
 *
 *
 * [\subpage icscf_overview]
 * [\subpage icscf_code]
 * [\subpage icscf_db] 
 * [\subpage icscf_config]
 * [\subpage icscf_thig] 
 * [\subpage icscf_example]
 * 
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de 
 * \section icscf_overview Overview
 * 
 * This module is supposed to provide the functionality required for an Interrogating-CSCF
 * 
 * To use it you need the \ref CDP loaded. This is because this module communicates using
 * Diameter with the Home Subscriber Server over the #IMS_Cx inteface.
 * 
 * 
 * \section icscf_code Code Structure
 * 
 * \section icscf_db Database Prerequisites
 * 
 * There are several tables that need to be provisioned in a database, in order for the
 * Interrogating-CSCF to function properly. Here you have a MySQL dump as example:
 * \include icscf.sql
 * 
 * \section icscf_config Configuration and Usage
 * 
 * For exported functions look at #icscf_cmds.\n
 * For configuration parameters look at #icscf_params.\n 
 * 
 * \section icscf_thig Topology Hidding (THIG)
 * Provides enhanced security for message transmission
 * by performing encryption on sensitive headers.\n
 * For more information check \ref THIG
 * 
 * \section icscf_example Example
 * And here is a real usage example:
 * - Interrogating-CSCF configuration file \include icscf.cfg
 * 
 * 
 */
 
/**
 * \file
 * 
 * Interrogating-CSCF - SER module interface
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#ifndef I_CSCF_MOD_H
#define I_CSCF_MOD_H

#define M_NAME "I-CSCF"

#include "../../sr_module.h"

/** Return and break the execution of routng script */
#define CSCF_RETURN_BREAK	0 
/** Return true in the routing script */
#define CSCF_RETURN_TRUE	1
/** Return false in the routing script */
#define CSCF_RETURN_FALSE -1
/** Return error in the routing script */
#define CSCF_RETURN_ERROR -2


#define STR_SHM_DUP(dest,src,txt)\
{\
	(dest).s = shm_malloc((src).len);\
	if (!(dest).s){\
		LOG(L_ERR,"ERRL:"M_NAME":"txt": Error allocating %d bytes\n",(src).len);\
	}\
	(dest).len = (src).len;\
	memcpy((dest).s,(src).s,(src).len);\
}

#define STR_PKG_DUP(dest,src,txt)\
{\
	(dest).s = pkg_malloc((src).len);\
	if (!(dest).s){\
		LOG(L_ERR,"ERRL:"M_NAME":"txt": Error allocating %d bytes\n",(src).len);\
	}\
	(dest).len = (src).len;\
	memcpy((dest).s,(src).s,(src).len);\
}


#define STR_APPEND(dst,src)\
	{memcpy((dst).s+(dst).len,(src).s,(src).len);\
	(dst).len = (dst).len + (src).len;}



int I_add_p_charging_vector(struct sip_msg *msg,char *str1,char*str2);


#endif /* I_CSCF_MOD_H */
