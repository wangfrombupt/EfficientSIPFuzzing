/*
 * $Id: third_party_reg.h 166 2007-03-02 19:28:23Z vingarzan $
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
 * Serving-CSCF - Third party register towards AS
 * 
 *  \author Erling Klaeboe klaboe -at- colibria dot com
 * 
 */
 
#ifndef _ISC_THIRD_PARTY_REG_H_
#define _ISC_THIRD_PARTY_REG_H_

#include "checker.h"
#include "mark.h"

#include "../../sr_module.h"
#include "../../locking.h"
#include "../tm/tm_load.h"


/** reg event notification structure */
typedef struct _r_third_party_reg {	
	str req_uri;            /* AS sip uri:  	*/
	str from;               /* SCSCF uri            */
	str to;                 /* Public user id       */
	str pani;		/* Access network info 	*/
	str cv;			/* Charging vector 	*/
} r_third_party_registration;

int isc_third_party_reg(struct sip_msg *msg, isc_match *m,isc_mark *mark);

int r_third_party_reg(str req_uri, str to, int duration);

r_third_party_registration* new_r_third_party_reg(str req_uri, str to, str from, str pani, str cv);

int r_send_third_party_reg(r_third_party_registration *r,int duration);

void r_third_party_reg_response(struct cell *t,int type,struct tmcb_params *ps);

void free_r_registration(r_third_party_registration *r);

#endif //_ISC_THIRD_PARTY_REG_H_
