/**
 * $Id: diameter_ims.h 438 2007-08-28 10:12:24Z albertoberlios $
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
 * CDiameterPeer Diameter IMS IANA defined numbers
 * 
 * This is a compilation of different 3GPP TSs:
 * - TS 29.209 for IMS_Gq
 * - TS 29.229 for IMS_Cx IMS_Dx
 * - TS 29.329 for IMS_Sh IMS_Ph
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *  
 */
 
#ifndef __DIAMETER_IMS_H
#define __DIAMETER_IMS_H

/* Application Identifiers	*/
#define IMS_Gq	16777222	/**< Gq interface between P-CSCF and PDF 		*/
#define IMS_Cx	16777216	/**< Cx interface between I/S-CSCF and HSS 		*/
#define IMS_Dx	16777216	/**< Cx interface between I/S-CSCF and SLF 		*/
#define IMS_Sh	16777217	/**< Sh interface between AS and HSS	 		*/
#define IMS_e2	16777231	/**< e2 interface between CLF and AF	 		*/
#define IMS_Ph	16777217	/**< Sh interface between PresenceServer and HSS*/
#define IMS_Rx  16777236	/**< Rx interface between P-CSCF and PCRF 		*/
#define IMS_Gx	16777224	/**< Gx interface between PCRF and PCEF 		*/
#define IMS_Rf  16777223    /**< Rf interface between P/I/S-CSCF and CDF, 
								according to TS32.299 R7    */ 

/* Vendor ID	*/

#define IMS_vendor_id_3GPP 		10415		/**< Vendor Id for 3GPP */
#define IMS_vendor_id_3GPP_char "10415" 	/**< char value for 3GPP's Vendor Id */
#define IMS_vendor_id_3GPP_len	5			/**< len of char value for 3GPP's Vendor Id */

#define IMS_vendor_id_ETSI 		13019		/**< Vendor Id for ETSI */
#define IMS_vendor_id_ETSI_char "13019" 	/**< char value for ETSI's Vendor Id */
#define IMS_vendor_id_ETSI_len	5			/**< len of char value for 3GPP's Vendor Id */

#define IMS_vendor_id_CableLabs 	 4491	/**< Vendor Id for CableLabs */
#define IMS_vendor_id_CableLabs_char "4491"	/**< char value for ETSI's Vendor Id */
#define IMS_vendor_id_CableLabs_len	 4		/**< len of char value for 3GPP's Vendor Id */

/*	Command Codes alocated for IMS	*/
/*		The Gq Interface 			*/
#define IMS_AAR		265		/**< Bearer-Authorization		Request	*/
#define IMS_AAA		265		/**< Bearer-Authorization		Answer	*/
#define IMS_RAR		258		/**< Re-Auth					Request */
#define IMS_RAA		258		/**< Re-Auth					Answer	*/
#define IMS_STR		275		/**< Session Termination 		Request */
#define IMS_STA		275		/**< Session Termination 		Answer	*/
#define IMS_ASR		274		/**< Abort-Session-Request		Request */
#define IMS_ASA		274		/**< Abort-Session-Request		Answer	*/
/* The Gx Interface */
#define IMS_CCR		272
#define IMS_CCA		272
/*		The Cx/Dx Interface 			*/
#define IMS_UAR		300		/**< User-Authorization			Request	*/
#define IMS_UAA		300		/**< User-Authorization			Answer	*/
#define IMS_SAR		301		/**< Server-Assignment			Request */
#define IMS_SAA		301		/**< Server-Assignment			Answer	*/
#define IMS_LIR		302		/**< Location-Info				Request */
#define IMS_LIA		302		/**< Location-Info				Answer	*/
#define IMS_MAR		303		/**< Multimedia-Auth			Request */
#define IMS_MAA		303		/**< Multimedia-Auth			Answer	*/
#define IMS_RTR		304		/**< Registration-Termination	Request */
#define IMS_RTA		304		/**< Registration-Termination	Answer	*/
#define IMS_PPR		305		/**< Push-Profile				Request */
#define IMS_PPA		305		/**< Push-Profile				Answer	*/
/**		The Sh/Ph Interface 			*/
#define IMS_UDR		306		/**< User-Data					Request */
#define IMS_UDA		306		/**< User-Data					Answer	*/
#define IMS_PUR		307		/**< Profile-Update				Request */
#define IMS_PUA		307		/**< Profile-Update				Answer	*/
#define IMS_SNR		308		/**< Subscriber-Notifications	Request */
#define IMS_SNA		308		/**< Subscriber-Notifications	Answer	*/
#define IMS_PNR		309		/**< Push-Notification			Request */
#define IMS_PNA		309		/**< Push-Notification			Answer	*/
/**	Allocated Command Codes, not used yet	*/
#define IMS_10R		310
#define IMS_10A		310
#define IMS_11R		311
#define IMS_11A		311
#define IMS_12R		312
#define IMS_12A		312
#define IMS_13R		313
#define IMS_13A		313


