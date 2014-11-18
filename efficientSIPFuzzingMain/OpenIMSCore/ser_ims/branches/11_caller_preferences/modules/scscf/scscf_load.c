/*
 * $Id: scscf_load.c 78 2007-01-02 14:09:37Z vingarzan $
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
 * Serving-CSCF - Loads the S-CSCF bindings - interface with other modules
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */



#include "scscf_load.h"

#define LOAD_ERROR "ERROR: scscf_bind: S-CSCF module function "


/**
 * Load the bindings to the S-CSCF module.
 * @param scscfb - binding to load into
 * returns 1 on success, -1 on error
 */
int load_scscf( struct scscf_binds *scscfb)
{
	if (!( scscfb->get_r_public=(get_r_public_f) 
		find_export("get_r_public", NO_SCRIPT, 0)) ) {
		LOG(L_ERR, LOAD_ERROR "'get_r_public' not found\n");
		return -1;
	}
	if (!( scscfb->r_unlock=(r_unlock_f) 
		find_export("r_unlock", NO_SCRIPT, 0)) ) {
		LOG(L_ERR, LOAD_ERROR "'r_unlock' not found\n");
		return -1;
	}
	if (!( scscfb->get_r_public_expires=(get_r_public_expires_f) 
		find_export("get_r_public_expires", NO_SCRIPT, 0)) ) {
		LOG(L_ERR, LOAD_ERROR "'get_r_public_expires' not found\n");
		return -1;
	}
	return 1;
}
