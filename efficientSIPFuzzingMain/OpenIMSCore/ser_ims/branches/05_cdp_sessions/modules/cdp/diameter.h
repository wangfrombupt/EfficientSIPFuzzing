/**
 * $Id: diameter.h 243 2007-04-20 07:32:35Z shenny $
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
 * CDiameterPeer Diameter constants definitions and basic macros
 
 *  \note This file is mostly taken from DISC http://developer.berlios.de/projects/disc/
 *  
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */ 
#ifndef DIAMETER_H_
#define DIAMETER_H_


#include "utils.h"
#include <ctype.h>

#define get_2bytes(_b) \
	((((unsigned char)(_b)[0])<<8)|\
	 (((unsigned char)(_b)[1])))

#define get_3bytes(_b) \
	((((unsigned char)(_b)[0])<<16)|(((unsigned char)(_b)[1])<<8)|\
	(((unsigned char)(_b)[2])))

#define get_4bytes(_b) \
	((((unsigned char)(_b)[0])<<24)|(((unsigned char)(_b)[1])<<16)|\
	(((unsigned char)(_b)[2])<<8)|(((unsigned char)(_b)[3])))

#define set_2bytes(_b,_v) \
	{(_b)[0]=((_v)&0x0000ff00)>>8;\
	(_b)[1]=((_v)&0x000000ff);}

#define set_3bytes(_b,_v) \
	{(_b)[0]=((_v)&0x00ff0000)>>16;(_b)[1]=((_v)&0x0000ff00)>>8;\
	(_b)[2]=((_v)&0x000000ff);}

#define set_4bytes(_b,_v) \
	{(_b)[0]=((_v)&0xff000000)>>24;(_b)[1]=((_v)&0x00ff0000)>>16;\
	(_b)[2]=((_v)&0x0000ff00)>>8;(_b)[3]=((_v)&0x000000ff);}

#define to_32x_len( _len_ ) \
	( (_len_)+(((_len_)&3)?4-((_len_)&3):0) )
	
	
/* AAA TYPES */

#define AAA_NO_VENDOR_ID           0

#define VER_SIZE                   1
#define MESSAGE_LENGTH_SIZE        3
#define FLAGS_SIZE                 1
#define COMMAND_CODE_SIZE          3
#define APPLICATION_ID_SIZE        4
#define HOP_BY_HOP_IDENTIFIER_SIZE 4
#define END_TO_END_IDENTIFIER_SIZE 4
#define AVP_CODE_SIZE      4
#define AVP_FLAGS_SIZE     1
#define AVP_LENGTH_SIZE    3
#define AVP_VENDOR_ID_SIZE 4

#define AAA_MSG_HDR_SIZE  \
	(VER_SIZE + MESSAGE_LENGTH_SIZE + FLAGS_SIZE + COMMAND_CODE_SIZE +\
	APPLICATION_ID_SIZE+HOP_BY_HOP_IDENTIFIER_SIZE+END_TO_END_IDENTIFIER_SIZE)

#define AVP_HDR_SIZE(_flags_)  \
	(AVP_CODE_SIZE+AVP_FLAGS_SIZE+AVP_LENGTH_SIZE+\
	AVP_VENDOR_ID_SIZE*(((_flags_)&AAA_AVP_FLAG_VENDOR_SPECIFIC)!=0) )

/* mesage codes */
#ifndef WORDS_BIGENDIAN
	#define AS_MSG_CODE      0x12010000
	#define AC_MSG_CODE      0x0f010000
	#define CE_MSG_CODE      0x01010000
	#define DW_MSG_CODE      0x18010000
	#define DP_MSG_CODE      0x1a010000
	#define RA_MSG_CODE      0x02010000
	#define ST_MSG_CODE      0x13010000
	#define MASK_MSG_CODE    0xffffff00
#else
	#error BIG endian detected!!
	#define AS_MSG_CODE      0x00000112
	#define AC_MSG_CODE      0x0000010f
	#define CE_MSG_CODE      0x00000101
	#define DW_MSG_CODE      0x00000118
	#define DP_MSG_CODE      0x0000011a
	#define RA_MSG_CODE      0x00000102
	#define ST_MSG_CODE      0x00000113
	#define MASK_MSG_CODE    0x00ffffff