/** 3GPP AVP Codes */ 
enum {
/**   1 to 255 reserved for backward compatibility with IMS Radius TS29.061	*/
/** 256 to 299 reserved for future use										*/
	AVP_IMS_Vendor_Id									= 266,
	AVP_IMS_Experimental_Result_Code					= 298,
	AVP_IMS_Experimental_Result							= 297,
/** 300 to 399 reserved for TS29.234											*/
/** 400 to 499 reserved for TS29.109											*/
/** 500 to 599 reserved for TS29.209											*/
	AVP_IMS_Abort_Cause									= 500,
	AVP_IMS_Access_Network_Charging_Address				= 501,
	AVP_IMS_Access_Network_Charging_Identifier			= 502,
	AVP_IMS_Access_Network_Charging_Identifier_Value	= 503,
	AVP_IMS_AF_Application_Identifier					= 504,
	AVP_IMS_AF_Charging_Identifier						= 505,
	AVP_IMS_Authorization_Token							= 506,
	AVP_IMS_Flow_Description							= 507,
	AVP_IMS_Flow_Grouping								= 508,
	AVP_IMS_Flow_Number									= 509,
	AVP_IMS_Flows										= 510,
	AVP_IMS_Flow_Status									= 511,
	AVP_IMS_Flow_Usage									= 512,
	AVP_IMS_Specific_Action								= 513,
	AVP_IMS_Max_Requested_Bandwidth_DL					= 515,
	AVP_IMS_Max_Requested_Bandwidth_UL					= 516,
	AVP_IMS_Media_Component_Description					= 517,
	AVP_IMS_Media_Component_Number						= 518,
	AVP_IMS_Media_Sub_Component							= 519,
	AVP_IMS_Media_Type									= 520,
	AVP_IMS_RR_Bandwidth								= 521,
	AVP_IMS_RS_Bandwidth								= 522,
	AVP_IMS_SIP_Forking_Indication						= 523,
/** Codec-Data is from TS 29.214*/
	AVP_IMS_Codec_Data									= 524,
/** 600 to 699 reserved for TS29.229											*/
	AVP_IMS_Visited_Network_Identifier					= 600,
	AVP_IMS_Public_Identity								= 601,
	AVP_IMS_Server_Name									= 602,
	AVP_IMS_Server_Capabilities							= 603,
	AVP_IMS_Mandatory_Capability						= 604,
	AVP_IMS_Optional_Capability							= 605,
	AVP_IMS_User_Data									= 606,
	AVP_IMS_SIP_Number_Auth_Items						= 607,
	AVP_IMS_SIP_Authentication_Scheme					= 608,
	AVP_IMS_SIP_Authenticate							= 609,
	AVP_IMS_SIP_Authorization							= 610,
	AVP_IMS_SIP_Authentication_Context					= 611,
	AVP_IMS_SIP_Auth_Data_Item							= 612,
	AVP_IMS_SIP_Item_Number								= 613,
	AVP_IMS_Server_Assignment_Type						= 614,
	AVP_IMS_Deregistration_Reason						= 615,
	AVP_IMS_Reason_Code									= 616,
	AVP_IMS_Reason_Info									= 617,
	AVP_IMS_Charging_Information						= 618,
	AVP_IMS_Primary_Event_Charging_Function_Name		= 619,
	AVP_IMS_Secondary_Event_Charging_Function_Name		= 620,
	AVP_IMS_Primary_Charging_Collection_Function_Name	= 621,
	AVP_IMS_Secondary_Charging_Collection_Function_Name	= 622,
	AVP_IMS_User_Authorization_Type						= 623,
	AVP_IMS_User_Data_Already_Available					= 624,
	AVP_IMS_Confidentiality_Key							= 625,
	AVP_IMS_Integrity_Key								= 626,
	AVP_IMS_User_Data_Request_Type						= 627,
	AVP_IMS_Supported_Features							= 628,
	AVP_IMS_Feature_List_ID								= 629,
	AVP_IMS_Feature_List								= 630,
	AVP_IMS_Supported_Applications						= 631,
	AVP_IMS_Associated_Identities						= 632,
	AVP_IMS_Originating_Request							= 633,
/** 700 to 799 reserved for TS29.329											*/
	AVP_IMS_User_Identity								= 700,
	AVP_IMS_MSISDN										= 701,
	AVP_IMS_User_Data_2									= 702,
	AVP_IMS_Data_Reference								= 703,
	AVP_IMS_Service_Indication							= 704,
	AVP_IMS_Subs_Req_Type								= 705,
	AVP_IMS_Requested_Domain							= 706,
	AVP_IMS_Current_Location							= 707,
	AVP_IMS_Identity_Set								= 708,
	AVP_IMS_Expiry_Time									= 709,
	AVP_IMS_Send_Data_Indication						= 710,
	AVP_IMS_DSAI_Tag									= 711,
	
/** 800 to 899 reserved for TS29.299											*/
	AVP_IMS_Event_Type 									= 823,
	AVP_IMS_SIP_Method									= 824,
	AVP_IMS_Event										= 825,
	AVP_IMS_Content_Type								= 826,
	AVP_IMS_Content_Length								= 827,
	AVP_IMS_Content_Disposition							= 828,
	AVP_IMS_Role_Of_Node 								= 829,
	AVP_IMS_User_Session_Id								= 830,
	AVP_IMS_Calling_Party_Address						= 831,
	AVP_IMS_Called_Party_Address						= 832,
	AVP_IMS_Time_Stamps									= 833,
	AVP_IMS_SIP_Request_Timestamp						= 834,
	AVP_IMS_SIP_Response_Timestamp						= 835,
	AVP_IMS_Application_Server							= 836,
	AVP_IMS_Application_Provided_Called_Party_Address	= 837,
	AVP_IMS_Inter_Operator_Identifier					= 838,
	AVP_IMS_Originating_IOI								= 839,
	AVP_IMS_Terminating_IOI								= 840,
	AVP_IMS_IMS_Charging_identifier						= 841,
	AVP_IMS_SDP_Session_Description						= 842,
	AVP_IMS_SDP_Media_Component							= 843,
	AVP_IMS_SDP_Media_Name								= 844,
	AVP_IMS_SDP_Media_Description						= 845,
	AVP_IMS_CG_Address									= 846,
	AVP_IMS_GGSN_Address								= 847,
	AVP_IMS_Served_Party_IP_Address						= 848,
	AVP_IMS_Authorized_QoS								= 849,
	AVP_IMS_Application_Service_Information				= 850,
	AVP_IMS_Trunk_Group_Id								= 851,
	AVP_IMS_Incoming_Trunk_Group_Id						= 852,
	AVP_IMS_Outgoing_Trunk_Group_Id						= 853,
	AVP_IMS_Bear_Service								= 854,
	AVP_IMS_Service_Id									= 855,
	AVP_IMS_Associated_URI								= 856,
	AVP_IMS_Charged_Party								= 857,
	AVP_IMS_PoC_Controlling_Address						= 858,
	AVP_IMS_PoC_Group_Name								= 859,
	AVP_IMS_Cause										= 860,
	AVP_IMS_Cause_Code									= 861,
	
