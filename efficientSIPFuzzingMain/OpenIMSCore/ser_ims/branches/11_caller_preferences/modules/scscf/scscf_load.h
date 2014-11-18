/*
 * $Id: scscf_load.h 78 2007-01-02 14:09:37Z vingarzan $
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

#ifndef _SCSCF_BIND_H
#define _SCSCF_BIND_H



#include "../../sr_module.h"

#include "registrar_storage.h"

/** export not usable from scripts */
#define NO_SCRIPT	-1

/** S-CSCF bindings */
struct scscf_binds {
	get_r_public_f	get_r_public;	/**< the get_r_public() function that retrieves a 
	r_public from the S-CSCF registrar. The registrar slot is locked and you have to
	unlock it with r_unlock(p->hash) when done */
	r_unlock_f		r_unlock;		/**< the r_unlock() function to unlock a registrar slot */
	
	get_r_public_expires_f get_r_public_expires;	/**< returns the expires value */	
};

/** function to load the scscf module bindings */
typedef int(*load_scscf_f)( struct scscf_binds *scscfb );

int load_scscf( struct scscf_binds *scscfb);


#endif
