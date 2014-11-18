/**
 * $Id: e2_avp.c 299 2007-05-31 18:19:30Z vingarzan $
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
 * P-CSCF Module - e2 AVP Operations 
 * 
 * Scope:
 * - Defining Operations between P-CSCF <-> HSS
 *   
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 * Copyright (C) 2005 FhG Fokus
 * 
 */

#include "e2_avp.h"
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
static inline int e2_add_avp(AAAMessage *m,char *d,int len,int avp_code,
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
static inline int e2_add_avp_list(AAA_AVP_LIST *list,char *d,int len,int avp_code,
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
static inline str e2_get_avp(AAAMessage *msg,int avp_code,int vendor_id,
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
inline int e2_add_user_name(AAAMessage *msg,str data)
{
	return 
	e2_add_avp(msg,data.s,data.len,
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
inline int e2_add_public_identity(AAAMessage *msg,str data)
{
	return 
	e2_add_avp(msg,data.s,data.len,
		AVP_IMS_Public_Identity,
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
inline int e2_add_authorization_type(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	e2_add_avp(msg,x,4,
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
inline int e2_add_server_name(AAAMessage *msg,str data)
{
	return 
	e2_add_avp(msg,data.s,data.len,
		AVP_IMS_Server_Name,
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
inline int e2_add_result_code(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	e2_add_avp(msg,x,4,
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
inline int e2_add_experimental_result_code(AAAMessage *msg,unsigned int data)
{
	AAA_AVP_LIST list;
	str group;
	char x[4];
	list.head=0;list.tail=0;
		
	set_4bytes(x,data);
	e2_add_avp_list(&list,
		x,4,
		AVP_IMS_Experimental_Result_Code,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
	set_4bytes(x,IMS_vendor_id_3GPP);
	e2_add_avp_list(&list,
		x,4,
		AVP_IMS_Vendor_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
	
	group = cdpb.AAAGroupAVPS(list);
	
	cdpb.AAAFreeAVPList(&list);
	
	return 
	e2_add_avp(msg,group.s,group.len,
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
inline int e2_add_vendor_specific_appid(AAAMessage *msg,unsigned int vendor_id,
	unsigned int auth_id,unsigned int acct_id)
{
	AAA_AVP_LIST list;
	str group;
	char x[4];

	list.head=0;list.tail=0;
		
	set_4bytes(x,vendor_id);
	e2_add_avp_list(&list,
		x,4,
		AVP_Vendor_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);

	if (auth_id) {
		set_4bytes(x,auth_id);
		e2_add_avp_list(&list,
			x,4,
			AVP_Auth_Application_Id,
			AAA_AVP_FLAG_MANDATORY,
			0,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
	}
	if (acct_id) {
		set_4bytes(x,acct_id);
		e2_add_avp_list(&list,
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
	e2_add_avp(msg,group.s,group.len,
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
inline int e2_add_auth_session_state(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	e2_add_avp(msg,x,4,
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
inline int e2_add_destination_realm(AAAMessage *msg,str data)
{
	return 
	e2_add_avp(msg,data.s,data.len,
		AVP_Destination_Realm,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a AVP_Framed_IP_Address and a AVP_ETSI_Address_Realm to a  AVP_ETSI_Globally_Unique_Address group.
 * @param msg - the Diameter message to add to.
 * @param ip - ue ip address
 * @param realm - realm
 * @returns 1 on success or 0 on error
 */
inline int e2_add_g_unique_address(AAAMessage *msg, str ip,str realm) 
{
	AAA_AVP_LIST list;
	str group;
	char x[6];
	int i,j,k;
	list.head=0;list.tail=0;
	
	if (ip.len>0){		
		memset(x,0,4);
		x[1]=1;
		i=2;k=0;		
		for(j=0;j<ip.len;j++){
			if (ip.s[j]=='.') {x[i++]=k;k=0;}
			else if (ip.s[j]>='0' && ip.s[j]<='9')
					k = k*10 + ip.s[j]-'0';
		}
		x[i]=k;		
		
		e2_add_avp_list(&list,
			x,6,
			AVP_Framed_IP_Address,
			0,
			0,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
	}
	
	if (realm.len)
	{
		e2_add_avp_list(&list,
			realm.s,realm.len,
			AVP_ETSI_Address_Realm,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_ETSI,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
	}
	
	group = cdpb.AAAGroupAVPS(list);
	
	cdpb.AAAFreeAVPList(&list);
	
	return 
	e2_add_avp(msg,group.s,group.len,
		AVP_ETSI_Globally_Unique_Address,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id_ETSI,
		AVP_FREE_DATA,
		__FUNCTION__);
}


/**
 * Creates and adds AVP_IMS_AF_Application_Identifier
 * @param msg - the Diameter message to add to.
 * @param data - application_Identifier
 * @returns 1 on success or 0 on error
 */
inline int e2_add_app_identifier(AAAMessage *msg, str data) 
{
	return 
		e2_add_avp(msg,data.s,data.len,
			AVP_IMS_AF_Application_Identifier,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id_3GPP,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
}


/**
 * Returns the Session-Id AVP of a Diameter message.
 * @param msg - the Diameter message
 * @returns AVP payload on success or an empty string on error
 */
inline str e2_get_session_id(AAAMessage *msg)
{
	return e2_get_avp(msg,
		AVP_Session_Id,
		0,
		__FUNCTION__);
}


/**
 * Returns the User-Name AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline str e2_get_user_name(AAAMessage *msg)
{
	return e2_get_avp(msg,
		AVP_User_Name,
		0,
		__FUNCTION__);
}

inline str e2_get_terminal_type(AAAMessage *msg)
{
	return 
	e2_get_avp(msg,
		AVP_ETSI_Terminal_Type,
		IMS_vendor_id_ETSI,
		__FUNCTION__);
}


int e2_get_access_net(AAAMessage *msg, int *data)
{
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	grp = e2_get_avp(msg,
		AVP_ETSI_Access_Network_Type,
		IMS_vendor_id_ETSI,
		__FUNCTION__);
	
	if (!grp.s) return 0;

	list = cdpb.AAAUngroupAVPS(grp);

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_NAS_Port_Type,0,0);
	if (!avp||!avp->data.s) {
		cdpb.AAAFreeAVPList(&list);
		return 0;
	}	
	*data = atoi(avp->data.s);
	return 1;
}

int e2_get_location_info(AAAMessage *msg, str *data)
{
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	data->len = 0;
	grp = e2_get_avp(msg,
		 AVP_ETSI_Location_Information,
		IMS_vendor_id_ETSI,
		__FUNCTION__);
	
	if (!grp.s) return 0;

	list = cdpb.AAAUngroupAVPS(grp);

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_Line_Identifier,IMS_vendor_id_ETSI,0);
	if (!avp||!avp->data.s) {
		cdpb.AAAFreeAVPList(&list);
		return 0;
	}
	
	*data = avp->data;
	return 1;
}
