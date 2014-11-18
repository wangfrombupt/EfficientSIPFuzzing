/**
 * $Id: diameter_msg.c 558 2008-05-06 12:48:12Z vingarzan $
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
 * CDiameterPeer Diameter Message API
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#include "diameter.h"
#include "diameter_api.h"


#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "utils.h"
#include "globals.h"

#include "config.h"
#include "peermanager.h"

extern dp_config *config;	/**< Configuration for this diameter peer */


/**
 * This function encodes a AAAMessage to its network representation (encoder).
 *  From a AAAMessage structure, a buffer to be send is built.
 * @param msg - the message to encode
 * @returns 1 on success, -1 on error
 * \note This function is taken from DISC http://developer.berlios.de/projects/disc/
 */
AAAReturnCode AAABuildMsgBuffer( AAAMessage *msg )
{
	unsigned char *p;
	AAA_AVP       *avp;

	/* first let's comput the length of the buffer */
	msg->buf.len = AAA_MSG_HDR_SIZE; /* AAA message header size */
	/* count and add the avps */
	for(avp=msg->avpList.head;avp;avp=avp->next) {
		msg->buf.len += AVP_HDR_SIZE(avp->flags)+ to_32x_len( avp->data.len );
	}

	DBG("AAABuildMsgBuffer(): len=%d\n",msg->buf.len);
	/* allocate some memory */
	msg->buf.s = (char*)shm_malloc( msg->buf.len );
	if (!msg->buf.s) {
		LOG(L_ERR,"ERROR:AAABuildMsgBuffer: no more free memory!\n");
		goto error;
	}
	memset(msg->buf.s, 0, msg->buf.len);

	/* fill in the buffer */
	p = (unsigned char*)msg->buf.s;
	/* DIAMETER HEADER */
	/* message length */
	((unsigned int*)p)[0] =htonl(msg->buf.len);
	/* Diameter Version */
	*p = 1;
	p += VER_SIZE + MESSAGE_LENGTH_SIZE;
	/* command code */
	((unsigned int*)p)[0] = htonl(msg->commandCode);
	/* flags */
	*p = (unsigned char)msg->flags;
	p += FLAGS_SIZE + COMMAND_CODE_SIZE;
	/* application-ID */
	((unsigned int*)p)[0] = htonl(msg->applicationId);
	p += APPLICATION_ID_SIZE;
	/* hop by hop id */
	((unsigned int*)p)[0] = htonl(msg->hopbyhopId);
	p += HOP_BY_HOP_IDENTIFIER_SIZE;
	/* end to end id */
	((unsigned int*)p)[0] = htonl(msg->endtoendId);
	p += END_TO_END_IDENTIFIER_SIZE;

	/* AVPS */
	for(avp=msg->avpList.head;avp;avp=avp->next) {
		/* AVP HEADER */
		/* avp code */
		set_4bytes(p,avp->code);
		p +=4;
		/* flags */
		(*p++) = (unsigned char)avp->flags;
		/* avp length */
		set_3bytes(p, (AVP_HDR_SIZE(avp->flags)+avp->data.len) );
		p += 3;
		/* vendor id */
		if ((avp->flags&0x80)!=0) {
			set_4bytes(p,avp->vendorId);
			p +=4;
		}
		/* data */
		memcpy( p, avp->data.s, avp->data.len);
		p += to_32x_len( avp->data.len );
	}

	if ((char*)p-msg->buf.s!=msg->buf.len) {
		LOG(L_ERR,"BUG: build_buf_from_msg: mismatch between len and buf!\n");
		shm_free( msg->buf.s );
		msg->buf.s = 0;
		msg->buf.len = 0;
		goto error;
	}

	return 1;
error:
	return -1;
}




/**
 *  Allocates a new AAAMessage.
 * @param commandCode - the command code for this message
 * @param applicationId - application id to be set
 * @param sessionId - session id to be set
 * @param request - if you want to create a response, put the request here. If you want a 
 * request, call with NULL
 * @returns the AAAMessage* or NULL on error
 * \note This function is taken from DISC http://developer.berlios.de/projects/disc/ 
 */