	/* TODO finish the list... */
	AVP_IMS_Node_Functionality							= 862,
	AVP_IMS_Service_Information							= 873,
	AVP_IMS_IMS_Information								= 876,
	AVP_IMS_Expires										= 888,
	AVP_IMS_Message_Body								= 889,
	AVP_IMS_Service_Specific_Info						= 1249,
	AVP_IMS_Requested_Party_Address						= 1251,
	AVP_IMS_Access_Network_Information					= 1263,
/** 1000   from TS29.212 */
 	AVP_IMS_Bearer_Identifier							= 1020,
 	AVP_IMS_Charging_Rule_Install						= 1001,
 	AVP_IMS_Charging_Rule_Remove						= 1002,
 	AVP_IMS_Charging_Rule_Definition					= 1003,
 	AVP_IMS_Charging_Rule_Base_Name						= 1004,
 	AVP_IMS_Charging_Rule_Name							= 1005,
 	AVP_IMS_Charging_Rule_Report						= 1018
};

/** ETSI AVP Codes */ 
enum {
	
	/*added from ETSI 283 034 */

	
	AVP_Line_Identifier									= 500,
	AVP_ETSI_SIP_Authenticate 							= 501, 
	AVP_ETSI_SIP_Authorization 							= 502, 
	AVP_ETSI_SIP_Authentication_Info 					= 503, 
	AVP_ETSI_Digest_Realm 								= 504,  
	AVP_ETSI_Digest_Nonce 								= 505,  
	AVP_ETSI_Digest_Domain								= 506,  
	AVP_ETSI_Digest_Opaque 								= 507,  
	AVP_ETSI_Digest_Stale 								= 508,  
	AVP_ETSI_Digest_Algorithm 							= 509,  
	AVP_ETSI_Digest_QoP 								= 510,  
	AVP_ETSI_Digest_HA1 								= 511,  
	AVP_ETSI_Digest_Auth_Param 							= 512,  
	AVP_ETSI_Digest_Username 							= 513,  
	AVP_ETSI_Digest_URI 								= 514,  
	AVP_ETSI_Digest_Response 							= 515,  
	AVP_ETSI_Digest_CNonce 								= 516,  
	AVP_ETSI_Digest_Nonce_Count 						= 517,  
	AVP_ETSI_Digest_Method 								= 518,  
	AVP_ETSI_Digest_Entity_Body_Hash 					= 519,  
	AVP_ETSI_Digest_Nextnonce 							= 520,  
	AVP_ETSI_Digest_Response_Auth						= 521	
};

