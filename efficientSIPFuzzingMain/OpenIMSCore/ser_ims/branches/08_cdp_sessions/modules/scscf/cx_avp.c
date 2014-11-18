/**
 * $Id: cx_avp.c 358 2007-06-28 16:51:14Z flp $
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
 * I/S-CSCF Module - Cx AVP Operations 
 * 
 * Scope:
 * - Defining Operations between I/S-CSCF <-> HSS
 *   
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 * Copyright (C) 2005 FhG Fokus
 * 
 */

#include "cx_avp.h"
#include "../../mem/shm_mem.h"
#include <stdio.h>

extern struct cdp_binds cdpb;            /**< Structure with pointers to cdp funcs 		*/

/**
 * Create and add an AVP to a Diameter message.
 * @param m - Diameter message to add to 
 * @param d - the payload data
 * @param len - length of the payload data
 * @param avp_code - the code of the AVP
 * @param flags - flags for the AVP
 * @param vendorid - the value of the vendor id or 0 if none
 * @param data_do - what to do with the data when done
 * @param func - the name of the calling function, for debugging purposes
 * @returns 1 on success or 0 on failure
 */
static inline int Cx_add_avp(AAAMessage *m,char *d,int len,int avp_code,
	int flags,int vendorid,int data_do,const char *func)
{
	AAA_AVP *avp;
	if (vendorid!=0) flags |= AAA_AVP_FLAG_VENDOR_SPECIFIC;
	avp = cdpb.AAACreateAVP(avp_code,flags,vendorid,d,len,data_do);
	if (!avp) {
		LOG(L_ERR,"ERR:"M_NAME":%s: Failed creating avp\n",func);
		return 0;
	}
	if (cdpb.AAAAddAVPToMessage(m,avp,m->avpList.tail)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERR:"M_NAME":%s: Failed adding avp to message\n",func);
		cdpb.AAAFreeAVP(&avp);
		return 0;
	}
	return 1;
}

/**
 * Create and add an AVP to a list of AVPs.
 * @param list - the AVP list to add to 
 * @param d - the payload data
 * @param len - length of the payload data
 * @param avp_code - the code of the AVP
 * @param flags - flags for the AVP
 * @param vendorid - the value of the vendor id or 0 if none
 * @param data_do - what to do with the data when done
 * @param func - the name of the calling function, for debugging purposes
 * @returns 1 on success or 0 on failure
 */
static inline int Cx_add_avp_list(AAA_AVP_LIST *list,char *d,int len,int avp_code,
	int flags,int vendorid,int data_do,const char *func)
{
	AAA_AVP *avp;
	if (vendorid!=0) flags |= AAA_AVP_FLAG_VENDOR_SPECIFIC;
	avp = cdpb.AAACreateAVP(avp_code,flags,vendorid,d,len,data_do);
	if (!avp) {
		LOG(L_ERR,"ERR:"M_NAME":%s: Failed creating avp\n",func);
		return 0;
	}
	if (list->tail) {
		avp->prev=list->tail;
		avp->next=0;	
		list->tail->next = avp;
		list->tail=avp;
	} else {
		list->head = avp;
		list->tail = avp;
		avp->next=0;
		avp->prev=0;
	}
	
	return 1;
}

/**
 * Returns the value of a certain AVP from a Diameter message.
 * @param m - Diameter message to look into
 * @param avp_code - the code to search for
 * @param vendorid - the value of the vendor id to look for or 0 if none
 * @param func - the name of the calling function, for debugging purposes
 * @returns the str with the payload on success or an empty string on failure
 */
static inline str Cx_get_avp(AAAMessage *msg,int avp_code,int vendor_id,
							const char *func)
{
	AAA_AVP *avp;
	str r={0,0};
	
	avp = cdpb.AAAFindMatchingAVP(msg,0,avp_code,vendor_id,0);
	if (avp==0){
		LOG(L_INFO,"INFO:"M_NAME":%s: Failed finding avp\n",func);
		return r;
	}
	else 
		return avp->data;
}




