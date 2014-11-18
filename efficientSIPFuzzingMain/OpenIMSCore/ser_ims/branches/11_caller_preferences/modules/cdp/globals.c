/**
 * $Id: globals.c 420 2007-07-26 14:38:12Z vingarzan $
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
 * CDiameterPeer Global functions
 * 
 * These are only usefull if you use the CDiameterPeer without SER
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "globals.h"
#include "utils.h"

#ifdef CDP_FOR_SER

#else
	unsigned long shm_mem_size = SHM_MEM_SIZE;
	int memlog = L_ERR;
	int memdbg = L_MEM;
	int debug = L_MEM;
	int log_facility = 1;
	int log_stderr = 1;
	int process_no=0;
#endif



//str aaa_fqdn={"unset_fqdn",10};
//str aaa_realm={"unset_realm",11};
//str aaa_identity={"unset_identity",14};

/** initialized the pkg and shm memory */
int init_memory(int show_status)
{
#ifdef PKG_MALLOC
	if (init_pkg_mallocs()==-1)
		goto error;
	if (show_status){
		LOG(memlog, "Memory status (pkg):\n");
		pkg_status();
	}
#endif

#ifdef SHM_MEM
	if (init_shm_mallocs()==-1)
		goto error;
	if (show_status){
		LOG(memlog, "Memory status (shm):\n");
		shm_status();
	}
#endif
	return 1;
error:
	return 0;
}	

/** call it before exiting; if show_status==1, mem status is displayed */
void destroy_memory(int show_status)
{
	/*clean-up*/
	if (mem_lock)
	    shm_unlock(); /* hack: force-unlock the shared memory lock in case
	                             some process crashed and let it locked; this will
	                             allow an almost gracious shutdown */
#ifdef SHM_MEM
	if (show_status){
		LOG(memlog, "Memory status (shm):\n");
		//shm_status();
		shm_sums();
	}
	/* zero all shmem alloc vars that we still use */
	shm_mem_destroy();
#endif
#ifdef PKG_MALLOC
	if (show_status){
		LOG(memlog, "Memory status (pkg):\n");
		//pkg_status();
		pkg_sums();
	}
#endif
}