#endif


typedef unsigned int    AAACommandCode;		/**< Code for a Diameter Command 	*/
typedef unsigned int    AAAVendorId;		/**< Vendor identifier				*/
typedef unsigned int    AAAExtensionId;		/**< Extension identifier			*/
typedef unsigned int    AAA_AVPCode;		/**< Code for an AVP				*/
typedef unsigned int    AAAValue;			/**< Value							*/
typedef unsigned int    AAAApplicationId;	/**< Application Identifier 		*/
typedef void*           AAAApplicationRef;	/**< Application Reference 			*/
typedef str             AAASessionId;		/**< Session Identifier				*/
typedef unsigned int    AAAMsgIdentifier;	/**< Message Identifier				*/
typedef unsigned char   AAAMsgFlag;			/**< Message flag					*/

#define Flag_Request 	0x80
#define Flag_Proxyable  0x40

#define Code_CE 	257
#define Code_DW 	280
#define Code_DP 	282
	

/** Status codes returned by functions in the AAA API */
typedef enum {
	AAA_ERR_NOT_FOUND = -2,         /**< handle or id not found */
	AAA_ERR_FAILURE   = -1,         /**< unspecified failure during an AAA op. */
	AAA_ERR_SUCCESS   =  0,         /**< AAA operation succeeded */
	AAA_ERR_NOMEM,                  /**< op. caused memory to be exhausted */
	AAA_ERR_PROTO,                  /**<  AAA protocol error */
	AAA_ERR_SECURITY,
	AAA_ERR_PARAMETER,
	AAA_ERR_CONFIG,
	AAA_ERR_UNKNOWN_CMD,
	AAA_ERR_MISSING_AVP,
	AAA_ERR_ALREADY_INIT,
	AAA_ERR_TIMED_OUT,
	AAA_ERR_CANNOT_SEND_MSG,
	AAA_ERR_ALREADY_REGISTERED,
	AAA_ERR_CANNOT_REGISTER,
	AAA_ERR_NOT_INITIALIZED,
	AAA_ERR_NETWORK_ERROR,
} AAAReturnCode;


/** The following are AVP data type codes. They correspond directly to
 * the AVP data types outline in the Diameter specification [1]: */
typedef enum {
	AAA_AVP_DATA_TYPE,
	AAA_AVP_STRING_TYPE,
	AAA_AVP_ADDRESS_TYPE,
	AAA_AVP_INTEGER32_TYPE,
	AAA_AVP_INTEGER64_TYPE,
	AAA_AVP_TIME_TYPE,
} AAA_AVPDataType;


/** The following are used for AVP header flags and for flags in the AVP
 *  wrapper struct and AVP dictionary definitions. */
typedef enum {
	AAA_AVP_FLAG_NONE               = 0x00,
	AAA_AVP_FLAG_MANDATORY          = 0x40,
	AAA_AVP_FLAG_RESERVED           = 0x1F,
	AAA_AVP_FLAG_VENDOR_SPECIFIC    = 0x80,
	AAA_AVP_FLAG_END_TO_END_ENCRYPT = 0x20,
} AAA_AVPFlag;


/** List with all known application identifiers */
typedef enum {
	AAA_APP_DIAMETER_COMMON_MSG  = 0,
	AAA_APP_NASREQ               = 1,
	AAA_APP_MOBILE_IP            = 2,
	AAA_APP_DIAMETER_BASE_ACC    = 3,
	AAA_APP_RELAY                = 0xffffffff,
}AAA_APP_IDS;


/** The following are the result codes returned from remote servers as
 * part of messages */