/** CableLabs AVP Codes */ 
enum {
	AVP_CableLabs_SIP_Digest_Authenticate 				= 228,
	AVP_CableLabs_Digest_Realm 							= 209,
	AVP_CableLabs_Digest_Domain 						= 206,
	AVP_CableLabs_Digest_Algorithm 						= 204,
	AVP_CableLabs_Digest_QoP 							= 208,
	AVP_CableLabs_Digest_HA1 							= 207,
	AVP_CableLabs_Digest_Auth_Param 					= 205
};

/** Server-Assignment-Type Enumerated AVP */
enum {
	AVP_IMS_SAR_ERROR									= -1,
	AVP_IMS_SAR_NO_ASSIGNMENT							= 0,
	AVP_IMS_SAR_REGISTRATION							= 1,
	AVP_IMS_SAR_RE_REGISTRATION							= 2,
	AVP_IMS_SAR_UNREGISTERED_USER						= 3,
	AVP_IMS_SAR_TIMEOUT_DEREGISTRATION					= 4,
	AVP_IMS_SAR_USER_DEREGISTRATION						= 5,
	AVP_IMS_SAR_TIMEOUT_DEREGISTRATION_STORE_SERVER_NAME= 6,
	AVP_IMS_SAR_USER_DEREGISTRATION_STORE_SERVER_NAME	= 7,
	AVP_IMS_SAR_ADMINISTRATIVE_DEREGISTRATION			= 8,
	AVP_IMS_SAR_AUTHENTICATION_FAILURE					= 9,
	AVP_IMS_SAR_AUTHENTICATION_TIMEOUT					= 10,
	AVP_IMS_SAR_DEREGISTRATION_TOO_MUCH_DATA			= 11
};