AAAMessage *AAANewMessage(
	AAACommandCode commandCode,
	AAAApplicationId applicationId,
	AAASessionId *sessionId,
	AAAMessage *request)
{
	AAAMessage   *msg;
	AAA_AVP      *avp;
	AAA_AVP      *avp_t;
#if 0
	unsigned int code;
#endif
	str dest_host={"?",1};
	str dest_realm={"?",1};

	msg = 0;

	if (!sessionId||!sessionId->s) {
		if (request && request->sessionId){
			/* copy old session id */
			avp = request->sessionId;
			if (avp) {
				sessionId = &(avp->data);
			}
		}else{
//because of diameter base messages etc
//			LOG(L_ERR,"ERROR:AAANewMessage: param session-ID received null and it's a request!!\n");
//			goto error;
		}
	}

	/* allocated a new AAAMessage structure and set it to 0 */
	msg = (AAAMessage*)shm_malloc(sizeof(AAAMessage));
	if (!msg) {
		LOG(L_ERR,"ERROR:AAANewMessage: no more free memory!!\n");
		goto error;
	}
	memset(msg,0,sizeof(AAAMessage));

	/* command code */
	msg->commandCode = commandCode;
	/* application ID */
	msg->applicationId = applicationId;

	/* add session ID */
	if (sessionId){
		avp = AAACreateAVP( 263, 0, 0, sessionId->s, sessionId->len,
			AVP_DUPLICATE_DATA);
		if ( !avp || AAAAddAVPToMessage(msg,avp,0)!=AAA_ERR_SUCCESS) {
			LOG(L_ERR,"ERROR:AAANewMessage: cannot create/add Session-Id avp\n");
			if (avp) AAAFreeAVP( &avp );
			goto error;
		}
		msg->sessionId = avp;
	}
	

	/* add origin host AVP */
	/* changed by cristian to comply with rfc3588: 
	 * 6.3.  Origin-Host AVP
	 *
	 *    The Origin-Host AVP (AVP Code 264) is of type
	 *    DiameterIdentity... */
	avp = AAACreateAVP( 264, 0, 0, config->fqdn.s, config->fqdn.len,
		AVP_DUPLICATE_DATA);
	if (!avp||AAAAddAVPToMessage(msg,avp,msg->avpList.tail)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERROR:AAANewMessage: cannot create/add Origin-Host avp\n");
		if (avp) AAAFreeAVP( &avp );
		goto error;
	}
	msg->orig_host = avp;
	/* add origin realm AVP */
	avp = AAACreateAVP( 296, 0, 0, config->realm.s, config->realm.len,
		AVP_DUPLICATE_DATA);
	if (!avp||AAAAddAVPToMessage(msg,avp,msg->avpList.tail)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERROR:AAANewMessage: cannot create/add Origin-Realm avp\n");
		if (avp) AAAFreeAVP( &avp );
		goto error;
	}
	msg->orig_realm = avp;

	if (!request) {
		/* it's a new request -> set the flag */
		msg->flags = 0x80;
		/* keep track of the session -> SendMessage will need it! */
		msg->sId = sessionId;
	} else {
		/* it'a an answer -> it will have the same session Id */
		msg->sId = request->sId;
		/* link the incoming peer to the answer */
		msg->in_peer = request->in_peer;
		/* set the P flag as in request */
		msg->flags |= request->flags&0x40;
		/**/
		msg->endtoendId = request->endtoendId;
		msg->hopbyhopId = request->hopbyhopId;

	/* Mirror the old originhost/realm to destinationhost/realm*/
	avp = AAAFindMatchingAVP(request,0,AVP_Origin_Host,0,0);
	if (avp) dest_host = avp->data;
	/* add destination host and destination realm */
	avp = AAACreateAVP(AVP_Destination_Host,AAA_AVP_FLAG_MANDATORY,0,
		dest_host.s,dest_host.len,AVP_DUPLICATE_DATA);
	if (!avp) {
		LOG(L_ERR,"ERR:AAANewMessage: Failed creating Destination Host avp\n");
		return 0;
	}
	if (AAAAddAVPToMessage(msg,avp,msg->avpList.tail)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERR:AAANewMessage: Failed adding Destination Host avp to message\n");
		AAAFreeAVP(&avp);
		return 0;
	}
	avp = AAAFindMatchingAVP(request,0,AVP_Origin_Realm,0,0);
	if (avp) dest_realm = avp->data;

	avp = AAACreateAVP(AVP_Destination_Realm,AAA_AVP_FLAG_MANDATORY,0,
		dest_realm.s,dest_realm.len,AVP_DUPLICATE_DATA);
	if (!avp) {
		LOG(L_ERR,"ERR:AAANewMessage: Failed creating Destination Realm avp\n");
		return 0;
	}
	if (AAAAddAVPToMessage(msg,avp,msg->avpList.tail)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERR:AAANewMessage: Failed adding Destination Realm avp to message\n");
		AAAFreeAVP(&avp);
		return 0;
	}		


		msg->res_code=0;
		/* mirror all the proxy-info avp in the same order */
		avp_t = request->avpList.head;
		while ( (avp_t=AAAFindMatchingAVP
		(request,avp_t,284,0,AAA_FORWARD_SEARCH))!=0 ) {
			if ( (avp=AAACloneAVP(avp_t,1))==0 || AAAAddAVPToMessage( msg, avp,
			msg->avpList.tail)!=AAA_ERR_SUCCESS )
				goto error;
		}
	}

	return msg;
error:
	LOG(L_ERR,"ERROR:AAANewMessage: failed to create a new AAA message!\n");
	AAAFreeMessage(&msg);
	return 0;
}

