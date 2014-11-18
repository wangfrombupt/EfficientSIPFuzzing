/**
 * $Id: diameter_api.h 355 2007-06-28 15:41:19Z vingarzan $
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
 * CDiameterPeer - Diameter API interface
 * 
 * For definition, you would want to look into:
 * - diameter_api.c for the generic API.
 * - diameter_avp.c for the AVP API.
 * - diameter_msg.c for the message API. 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#ifndef __DIAMETER_API_H_
#define __DIAMETER_API_H_

#include "diameter.h"
#include "utils.h"

				/* TRANSACTIONS */
				
/** Timeout for Diameter transactions (this is quite big, 
 * but increase in case that you have a slow peer) */ 				
#define DP_TRANS_TIMEOUT 5




AAATransaction *AAACreateTransaction(AAAApplicationId app_id,AAACommandCode cmd_code);
int AAADropTransaction(AAATransaction *trans);

AAASessionId AAACreateSession();
int AAADropSession(AAASessionId *s);


				/* CALLBACKS */


int AAAAddRequestHandler(AAARequestHandler_f *f,void *param);
int AAAAddResponseHandler(AAAResponseHandler_f *f,void *param);

				/* MESSAGE SENDING */

AAAReturnCode AAASendMessage(	
		AAAMessage *message,
		AAATransactionCallback_f *callback_f,
		void *callback_param);

AAAReturnCode AAASendMessageToPeer(	
		AAAMessage *message,
		str *peer_id, 
		AAATransactionCallback_f *callback_f,
		void *callback_param);

AAAMessage* AAASendRecvMessage(AAAMessage *msg);

AAAMessage* AAASendRecvMessageToPeer(AAAMessage *msg, str *peer_id);

				/* MESSAGE CREATION */
				
AAAMessage *AAACreateRequest(AAAApplicationId app_id,
							AAACommandCode command_code,
							AAAMsgFlag flags,
							AAASessionId *sessId);

AAAMessage *AAACreateResponse(AAAMessage *request);

AAAMessage *AAANewMessage(
		AAACommandCode commandCode,
		AAAApplicationId appId,
		AAASessionId *sessionId,
		AAAMessage *request);

AAAReturnCode AAAFreeAVPList(
		AAA_AVP_LIST *avpList);

AAAReturnCode AAAFreeMessage(
		AAAMessage **message);

AAAResultCode AAASetMessageResultCode(
		AAAMessage *message,
		AAAResultCode resultCode);

void AAAPrintMessage(
		AAAMessage *msg);

AAAReturnCode AAABuildMsgBuffer(
		AAAMessage *msg );

AAAMessage* AAATranslateMessage(
		unsigned char* source,
		unsigned int sourceLen,
		int attach_buf );


/* AVPS */

/** 
 * Create and add an AVP to the message, by duplicating the storage space
 */
#define AAACreateAndAddAVPToMessage(_msg_,_code_,_flags_,_vdr_,_data_,_len_) \
	( AAAAddAVPToMessage(_msg_, \
	AAACreateAVP(_code_,_flags_,_vdr_,_data_,_len_, AVP_DUPLICATE_DATA),\
	(_msg_)->avpList.tail) )

AAA_AVP* AAACreateAVP(
		AAA_AVPCode code,
		AAA_AVPFlag flags,
		AAAVendorId vendorId,
		char *data,
		size_t length,
		AVPDataStatus data_status);

AAA_AVP* AAACloneAVP(
		AAA_AVP *avp,
		unsigned char duplicate_data );

AAAReturnCode AAAAddAVPToMessage(
		AAAMessage *msg,
		AAA_AVP *avp,
		AAA_AVP *position);

AAA_AVP *AAAFindMatchingAVP(
		AAAMessage *msg,
		AAA_AVP *startAvp,
		AAA_AVPCode avpCode,
		AAAVendorId vendorId,
		AAASearchType searchType);				


AAAReturnCode AAARemoveAVPFromMessage(
		AAAMessage *msg,
		AAA_AVP *avp);

AAAReturnCode AAAFreeAVP(
		AAA_AVP **avp);

AAA_AVP* AAAGetFirstAVP(
		AAA_AVP_LIST *avpList);

AAA_AVP* AAAGetLastAVP(
		AAA_AVP_LIST *avpList);

AAA_AVP* AAAGetNextAVP(
		AAA_AVP *avp);

AAA_AVP* AAAGetPrevAVP(
		AAA_AVP *avp);

char *AAAConvertAVPToString(
		AAA_AVP *avp,
		char *dest,
		unsigned int destLen);

 

str AAAGroupAVPS(AAA_AVP_LIST avps);

AAA_AVP_LIST AAAUngroupAVPS(str buf);

AAA_AVP  *AAAFindMatchingAVPList(
	AAA_AVP_LIST avpList,
	AAA_AVP *startAvp,
	AAA_AVPCode avpCode,
	AAAVendorId vendorId,
	AAASearchType searchType);

void AAAAddAVPToAVPList(
	AAA_AVP_LIST *list,
	AAA_AVP *avp);	


//typedef struct {
//	str endtoendid;
//	str hopbyhopid;
//	void **udata;
//} AAATransaction;
//
//enum {
//	AAATransaction_Error	=-1,
//	AAATransaction_Timeout	= 0,
//	AAATransaction_Success	= 1	
//} AAATransactionResponse;
//
//int AAAPeerConnect();
//
//int AAAPeerDisconnect();
//
//AAATransaction *AAACreateTransaction(AAAApplicationId app_id,void *udata);
//
//int AAADropTransaction(AAATransaction *trans);
//

						

//AAAMessage *AAAExchangeMessage(AAAMessage *request);

//int AAASendMessage(AAAMessage *msg);

//int AAAAnswerHandler(AAAMessage *response,void **udata);

//int AAAFailureHandler(int reason,void **udata);

//int AAASIRequestHandler(AAAMessage *request);



#endif /*DIAMETER_API_H_*/