/** User-Data-Already-Available Enumerated AVP */
enum {
	AVP_IMS_SAR_USER_DATA_NOT_AVAILABLE					= 0,
	AVP_IMS_SAR_USER_DATA_ALREADY_AVAILABLE				= 1
};

/** User-Authorization-Type Enumerated AVP */
enum {
	AVP_IMS_UAR_REGISTRATION							= 0,
	AVP_IMS_UAR_DE_REGISTRATION							= 1,
	AVP_IMS_UAR_REGISTRATION_AND_CAPABILITIES			= 2
};

/** Originating-Request Enumerated AVP */
enum {
	AVP_IMS_LIR_ORIGINATING_REQUEST						= 0	
};

/** Data-Reference AVP */
enum {
	AVP_IMS_Data_Reference_Repository_Data				= 0,
	AVP_IMS_Data_Reference_IMS_Public_Identity			= 10,
	AVP_IMS_Data_Reference_IMS_User_State				= 11,
	AVP_IMS_Data_Reference_SCSCF_Name					= 12,
	AVP_IMS_Data_Reference_Initial_Filter_Criteria		= 13,
	AVP_IMS_Data_Reference_Location_Information			= 14,
	AVP_IMS_Data_Reference_User_State					= 15,
	AVP_IMS_Data_Reference_Charging_Information			= 16,
	AVP_IMS_Data_Reference_MSISDN						= 17,	
	AVP_IMS_Data_Reference_PSI_Activation				= 18,	
	AVP_IMS_Data_Reference_DSAI							= 19,	
	AVP_IMS_Data_Reference_Aliases_Repository_Data		= 20	
};

/** Subs-Req-Type AVP */
enum {
	AVP_IMS_Subs_Req_Type_Subscribe						= 0,
	AVP_IMS_Subs_Req_Type_Unubscribe					= 1
};

/** Requested-Domain AVP */
enum {
	AVP_IMS_Requested_Domain_CS							= 0,
	AVP_IMS_Requested_Domain_PS							= 1
};

/** Current-Location AVP */
enum {
	AVP_IMS_Current_Location_Do_Not_Need_Initiate_Active_Location_Retrieval	=0,
	AVP_IMS_Current_Location_Initiate_Active_Location_Retrieval				=1
};

/** Identity-Set AVP */
enum {
	AVP_IMS_Identity_Set_All_Identities					= 0,
	AVP_IMS_Identity_Set_Registered_Identities			= 1,
	AVP_IMS_Identity_Set_Implicit_Identities			= 2,	
	AVP_IMS_Identity_Set_Alias_Identities				= 3	
};

/** Deregistration-Reason AVP */
enum {
	AVP_IMS_Deregistration_Reason_Permanent_Termination	= 0,
	AVP_IMS_Deregistration_Reason_New_Server_Assigned	= 1,
	AVP_IMS_Deregistration_Reason_Server_Change			= 2,	
	AVP_IMS_Deregistration_Reason_Remove_S_CSCF			= 3
};