/**
 * Create a Diameter Request.
 * @param app_id - application id to be set
 * @param command_code - the command code for this message
 * @param flags - flags to be set
 * @param sessId - session id to be set
 * @returns the AAAMessage* or NULL on error
 */
AAAMessage *AAACreateRequest(AAAApplicationId app_id,
							AAACommandCode command_code,
							AAAMsgFlag flags,
							AAASessionId *sessId)
{
	AAAMessage *msg;
	msg = AAANewMessage(command_code,app_id,sessId,0);
	if (!msg) return 0;
	msg->hopbyhopId = next_hopbyhop();
	msg->endtoendId = next_endtoend();
	msg->flags |= flags;	
	return msg;
}

/**
 * Create a Diameter Response to a given Request. 
 * @param request - the request that this response is for
 * @returns the AAAMessage* or NULL on error
 */
AAAMessage *AAACreateResponse(AAAMessage *request)
{
	AAAMessage *msg;
	msg = AAANewMessage(request->commandCode,request->applicationId,request->sId,request);
		
	return msg;
}


/**
 *  Frees a AVP List and all the members
 * @param avpList - list to be freed
 * @returns AAA_ERR_SUCCESS 
 */
AAAReturnCode  AAAFreeAVPList(AAA_AVP_LIST *avpList)
{
	AAA_AVP *avp_t;
	AAA_AVP *avp;
	/* free the avp list */
	avp = avpList->head;
	while (avp) {
		avp_t = avp;
		avp = avp->next;
		/*free the avp*/
		AAAFreeAVP(&avp_t);
	}
	avpList->head = 0;
	avpList->tail = 0;
	return AAA_ERR_SUCCESS;
}

/**
 *  Frees completely a message allocated through AAANewMessage()
 * @param msg - pointer to the pointer containing the message.
 * @returns AAA_ERR_SUCCESS 
 */
AAAReturnCode  AAAFreeMessage(AAAMessage **msg)
{
	LOG(L_DBG,"DBG:AAAFreeMessage: Freeing message (%p) %d\n",*msg,(*msg)->commandCode);
	/* param check */
	if (!msg || !(*msg))
		goto done;

	/* free the avp list */
	AAAFreeAVPList(&((*msg)->avpList));

	/* free the buffer (if any) */
	if ( (*msg)->buf.s )
		shm_free( (*msg)->buf.s );

	/* free the AAA msg */
	shm_free(*msg);
	*msg = 0;

done:
	return AAA_ERR_SUCCESS;
}

/**
 *  Sets the proper result_code into the Result-Code AVP; ths avp must already
 * exists into the reply messge.
 * @param message - the message to set the Result-Code to
 * @param resultCode - code to set as result 
 * \note This function is taken from DISC http://developer.berlios.de/projects/disc/ 
 */
AAAResultCode  AAASetMessageResultCode(
	AAAMessage *message,
	AAAResultCode resultCode)
{
	if ( !is_req(message) && message->res_code) {
		*((unsigned int*)(message->res_code->data.s)) = htonl(resultCode);
		return AAA_ERR_SUCCESS;
	}
	return AAA_ERR_FAILURE;
}



/**
 *  This function convert message from the network format to the AAAMessage structure (decoder).
 * @param source - the source char buffer
 * @param sourceLen - the length of the input buffer
 * @param attach_buf - whether to attach the input buffer to the message
 * @returns the AAAMessage* or NULL on error
 * \note This function is taken from DISC http://developer.berlios.de/projects/disc/ 
 */
