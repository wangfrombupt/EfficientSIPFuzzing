/**
 * $Id: mark.h 490 2007-11-09 00:45:40Z vingarzan $
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
 * IMS Service Control - ISC Marking Procedures
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#ifndef _ISC_MARK_H
#define _ISC_MARK_H

#include "../../parser/msg_parser.h"
#include "../../lump_struct.h"
#include "../../data_lump.h"

#include "checker.h"

/** username to be used in the Route */ 
#define ISC_MARK_USERNAME "sip:iscmark"
/** length of #ISC_MARK_USERNAME */
#define ISC_MARK_USERNAME_LEN 11

/** no mark */
#define ISC_MARK_NOMARK				0
/** mark for originating */
#define ISC_MARK_ORIG_LEG			1
/** mark for terminating */
#define ISC_MARK_TERM_LEG			2
/** mark for terminating to unregistered users */
#define ISC_MARK_TERM_UNREG_LEG		3
#define ISC_MARK_STATUS_MAX			99

/** ISC marking structure */
typedef struct _isc_mark{
	int skip;		/**< how many IFCs to skip */
	char handling;	/**< handling to apply on failure to contact the AS */
	char direction;	/**< session case: orig,term,term unreg */
	str aor;		/**< the save user aor - terminating or originating */
} isc_mark;



int isc_mark_set(struct sip_msg *msg, isc_match *match, isc_mark *mark);

int isc_mark_get_from_msg(struct sip_msg *msg,isc_mark *mark);

int isc_mark_get_from_lump(struct sip_msg *msg,isc_mark *mark);

inline int isc_mark_write_route(struct sip_msg *msg,str *as,str *iscmark);
	
inline int isc_mark_drop_route(struct sip_msg *msg);

#endif