/** Abort-Cause AVP */
enum {
	AVP_IMS_Abort_Cause_Bearer_Released					= 0,
	AVP_IMS_Abort_Cause_Insufficient_Server_Resources	= 1,
	AVP_IMS_Abort_Cause_Insufficient_Bearer_Resources	= 2
};
/** Flow-Status AVP */
enum {
	AVP_IMS_Flow_Status_Enabled_Uplink					= 0,
	AVP_IMS_Flow_Status_Enabled_Downlink				= 1,
	AVP_IMS_Flow_Status_Enabled							= 2,
	AVP_IMS_Flow_Status_Disabled						= 3,
	AVP_IMS_Flow_Status_Removed							= 4
};
/** Flow-Usage AVP */
enum {
	AVP_IMS_Flow_Usage_No_Information					= 0,
	AVP_IMS_Flow_Usage_Rtcp								= 1
};
/** Specific-Action AVP */
enum {
	AVP_IMS_Specific_Action_Service_Information_Request						= 0,
	AVP_IMS_Specific_Action_Charging_Correlation_Exchange					= 1,
	AVP_IMS_Specific_Action_Indication_Of_Loss_Of_Bearer					= 2,
	AVP_IMS_Specific_Action_Indication_Of_Recovery_Of_Bearer				= 3,
	AVP_IMS_Specific_Action_Indication_Of_Release_Of_Bearer					= 4,
	AVP_IMS_Specific_Action_Indication_Of_Establishment_Of_Bearer			= 5
};
/** Media-Type AVP */
enum {
	AVP_IMS_Media_Type_Audio					= 0,
	AVP_IMS_Media_Type_Video					= 1,
	AVP_IMS_Media_Type_Data						= 2,
	AVP_IMS_Media_Type_Application				= 3,
	AVP_IMS_Media_Type_Control					= 4,
	AVP_IMS_Media_Type_Text						= 5,
	AVP_IMS_Media_Type_Message					= 6,
	AVP_IMS_Media_Type_Other					= 0xFFFFFFFF
};
/**	Diameter Result Codes				*/
enum {
	DIAMETER_SUCCESS									= 2001,//7D1
	DIAMETER_REALM_NOT_SERVED							= 3003,//0xBBB
	DIAMETER_AUTHENTICATION_REJECTED					= 4001,//FA1
	DIAMETER_AUTHORIZATION_REJECTED						= 5003,//138B
	DIAMETER_MISSING_AVP								= 5005,//0x138D
	DIAMETER_AVP_NOT_ALLOWED							= 5008,//0x140
	DIAMETER_AVP_OCCURS_TOO_MANY_TIMES 					= 5009,
	DIAMETER_NO_COMMON_APPLICATION						= 5010,//0x1392
	DIAMETER_UNABLE_TO_COMPLY							= 5012,//0x1394
	DIAMETER_NO_COMMON_SECURITY							= 5017,//0x1399
	DIAMETER_INVALID_AVP_VALUE							= 5040,//0x13B0
};

/**	IMS Specific Result Codes			*/
enum{
/** 1001 to 1999	Informational			*/
/** 2001 to 2999	Success					*/
/**	2001 to 2020 Reserved for TS29.229	*/
	RC_IMS_DIAMETER_FIRST_REGISTRATION 					= 2001,
	RC_IMS_DIAMETER_SUBSEQUENT_REGISTRATION				= 2002,
	RC_IMS_DIAMETER_UNREGISTERED_SERVICE				= 2003,
	RC_IMS_DIAMETER_SUCCESS_SERVER_NAME_NOT_STORED		= 2004,
	RC_IMS_DIAMETER_SERVER_SELECTION					= 2005,
/**	2401 to 2420 Reserved for TS29.109	*/
/** 4001 to 4999	Transient Failures	*/
/**	4100 to 4120 Reserved for TS29.329	*/
	RC_IMS_DIAMETER_USER_DATA_NOT_AVAILABLE 			= 4100,
	RC_IMS_DIAMETER_PRIOR_UPDATE_IN_PROGRESS			= 4101,
/**	41xx to 41yy Reserved for TS32.299	*/
/** 5001 to 5999	Permanent Failures		*/
/**	5001 to 5020 Reserved for TS29.229	*/
	RC_IMS_DIAMETER_ERROR_USER_UNKNOWN					= 5001,
	RC_IMS_DIAMETER_ERROR_IDENTITIES_DONT_MATCH			= 5002,
	RC_IMS_DIAMETER_ERROR_IDENTITY_NOT_REGISTERED		= 5003,
	RC_IMS_DIAMETER_ERROR_ROAMING_NOT_ALLOWED			= 5004,
	RC_IMS_DIAMETER_ERROR_IDENTITY_ALREADY_REGISTERED	= 5005,
	RC_IMS_DIAMETER_ERROR_AUTH_SCHEME_NOT_SUPPORTED		= 5006,
	RC_IMS_DIAMETER_ERROR_IN_ASSIGNMENT_TYPE			= 5007,
	RC_IMS_DIAMETER_ERROR_TOO_MUCH_DATA					= 5008,
	RC_IMS_DIAMETER_ERROR_NOT_SUPPORTED_USER_DATA		= 5009,
	RC_IMS_DIAMETER_MISSING_USER_ID						= 5010,
	RC_IMS_DIAMETER_ERROR_FEATURE_UNSUPPORTED			= 5011,
/**	5021 to 5040 Reserved for TS32.299	*/
/**	5041 to 5060 Reserved for TS29.234	*/
/**	5061 to 5080 Reserved for TS29.209	*/
	RC_IMS_DIAMETER_ERROR_INVALID_SERVICE_INFORMATION	= 5061,
	RC_IMS_DIAMETER_ERROR_FILTER_RESTRICTIONS			= 5062,
/**	5100 to 5119 Reserved for TS29.329	*/
	RC_IMS_DIAMETER_ERROR_USER_DATA_NOT_RECOGNIZED		= 5100,
	RC_IMS_DIAMETER_ERROR_OPERATION_NOT_ALLOWED			= 5101,
	RC_IMS_DIAMETER_ERROR_USER_DATA_CANNOT_BE_READ		= 5102,
	RC_IMS_DIAMETER_ERROR_USER_DATA_CANNOT_BE_MODIFIED	= 5103,
	RC_IMS_DIAMETER_ERROR_USER_DATA_CANNOT_BE_NOTIFIED	= 5104,
	RC_IMS_DIAMETER_ERROR_TRANSPARENT_DATA_OUT_OF_SYNC	= 5105
/** 5400 to 5419 Reserved for TS29.109	*/
};