typedef enum {
	AAA_MULTI_ROUND_AUTH          = 1001,
	AAA_SUCCESS                   = 2001,
	AAA_COMMAND_UNSUPPORTED       = 3001,
	AAA_UNABLE_TO_DELIVER         = 3002,
	AAA_REALM_NOT_SERVED          = 3003,
	AAA_TOO_BUSY                  = 3004,
	AAA_LOOP_DETECTED             = 3005,
	AAA_REDIRECT_INDICATION       = 3006,
	AAA_APPLICATION_UNSUPPORTED   = 3007,
	AAA_INVALID_HDR_BITS          = 3008,
	AAA_INVALID_AVP_BITS          = 3009,
	AAA_UNKNOWN_PEER              = 3010,
	AAA_AUTHENTICATION_REJECTED   = 4001,
	AAA_OUT_OF_SPACE              = 4002,
	AAA_ELECTION_LOST             = 4003,
	AAA_AVP_UNSUPPORTED           = 5001,
	AAA_UNKNOWN_SESSION_ID        = 5002,
	AAA_AUTHORIZATION_REJECTED    = 5003,
	AAA_INVALID_AVP_VALUE         = 5004,
	AAA_MISSING_AVP               = 5005,
	AAA_RESOURCES_EXCEEDED        = 5006,
	AAA_CONTRADICTING_AVPS        = 5007,
	AAA_AVP_NOT_ALLOWED           = 5008,
	AAA_AVP_OCCURS_TOO_MANY_TIMES = 5009,
	AAA_NO_COMMON_APPLICATION     = 5010,
	AAA_UNSUPPORTED_VERSION       = 5011,
	AAA_UNABLE_TO_COMPLY          = 5012,
	AAA_INVALID_BIT_IN_HEADER     = 5013,
	AAA_INVALIS_AVP_LENGTH        = 5014,
	AAA_INVALID_MESSGE_LENGTH     = 5015,
	AAA_INVALID_AVP_BIT_COMBO     = 5016,
	AAA_NO_COMMON_SECURITY        = 5017,
} AAAResultCode;

/** Standard AVP Codes */
typedef enum {
	AVP_User_Name                     =    1,
	AVP_Framed_IP_Address             =	   8,	
	AVP_Class                         =   25,
	AVP_Session_Timeout               =   27,
	AVP_Proxy_State                   =   33,
	AVP_Event_Timestamp				  =   55,
	AVP_Acct_Interim_Interval		  =   85,
	AVP_Framed_Interface_Id           =   96,	
	AVP_Framed_IPv6_Prefix            =   97,
	AVP_Host_IP_Address               =  257,
	AVP_Auth_Application_Id           =  258,
	AVP_Acct_Application_Id           =  259,	
	AVP_Vendor_Specific_Application_Id=  260,
	AVP_Redirect_Max_Cache_Time       =  262,
	AVP_Session_Id                    =  263,
	AVP_Origin_Host                   =  264,
	AVP_Supported_Vendor_Id           =  265,
	AVP_Vendor_Id                     =  266,
	AVP_Result_Code                   =  268,
	AVP_Product_Name                  =  269,
	AVP_Session_Binding               =  270,
	AVP_Disconnect_Cause              =  273,
	AVP_Auth_Request_Type             =  274,
	AVP_Auth_Grace_Period             =  276,
	AVP_Auth_Session_State            =  277,
	AVP_Origin_State_Id               =  278,
	AVP_Proxy_Host                    =  280,
	AVP_Error_Message                 =  281,
	AVP_Record_Route                  =  282,
	AVP_Destination_Realm             =  283,
	AVP_Proxy_Info                    =  284,
	AVP_Re_Auth_Request_Type          =  285,
	AVP_Authorization_Lifetime        =  291,
	AVP_Redirect_Host                 =  292,
	AVP_Destination_Host              =  293,
	AVP_Termination_Cause             =  295,
	AVP_Origin_Realm                  =  296,
	AVP_Accounting_Record_Type		  =  480,
	AVP_Accounting_Record_Number      =  485,
	
	/* defined in IETF 4006 */
	AVP_Service_Context_Id			  =  461
	
}AAA_AVPCodeNr;


typedef enum {
        Permanent_Termination   = 0,
        New_Server_Assigned             = 1,
        Server_Change                   = 2,
        Remove_S_CSCF                   = 3,
}AAA_AVPReasonCode;