AAAMessage* AAATranslateMessage( unsigned char* source, unsigned int sourceLen,
															int attach_buf)
{
	unsigned char *ptr;
	AAAMessage    *msg;
	unsigned char version;
	unsigned int  msg_len;
	AAA_AVP       *avp;
	unsigned int  avp_code;
	unsigned char avp_flags;
	unsigned int  avp_len;
	unsigned int  avp_vendorID;
	unsigned int  avp_data_len;

	/* check the params */
	if( !source || !sourceLen || sourceLen<AAA_MSG_HDR_SIZE) {
		LOG(L_ERR,"ERROR:AAATranslateMessage: invalid buffered received!\n");
		goto error;
	}

	/* inits */
	msg = 0;
	avp = 0;
	ptr = source;

	/* alloc a new message structure */
	msg = (AAAMessage*)shm_malloc(sizeof(AAAMessage));
	if (!msg) {
		LOG(L_ERR,"ERROR:AAATranslateMessage: no more free memory!!\n");
		goto error;
	}
	memset(msg,0,sizeof(AAAMessage));

	/* get the version */
	version = (unsigned char)*ptr;
	ptr += VER_SIZE;
	if (version!=1) {
		LOG(L_ERR,"ERROR:AAATranslateMessage: invalid version [%d]in "
			"AAA msg\n",version);
		goto error;
	}

	/* message length */
	msg_len = get_3bytes( ptr );
	ptr += MESSAGE_LENGTH_SIZE;
	if (msg_len>sourceLen) {
		LOG(L_ERR,"ERROR:AAATranslateMessage: AAA message len [%d] bigger then"
			" buffer len [%d]\n",msg_len,sourceLen);
		goto error;
	}

	/* command flags */
	msg->flags = *ptr;
	ptr += FLAGS_SIZE;

	/* command code */
	msg->commandCode = get_3bytes( ptr );
	ptr += COMMAND_CODE_SIZE;

	/* application-Id */
	msg->applicationId = get_4bytes( ptr );
	ptr += APPLICATION_ID_SIZE;

	/* Hop-by-Hop-Id */
	msg->hopbyhopId = ntohl(*((unsigned int*)ptr));
	ptr += HOP_BY_HOP_IDENTIFIER_SIZE;

	/* End-to-End-Id */
	msg->endtoendId = ntohl(*((unsigned int*)ptr));
	ptr += END_TO_END_IDENTIFIER_SIZE;

	/* start decoding the AVPS */
	while (ptr < source+msg_len) {
		if (ptr+AVP_HDR_SIZE(0x80)>source+msg_len){
			LOG(L_ERR,"ERROR:AAATranslateMessage: source buffer to short!! "
				"Cannot read the whole AVP header!\n");
			goto error;
		}
		/* avp code */
		avp_code = get_4bytes( ptr );
		ptr += AVP_CODE_SIZE;
		/* avp flags */
		avp_flags = (unsigned char)*ptr;
		ptr += AVP_FLAGS_SIZE;
		/* avp length */
		avp_len = get_3bytes( ptr );
		ptr += AVP_LENGTH_SIZE;
		if (avp_len<1) {
			LOG(L_ERR,"ERROR:AAATranslateMessage: invalid AVP len [%d]\n",
				avp_len);
			goto error;
		}
		/* avp vendor-ID */
		avp_vendorID = 0;
		if (avp_flags&AAA_AVP_FLAG_VENDOR_SPECIFIC) {
			avp_vendorID = get_4bytes( ptr );
			ptr += AVP_VENDOR_ID_SIZE;
		}
		/* data length */
		avp_data_len = avp_len-AVP_HDR_SIZE(avp_flags);
		/*check the data length */
		if ( source+msg_len<ptr+avp_data_len) {
			LOG(L_ERR,"ERROR:AAATranslateMessage: source buffer to short!! "
				"Cannot read a whole data for AVP!\n");
			goto error;
		}

		/* create the AVP */
		avp = AAACreateAVP( avp_code, avp_flags, avp_vendorID, (char*) ptr,
			avp_data_len, AVP_DONT_FREE_DATA);
		if (!avp)
			goto error;

		/* link the avp into aaa message to the end */
		AAAAddAVPToMessage( msg, avp, msg->avpList.tail);

		ptr += to_32x_len( avp_data_len );
	}

	/* link the buffer to the message */
	if (attach_buf) {
		msg->buf.s = (char*) source;
		msg->buf.len = msg_len;
	}

	//AAAPrintMessage( msg );
	return  msg;
error:
	LOG(L_ERR,"ERROR:AAATranslateMessage: message conversion droped!!\n");
	AAAFreeMessage(&msg);
	return 0;
}



/**
 *  print as debug all info contained by an aaa message + AVPs
 * @param msg - the AAAMessage to print
 * \note This function is taken from DISC http://developer.berlios.de/projects/disc/ 
 */
void AAAPrintMessage( AAAMessage *msg)
{
	char    buf[1024];
	AAA_AVP *avp;

	/* print msg info */
	DBG("DEBUG: AAA_MESSAGE - %p\n",msg);
	DBG("\tCode = %u\n",msg->commandCode);
	DBG("\tFlags = %x\n",msg->flags);

	/*print the AVPs */
	avp = msg->avpList.head;
	while (avp) {
		AAAConvertAVPToString(avp,buf,1024);
		DBG("\n%s\n",buf);
		avp=avp->next;
	}
}