/** Node-Functionality AVP */
enum {
	AVP_IMS_S_CSCF										= 0,
	AVP_IMS_P_CSCF										= 1,
	AVP_IMS_I_CSCF										= 2,
	AVP_IMS_MRFC										= 3,
	AVP_IMS_MGCF										= 4,
	AVP_IMS_BGCF										= 5,
	AVP_IMS_AS											= 6
};

/** Role-Of-Node */
enum {
	AVP_IMS_ORIGINATING_ROLE							= 0,
	AVP_IMS_TERMINATING_ROLE							= 1,
	AVP_IMS_PROXY_ROLE									= 2,
	AVP_IMS_B2BUA_ROLE									= 3
};						



typedef enum
{
   Async									 = 0,
   Sync 										 = 1, 
   ISDN_Sync 								 = 2, 
   ISDN_Async_V120 					 = 3,
   ISDN_Async_V110					 = 4,
   Virtual									 = 5, 
   PIAFS									 	 = 6, 	
   HDLC_Clear_Channel				 = 7, 
   X_25										 = 8, 
   X_75										 = 9, 
   G_3_Fax									 =10,
   Symmetric_DSL						 =11, 	
   ADSL										 =12,
   ADSL_DMT								 =13, 
   IDSL										 =14, 
   Ethernet									 =15, 
   xDSL										 =16, 
   Cable										 =17, 
   Wireless_Other						 =18,
   Wireless_IEEE_802_11				 =19,
   Token_Ring								 =20, 
   FDDI										 =21,
   Wireless_CDMA2000				     =22,
   Wireless_UMTS						 =23,
   Wireless_1X_EV						 =24,
   IAPP  									 =25
}	nas_port_type;						

/*
access-info for each access type  
"ADSL" / "ADSL2" / "ADSL2+" / "RADSL" / "SDSL" / "HDSL" / "HDSL2" / "G.SHDSL" / "VDSL" / "IDSL"  -> dsl-location
"3GPP-GERAN" -> cgi-3gpp
"3GPP-UTRAN-FDD" / "3GPP-UTRAN-TDD" -> utran-cell-id-3gpp
"3GPP2-1X" / "3GPP2-1X-HRPD" -> ci-3gpp2
"IEEE-802.11" / "IEEE-802.11a" / "IEEE-802.11b" / "IEEE-802.11g" -> i-wlan-node-id = MAC
"DOCSIS" -> NULL
*/

#endif /* __DIAMETER_IMS_H */