/**
 * Creates and adds a User-Name AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_user_name(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_User_Name,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Public Identity AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_public_identity(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_IMS_Public_Identity,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Visited-Network-ID AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_visited_network_id(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_IMS_Visited_Network_Identifier,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Authorization-Type AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_authorization_type(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_IMS_User_Authorization_Type,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}


/**
 * Creates and adds a Server-Name AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_server_name(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_IMS_Server_Name,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a SIP-Number-Auth-Items AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_sip_number_auth_items(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_IMS_SIP_Number_Auth_Items,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}


static str s_empty = {0, 0};
/**
 * Creates and adds a SIP-Auth-Data-Item AVP.
 * @param msg - the Diameter message to add to.
 * @param auth_scheme - the value for the authorization scheme AVP
 * @param auth - the value for the authorization AVP
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_sip_auth_data_item_request(AAAMessage *msg, str auth_scheme, str auth, str username, str realm,str method, str server_name)
{
	AAA_AVP_LIST list;
	str group;
	str etsi_authorization = {0, 0};
	list.head=0;list.tail=0;
		
	if (auth_scheme.len){
		Cx_add_avp_list(&list,
			auth_scheme.s,auth_scheme.len,
			AVP_IMS_SIP_Authentication_Scheme,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_3GPP,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}	
	if (auth.len){
		Cx_add_avp_list(&list,
			auth.s,auth.len,
			AVP_IMS_SIP_Authorization,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_3GPP,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}

	if (server_name.len) 
	{
		etsi_authorization = Cx_ETSI_sip_authorization(username, realm, s_empty, server_name, s_empty, s_empty, method, s_empty);
	
		if (etsi_authorization.len){
			Cx_add_avp_list(&list,
				etsi_authorization.s,etsi_authorization.len,
				AVP_ETSI_SIP_Authorization,
				AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
				IMS_vendor_id_ETSI,
				AVP_FREE_DATA,
				__FUNCTION__);
		}	
	}

	if (!list.head) return 1;
	group = cdpb.AAAGroupAVPS(list);
	
	cdpb.AAAFreeAVPList(&list);
	
	return 
	Cx_add_avp(msg,group.s,group.len,
		AVP_IMS_SIP_Auth_Data_Item,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id_3GPP,
		AVP_FREE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a ETSI_sip_authorization AVP.
 * @param username - UserName
 * @param realm - Realm
 * @param nonce - Nonce
 * @param URI - URI
 * @param response - Response
 * @param algoritm - Algorithm
 * @param method - Method
 * @param hash - Enitity-Body-Hash
 * @returns grouped str on success
 */
