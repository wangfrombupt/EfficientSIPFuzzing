/**
 * $Id: cdp_load.c 355 2007-06-28 15:41:19Z vingarzan $
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
 * CDiameterPeer - functional bindings for usage in other SER modules
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
 
#include "cdp_load.h"

#include "../../sr_module.h"


#define LOAD_ERROR "ERROR: cdp_bind: S-CSCF module function "

#define FIND_EXP(NAME,STRING,TYPE) \
	if (!( cdpb->NAME=(TYPE) \
		find_export(STRING, NO_SCRIPT, 0)) ) {\
		LOG(L_ERR, LOAD_ERROR "'"STRING"' not found\n");\
		return -1;\
	}


/**
 * Load the CDiameterPeer bindings
 * @param *cdpb - target structure to load the bindings into
 * @returns 1 on success, -1 on failure
 */  
int load_cdp( struct cdp_binds *cdpb)
{
	FIND_EXP(AAASendMessage,"AAASendMessage",AAASendMessage_f);
	FIND_EXP(AAASendMessageToPeer,"AAASendMessageToPeer",AAASendMessageToPeer_f);
	FIND_EXP(AAASendRecvMessage,"AAASendRecvMessage",AAASendRecvMessage_f);
	FIND_EXP(AAASendRecvMessageToPeer,"AAASendRecvMessageToPeer",AAASendRecvMessageToPeer_f);
	FIND_EXP(AAAFreeMessage,"AAAFreeMessage",AAAFreeMessage_f);

	FIND_EXP(AAACreateRequest,"AAACreateRequest",AAACreateRequest_f);
	FIND_EXP(AAACreateResponse,"AAACreateResponse",AAACreateResponse_f);
	
	FIND_EXP(AAACreateSession,"AAACreateSession",AAACreateSession_f);
	FIND_EXP(AAADropSession,"AAADropSession",AAADropSession_f);

	FIND_EXP(AAACreateTransaction,"AAACreateTransaction",AAACreateTransaction_f);
	FIND_EXP(AAADropTransaction,"AAADropTransaction",AAADropTransaction_f);
	
	FIND_EXP(AAACreateAVP,"AAACreateAVP",AAACreateAVP_f);
	FIND_EXP(AAAAddAVPToMessage,"AAAAddAVPToMessage",AAAAddAVPToMessage_f);
	FIND_EXP(AAAFindMatchingAVP,"AAAFindMatchingAVP",AAAFindMatchingAVP_f);
	FIND_EXP(AAAFindMatchingAVPList,"AAAFindMatchingAVPList",AAAFindMatchingAVPList_f);
	FIND_EXP(AAAGetNextAVP,"AAAGetNextAVP",AAAGetNextAVP_f);
	FIND_EXP(AAAFreeAVP,"AAAFreeAVP",AAAFreeAVP_f);
	FIND_EXP(AAAFreeAVPList,"AAAFreeAVPList",AAAFreeAVPList_f);
	FIND_EXP(AAAGroupAVPS,"AAAGroupAVPS",AAAGroupAVPS_f);
	FIND_EXP(AAAUngroupAVPS,"AAAUngroupAVPS",AAAUngroupAVPS_f);

	FIND_EXP(AAAAddRequestHandler,"AAAAddRequestHandler",AAAAddRequestHandler_f);
	FIND_EXP(AAAAddResponseHandler,"AAAAddResponseHandler",AAAAddResponseHandler_f);
	
	return 1;
}
