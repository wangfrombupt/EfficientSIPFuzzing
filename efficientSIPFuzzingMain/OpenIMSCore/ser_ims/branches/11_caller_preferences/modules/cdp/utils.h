/** 
 * $Id: utils.h 139 2007-02-13 20:27:02Z vingarzan $
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
 * CDiameterPeer Includes
 * 
 * This header is used to switch between using the CDiameterPeer from SER
 * or as a standalone server.
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#ifndef __CDP_UTILS_H_
#define __CDP_UTILS_H_

#ifdef CDP_FOR_SER

#include "../../dprint.h"
#include "../../str.h"
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../locking.h"
#include "../../pt.h"

#else

#include "../utils/config.h"
#include "../utils/dprint.h"
#include "../utils/str.h"
#include "../utils/mem.h"
#include "../utils/shm_mem.h"
#include "../utils/locking.h"

#endif


#define LOG_NO_MEM(mem_type,data_len) \
	LOG(L_ERR,"ERROR:%s:%s()[%d]: Out of %s memory allocating %d bytes\n",\
		__FILE__,__FUNCTION__,__LINE__, \
		mem_type,data_len);

#define shm_str_dup(dst,src)\
{\
	(dst).s = shm_malloc((src).len+1);\
	if (!(dst).s){LOG_NO_MEM("shm",(src).len+1);}\
	else {memcpy((dst).s,(src).s,(src).len);(dst).s[(src).len]=0;(dst).len=(src).len;}\
}

#endif /*UTILS_H_*/
