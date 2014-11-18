/**
 * $Id: cdp_load.h 355 2007-06-28 15:41:19Z vingarzan $
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

#ifndef _CDP_BIND_H
#define _CDP_BIND_H

#define CDP_FOR_SER 1

#include "utils.h"
#include "diameter.h"
#include "diameter_ims.h"
#include "peer.h"

#define NO_SCRIPT	-1


typedef AAAMessage* (*AAACreateRequest_f)(AAAApplicationId app_id,
							AAACommandCode command_code,
							AAAMsgFlag flags,
							AAASessionId *sessId);
typedef AAAMessage* (*AAACreateResponse_f)(AAAMessage *request);

typedef AAASessionId (*AAACreateSession_f)();
typedef int (*AAADropSession_f)(AAASessionId *s);

typedef AAATransaction * (*AAACreateTransaction_f)(AAAApplicationId app_id,AAACommandCode cmd_code);
typedef int (*AAADropTransaction_f)(AAATransaction *trans);


typedef AAA_AVP* (*AAACreateAVP_f)(
									AAA_AVPCode code,
									AAA_AVPFlag flags,
									AAAVendorId vendorId,
									char *data,
									size_t length,
									AVPDataStatus data_status);
		
typedef AAAReturnCode (*AAAAddAVPToMessage_f)(
												AAAMessage *msg,
												AAA_AVP *avp,
												AAA_AVP *position);

typedef AAA_AVP* (*AAAFindMatchingAVP_f)(
										AAAMessage *msg,
										AAA_AVP *startAvp,
										AAA_AVPCode avpCode,
										AAAVendorId vendorId,
										AAASearchType searchType);

typedef AAA_AVP  *(*AAAFindMatchingAVPList_f)(
												AAA_AVP_LIST avpList,
												AAA_AVP *startAvp,
												AAA_AVPCode avpCode,
												AAAVendorId vendorId,
												AAASearchType searchType);
typedef AAA_AVP* (*AAAGetNextAVP_f)(AAA_AVP *avp);



typedef AAAReturnCode (*AAAFreeAVP_f)(
										AAA_AVP **avp);
		
typedef AAAReturnCode  (*AAAFreeAVPList_f)(AAA_AVP_LIST *avpList);

typedef str (*AAAGroupAVPS_f)(AAA_AVP_LIST avps);

typedef AAA_AVP_LIST (*AAAUngroupAVPS_f)(str buf);


typedef AAAReturnCode (*AAASendMessage_f)(	
											AAAMessage *message,
											AAATransactionCallback_f *callback_f,
											void *callback_param);

typedef AAAReturnCode (*AAASendMessageToPeer_f)(	
											AAAMessage *message,
											str *peer_id, 
											AAATransactionCallback_f *callback_f,
											void *callback_param);

typedef AAAMessage* (*AAASendRecvMessage_f)(AAAMessage *msg);

typedef AAAMessage* (*AAASendRecvMessageToPeer_f)(AAAMessage *msg, str *peer_id);

typedef AAAReturnCode (*AAAFreeMessage_f)(
											AAAMessage **message);


typedef int (*AAAAddRequestHandler_f)(AAARequestHandler_f *f,void *param);
typedef int (*AAAAddResponseHandler_f)(AAAResponseHandler_f *f,void *param);


struct cdp_binds {
	AAASendMessage_f AAASendMessage;
	AAASendMessageToPeer_f AAASendMessageToPeer;
	AAASendRecvMessage_f AAASendRecvMessage;
	AAASendRecvMessageToPeer_f AAASendRecvMessageToPeer;
	AAAFreeMessage_f AAAFreeMessage;
	
	AAACreateRequest_f AAACreateRequest;
	AAACreateResponse_f AAACreateResponse;	
	
	AAACreateSession_f AAACreateSession;
	AAADropSession_f AAADropSession;

	AAACreateTransaction_f AAACreateTransaction;
	AAADropTransaction_f AAADropTransaction;
	
	AAACreateAVP_f AAACreateAVP;
	AAAAddAVPToMessage_f AAAAddAVPToMessage;
	AAAFindMatchingAVP_f AAAFindMatchingAVP;
	AAAFindMatchingAVPList_f AAAFindMatchingAVPList;
	AAAGetNextAVP_f AAAGetNextAVP;
	AAAFreeAVP_f AAAFreeAVP;
	AAAFreeAVPList_f AAAFreeAVPList;
	AAAGroupAVPS_f AAAGroupAVPS;
	AAAUngroupAVPS_f AAAUngroupAVPS;
	
	AAAAddRequestHandler_f AAAAddRequestHandler;
	AAAAddResponseHandler_f AAAAddResponseHandler;
};


typedef int(*load_cdp_f)( struct cdp_binds *cdpb );
int load_cdp( struct cdp_binds *cdpb);

#endif