/**   The following type allows the client to specify which direction to
 *   search for an AVP in the AVP list: */
typedef enum {
	AAA_FORWARD_SEARCH = 0,	/**< search forward 	*/
	AAA_BACKWARD_SEARCH		/**< search backwards 	*/
} AAASearchType;


/** Accounting message types */
typedef enum {
	AAA_ACCT_EVENT = 1,
	AAA_ACCT_START = 2,
	AAA_ACCT_INTERIM = 3,
	AAA_ACCT_STOP = 4
} AAAAcctMessageType;

/** Hint on what do do with the AVP payload */
typedef enum {
	AVP_DUPLICATE_DATA,		/**< Duplicate the payload; the source can be safely removed at any time */
	AVP_DONT_FREE_DATA,		/**< Don't duplicate and don't free; the source will always be there. */
	AVP_FREE_DATA,			/**< Don't duplicate, but free when done; this is the only reference to source. */
} AVPDataStatus;

/** This structure contains a message AVP in parsed format */
typedef struct avp {
	struct avp *next;		/**< next AVP if in a list 				*/
	struct avp *prev;		/**< previous AVP if in a list 			*/
	AAA_AVPCode code;		/**< AVP code 							*/
	AAA_AVPFlag flags;		/**< AVP flags 							*/
	AAA_AVPDataType type;	/**< AVP payload type 					*/
	AAAVendorId vendorId;	/**< AVP vendor id 						*/
	str data;				/**< AVP payload						*/
	unsigned char free_it;	/**< if to free the payload when done	*/
} AAA_AVP;


/**
 * This structure is used for representing lists of AVPs on the
 * message or in grouped AVPs. */
typedef struct _avp_list_t {
	AAA_AVP *head;			/**< The first AVP in the list 	*/
	AAA_AVP *tail;			/**< The last AVP in the list 	*/
} AAA_AVP_LIST;


/** This structure contains the full AAA message. */
typedef struct _message_t {
	AAACommandCode      commandCode;	/**< command code for the message */
	AAAMsgFlag          flags;			/**< flags */
	AAAApplicationId    applicationId;	/**< application identifier */
	AAAMsgIdentifier    endtoendId;		/**< End-to-end identifier */
	AAAMsgIdentifier    hopbyhopId;		/**< Hop-by-hop identitfier */
	AAASessionId        *sId;			/**< Session identifier */
	AAA_AVP             *sessionId;		/**< shortcut to SessionId AVP */
	AAA_AVP             *orig_host;		/**< shortcut to Origin Host AVP */
	AAA_AVP             *orig_realm;	/**< shortcut to Origin Realm AVP */
	AAA_AVP             *dest_host;		/**< shortcut to Destination Host AVP */
	AAA_AVP             *dest_realm;	/**< shortcut to Destination Realm AVP */
	AAA_AVP             *res_code;		/**< shortcut to Result Code AVP */
	AAA_AVP             *auth_ses_state;/**< shortcut to Authorization Session State AVP */
	AAA_AVP_LIST        avpList;		/**< list of AVPs in the message */
	str                 buf;			/**< Diameter network representation */
	void                *in_peer;		/**< Peer that this message was received from */
} AAAMessage;




/**************************** AAA MESSAGE FUNCTIONS **************************/

/* MESSAGES */

/** if the message is a request */
#define is_req(_msg_) \
	(((_msg_)->flags)&0x80)




/**
 * This structure defines a Diameter Transaction.
 * This is used to link a response to a request
 */
typedef struct _AAATransaction{
	unsigned int hash,label;
	AAAApplicationId application_id;
	AAACommandCode command_code;
} AAATransaction;


/** Function for callback on transaction events: response or time-out for request. */
typedef void (AAATransactionCallback_f)(int is_timeout,void *param,AAAMessage *ans);
/** Function for callback on request received */
typedef AAAMessage* (AAARequestHandler_f)(AAAMessage *req, void *param);
/** Function for callback on response received */
typedef void (AAAResponseHandler_f)(AAAMessage *res, void *param);


#endif /*DIAMETER_H_*/