str Cx_ETSI_sip_authorization(str username, str realm, str nonce, str URI, str response, str algorithm, str method, str hash)
{
	AAA_AVP_LIST list;
	str group = {0, 0};
	list.head=0;list.tail=0;
		
	if (username.len){
		Cx_add_avp_list(&list,
			username.s,username.len,
			AVP_ETSI_Digest_Username,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_ETSI,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}	

	if (realm.len){
		Cx_add_avp_list(&list,
			realm.s,realm.len,
			AVP_ETSI_Digest_Realm,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_ETSI,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}	
	
	if (nonce.len){
		Cx_add_avp_list(&list,
			nonce.s,nonce.len,
			AVP_ETSI_Digest_Nonce,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_ETSI,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}

	if (URI.len){
		Cx_add_avp_list(&list,
			URI.s,URI.len,
			AVP_ETSI_Digest_URI,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_ETSI,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}

	if (response.len){
		Cx_add_avp_list(&list,
			response.s,response.len,
			AVP_ETSI_Digest_Response,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_ETSI,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}

	if (algorithm.len){
		Cx_add_avp_list(&list,
			algorithm.s,algorithm.len,
			AVP_ETSI_Digest_Algorithm,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_ETSI,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}

	if (method.len){
		Cx_add_avp_list(&list,
			method.s,method.len,
			AVP_ETSI_Digest_Method,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_ETSI,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}

	if (hash.len){
		Cx_add_avp_list(&list,
			hash.s,hash.len,
			AVP_ETSI_Digest_Entity_Body_Hash,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_ETSI,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}

	if (!list.head) return group;
	group = cdpb.AAAGroupAVPS(list);
	
	cdpb.AAAFreeAVPList(&list);
	
	return group;
}

/**
 * Creates and adds a Server-Assignment-Type AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_server_assignment_type(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_IMS_Server_Assignment_Type,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds Userdata-Available AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_userdata_available(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_IMS_User_Data_Already_Available,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Result-Code AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_result_code(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_Result_Code,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Experimental-Result-Code AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_experimental_result_code(AAAMessage *msg,unsigned int data)
{
	AAA_AVP_LIST list;
	str group;
	char x[4];
	list.head=0;list.tail=0;
		
	set_4bytes(x,data);
	Cx_add_avp_list(&list,
		x,4,
		AVP_IMS_Experimental_Result_Code,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
	set_4bytes(x,IMS_vendor_id_3GPP);
	Cx_add_avp_list(&list,
		x,4,
		AVP_IMS_Vendor_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
	
	group = cdpb.AAAGroupAVPS(list);
	
	cdpb.AAAFreeAVPList(&list);
	
	return 
	Cx_add_avp(msg,group.s,group.len,
		AVP_IMS_Experimental_Result,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_FREE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Vendor-Specifig-Application-ID AVP.
 * @param msg - the Diameter message to add to.
 * @param vendor_id - the value of the vendor_id,
 * @param auth_id - the authorization application id
 * @param acct_id - the accounting application id
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_vendor_specific_appid(AAAMessage *msg,unsigned int vendor_id,
	unsigned int auth_id,unsigned int acct_id)
{
	AAA_AVP_LIST list;
	str group;
	char x[4];

	list.head=0;list.tail=0;
		
	set_4bytes(x,vendor_id);
	Cx_add_avp_list(&list,
		x,4,
		AVP_Vendor_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);

	if (auth_id) {
		set_4bytes(x,auth_id);
		Cx_add_avp_list(&list,
			x,4,
			AVP_Auth_Application_Id,
			AAA_AVP_FLAG_MANDATORY,
			0,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
	}
	if (acct_id) {
		set_4bytes(x,acct_id);
		Cx_add_avp_list(&list,
			x,4,
			AVP_Acct_Application_Id,
			AAA_AVP_FLAG_MANDATORY,
			0,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
	}	
	
	group = cdpb.AAAGroupAVPS(list);
	
	cdpb.AAAFreeAVPList(&list);
	
	return 
	Cx_add_avp(msg,group.s,group.len,
		AVP_Vendor_Specific_Application_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_FREE_DATA,
		__FUNCTION__);
}


/**
 * Creates and adds a Auth-Session-State AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_auth_session_state(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_Auth_Session_State,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Destination-Realm AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Cx_add_destination_realm(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_Destination_Realm,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}


/**
 * Returns the Session-Id AVP of a Diameter message.
 * @param msg - the Diameter message
 * @returns AVP payload on success or an empty string on error
 */
inline str Cx_get_session_id(AAAMessage *msg)
{
	return Cx_get_avp(msg,
		AVP_Session_Id,
		0,
		__FUNCTION__);
}


/**
 * Returns the User-Name AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline str Cx_get_user_name(AAAMessage *msg)
{
	return Cx_get_avp(msg,
		AVP_User_Name,
		0,
		__FUNCTION__);
}

/**
 * Returns the Public-Identity AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline str Cx_get_public_identity(AAAMessage *msg)
{
	return Cx_get_avp(msg,
		AVP_IMS_Public_Identity,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
}

/**
 * Finds out the next Public-Identity AVP from a Diameter message.
 * @param msg - the Diameter message
 * @param pos - position to resume search or NULL if to start from the first AVP 
 * @param avp_code - the code of the AVP to look for
 * @param vendor_id - the vendor id of the AVP to look for
 * @param func - the name of the calling function for debugging purposes
 * @returns the AVP payload on success or an empty string on error
 */
inline AAA_AVP* Cx_get_next_public_identity(AAAMessage *msg,AAA_AVP* pos,int avp_code,int vendor_id,const char *func)
{		
	AAA_AVP *avp;
	
	avp = cdpb.AAAFindMatchingAVP(msg,pos,avp_code,vendor_id,0);
	if (avp==0){
		LOG(L_INFO,"INFO:"M_NAME":%s: Failed finding avp\n",func);
		return avp;
	}
	else 
		return avp;
}

/**
 * Returns the Visited-Network-ID AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline str Cx_get_visited_network_id(AAAMessage *msg)
{
	return Cx_get_avp(msg,
		AVP_IMS_Visited_Network_Identifier,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
}

/**
 * Returns the Authorization-Type AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline int Cx_get_authorization_type(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_IMS_User_Authorization_Type,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}

/**
 * Returns the Server-Assignment-Type AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline int Cx_get_server_assignment_type(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_IMS_Server_Assignment_Type,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}

/**
 * Returns the User-Data-Available AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline int Cx_get_userdata_available(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_IMS_User_Data_Already_Available,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}


/**
 * Returns the Result-Code AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline int Cx_get_result_code(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_Result_Code,
		0,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}

/**
 * Returns the Experimental-Result-Code AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline int Cx_get_experimental_result_code(AAAMessage *msg, int *data)
{
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	grp = Cx_get_avp(msg,
		AVP_IMS_Experimental_Result,
		0,
		__FUNCTION__);
	if (!grp.s) return 0;

	list = cdpb.AAAUngroupAVPS(grp);
	
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Experimental_Result_Code,0,0);
	if (!avp||!avp->data.s) {
		cdpb.AAAFreeAVPList(&list);
		return 0;
	}

	*data = get_4bytes(avp->data.s);
	cdpb.AAAFreeAVPList(&list);

	return 1;
}

/**
 * Returns the Server-Name AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline str Cx_get_server_name(AAAMessage *msg)
{	
	return Cx_get_avp(msg,
		AVP_IMS_Server_Name,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
}


/**
 * Returns the Capabilities from the grouped AVP from a Diameter message.
 * @param msg - the Diameter message
 * @param m - array to be filled with the retrieved mandatory capabilities
 * @param m_cnt - size of the array above to be filled
 * @param o - array to be filled with the retrieved optional capabilities
 * @param o_cnt - size of the array above to be filled
 * @returns 1 on success 0 on fail
 */
inline int Cx_get_capabilities(AAAMessage *msg,int **m,int *m_cnt,int **o,int *o_cnt)
{
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	grp = Cx_get_avp(msg,
		AVP_IMS_Server_Capabilities,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
	if (!grp.s) return 0;

	list = cdpb.AAAUngroupAVPS(grp);
	
	avp = list.head;
	*m_cnt=0;
	*o_cnt=0;
	while(avp){
		if (avp->code == AVP_IMS_Mandatory_Capability) (*m_cnt)++;
		if (avp->code == AVP_IMS_Optional_Capability) (*o_cnt)++;		
		avp = avp->next;
	}
	avp = list.head;
	*m=shm_malloc(sizeof(int)*(*m_cnt));
	*o=shm_malloc(sizeof(int)*(*o_cnt));
	*m_cnt=0;
	*o_cnt=0;
	while(avp){
		if (avp->code == AVP_IMS_Mandatory_Capability) 
			(*m)[(*m_cnt)++]=get_4bytes(avp->data.s);
		if (avp->code == AVP_IMS_Optional_Capability)		
			(*o)[(*o_cnt)++]=get_4bytes(avp->data.s);
		avp = avp->next;
	}
	cdpb.AAAFreeAVPList(&list);
	return 1;
}

/**
 * Returns the SIP-Number-Auth-Items AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the number or 0 on error
 */
inline int Cx_get_sip_number_auth_items(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_IMS_SIP_Number_Auth_Items,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}


/**
 * Returns the Auth-Data-Item from a Diameter Request message.
 * @param msg - the Diameter message
 * @param auth_scheme - the string to fill with the authorization scheme
 * @param authorization - the string to fill with the authorization
 * @returns the AVP payload on success or an empty string on error
 */
inline int Cx_get_auth_data_item_request(AAAMessage *msg,
		 str *auth_scheme, str *authorization)
{
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	grp = Cx_get_avp(msg,
		AVP_IMS_SIP_Auth_Data_Item,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
	if (!grp.s) return 0;

	list = cdpb.AAAUngroupAVPS(grp);
	
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authentication_Scheme,
		IMS_vendor_id_3GPP,0);
	if (!avp||!avp->data.s) {
		cdpb.AAAFreeAVPList(&list);
		return 0;
	}
	*auth_scheme = avp->data;
	
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authorization,
		IMS_vendor_id_3GPP,0);
	if (avp) *authorization = avp->data;
	else {authorization->s=0;authorization->len=0;}		

	cdpb.AAAFreeAVPList(&list);
	return 1;
}

/**
 * Returns the Auth-Data-Item from a Diameter answer message.
 * @param msg - the Diameter message
 * @param auth_date - the string to fill with the authorization data
 * @param item_number - the int to fill with the item number
 * @param auth_scheme - the string to fill with the authentication scheme data
 * @param authenticate - the string to fill with the authenticate data
 * @param authorization - the string to fill with the authorization data
 * @param ck - the string to fill with the cipher key
 * @param ik - the string to fill with the integrity key
 * @returns the AVP payload on success or an empty string on error
 */
int Cx_get_auth_data_item_answer(AAAMessage *msg, AAA_AVP **auth_data,
	int *item_number,str *auth_scheme,str *authenticate,str *authorization,
	str *ck,str *ik,str *ip, str *ha1, str *response_auth)
{
	AAA_AVP_LIST list;
	AAA_AVP_LIST list2;
	AAA_AVP *avp;
	AAA_AVP *avp2;
	str grp;
	static char buf[64];
	ha1->s = 0; ha1->len = 0;
	*auth_data = cdpb.AAAFindMatchingAVP(msg,*auth_data,AVP_IMS_SIP_Auth_Data_Item,
		IMS_vendor_id_3GPP,0);
	if (!*auth_data) return 0;
		
	grp = (*auth_data)->data;
	if (!grp.len) return 0;

	list = cdpb.AAAUngroupAVPS(grp);

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Item_Number,
		IMS_vendor_id_3GPP,0);
	if (!avp||!avp->data.len==4) *item_number=0;
	else *item_number = get_4bytes(avp->data.s);
	
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authentication_Scheme,
		IMS_vendor_id_3GPP,0);
	if (!avp||!avp->data.s) {auth_scheme->s=0;auth_scheme->len=0;}
	else *auth_scheme = avp->data;

	/* Early-IMS */
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_Framed_IP_Address,0,0);
	if (!avp||!avp->data.s) {ip->s=0;ip->len=0;}
	else {
		sprintf(buf,"%u.%u.%u.%u",(unsigned char)avp->data.s[2],(unsigned char)avp->data.s[3],(unsigned char)avp->data.s[4],(unsigned char)avp->data.s[5]);
		ip->len = strlen(buf);
		ip->s = buf;
	}

	/* Digest */
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_CableLabs_Digest_HA1,IMS_vendor_id_CableLabs,0);
	if (!avp||!avp->data.s) {ha1->s=0;ha1->len=0;}
	else *ha1 = avp->data;
	
	
	/* AKA, MD5 */
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authenticate,
		IMS_vendor_id_3GPP,0);
	if (!avp||!avp->data.s) {authenticate->s=0;authenticate->len=0;}
	else *authenticate = avp->data;
		
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authorization,
		IMS_vendor_id_3GPP,0);
	if (!avp||!avp->data.s) {authorization->s=0;authorization->len=0;}
	else *authorization = avp->data;	

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Confidentiality_Key,
		IMS_vendor_id_3GPP,0);
	if (!avp||!avp->data.s) {ck->s=0;ck->len=0;}
	else *ck = avp->data;

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Integrity_Key,
		IMS_vendor_id_3GPP,0);
	if (!avp||!avp->data.s) {ik->s=0;ik->len=0;}
	else *ik = avp->data;

	/* ETSI HTTP Digest */

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_ETSI_SIP_Authenticate,IMS_vendor_id_ETSI,0);
	if (avp  && avp->data.s) 
	{
		list2 = cdpb.AAAUngroupAVPS(avp->data);
		
		avp2 = cdpb.AAAFindMatchingAVPList(list2,0,AVP_ETSI_Digest_Nonce, IMS_vendor_id_ETSI,0);
		if (!avp2||!avp2->data.s) {
			authenticate->s=0;authenticate->len=0;
			cdpb.AAAFreeAVPList(&list2);
			return 0;
		}
		*authenticate = avp2->data;
		
		avp2 = cdpb.AAAFindMatchingAVPList(list2,0,AVP_ETSI_Digest_HA1, IMS_vendor_id_ETSI,0);
		if (!avp2||!avp2->data.s) {
			ha1->s = 0; ha1->len = 0;
			cdpb.AAAFreeAVPList(&list2);
			return 0;
		}
		*ha1 = avp2->data;
		
		cdpb.AAAFreeAVPList(&list2);
	}

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_ETSI_SIP_Authentication_Info,IMS_vendor_id_ETSI,0);
	if (avp  && avp->data.s) 
	{
		list2 = cdpb.AAAUngroupAVPS(avp->data);
		
		avp2 = cdpb.AAAFindMatchingAVPList(list2,0,AVP_ETSI_Digest_Response_Auth, IMS_vendor_id_ETSI,0);
		if (!avp2||!avp2->data.s) {
			response_auth->s=0;response_auth->len=0;
			cdpb.AAAFreeAVPList(&list2);
			return 0;
		}
		*response_auth = avp2->data;
		cdpb.AAAFreeAVPList(&list2);
	}
	else
	{
		response_auth->s=0;response_auth->len=0;
	}
	cdpb.AAAFreeAVPList(&list);
	return 1;
}


/**
 * Returns the Destination-Host from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline str Cx_get_destination_host(AAAMessage *msg)
{	
	return Cx_get_avp(msg,
		AVP_Destination_Host,
		0,
		__FUNCTION__);
}

/**
 * Returns the User-Data from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline str Cx_get_user_data(AAAMessage *msg)
{	
	return Cx_get_avp(msg,
		AVP_IMS_User_Data,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
}

/**
 * Returns the Charging-Information from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline int Cx_get_charging_info(AAAMessage *msg,str *ccf1,str *ccf2,str *ecf1,str *ecf2)
{		
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	grp = Cx_get_avp(msg,
		AVP_IMS_Charging_Information,
		IMS_vendor_id_3GPP,
		__FUNCTION__);
	if (!grp.s) return 0;

	list = cdpb.AAAUngroupAVPS(grp);
	
	if (ccf1){
		avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Primary_Charging_Collection_Function_Name,
			IMS_vendor_id_3GPP,0);
		if (avp) *ccf1 = avp->data;
	}		
	if (ccf2){
		avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Secondary_Charging_Collection_Function_Name,
			IMS_vendor_id_3GPP,0);
		if (avp) *ccf2 = avp->data;
	}		
	if (ecf1){
		avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Primary_Event_Charging_Function_Name,
			IMS_vendor_id_3GPP,0);
		if (avp) *ecf1 = avp->data;
	}		
	if (ecf2){
		avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Secondary_Event_Charging_Function_Name,
			IMS_vendor_id_3GPP,0);
		if (avp) *ecf2 = avp->data;
	}		
		
	cdpb.AAAFreeAVPList(&list);
	return 1;		
		
}
