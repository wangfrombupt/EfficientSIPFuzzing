/**
 * $Id$
 *  
 * Copyright (C) 2004-2007 FhG Fokus
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
 * P/S/(I)-CSCF Module - Rf AVP Operations
 * 
 * Scope:
 * - Defining Operations between P/S/(I)-CSCF <-> CDF (Charging Data Function)
 *   
 * 
 *  \author Shengyao Chen  shc -at- fokus dot fraunhofer dot de
 *
 * Copyright (C) 2007 FhG Fokus
 * 
 */

#include "rf_avp.h"
#include "../../mem/shm_mem.h"

/**< Structure with pointers to cdp funcs, global variable defined in mod.c  */
extern struct cdp_binds cdpb;

/*
 *******************************************************************************
 * Basic operations to modify an AVP, see also Cx_avp.c
 * 
 *******************************************************************************
 */

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
static inline int Rf_add_avp(AAAMessage *m,char *d,int len,int avp_code,
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
static inline int Rf_add_avp_list(AAA_AVP_LIST *list,char *d,int len,int avp_code,
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
static inline str Rf_get_avp(AAAMessage *msg,int avp_code,int vendor_id,
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

/*
 *******************************************************************************
 * Create and add AVPs to a Diameter message.
 * 
 *******************************************************************************
 */

/**
 * Creates and adds a Destination-Realm AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Rf_add_destination_realm(AAAMessage *msg, str data)
{
	return 
	Rf_add_avp(msg,data.s,data.len,
		AVP_Destination_Realm,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds an Acct-Application-Id AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @return 1 on success or 0 on error
 */
inline int Rf_add_acct_application_id(AAAMessage *msg, unsigned int data)
{
	char x[4];
	set_4bytes(x, data);	

	return
	Rf_add_avp(msg, x, 4,
			AVP_Acct_Application_Id,
			AAA_AVP_FLAG_MANDATORY,
			0,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
}
 

/**
 * Creates and adds a Accounting-Record-Type AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 * 
 * TODO should be added by base protocol
 */
inline int Rf_add_accounting_record_type(AAAMessage* msg, unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	
	return
	Rf_add_avp(msg, x, 4,
		AVP_Accounting_Record_Type,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/**
 * Creates and adds a Accounting-Number-Type AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 * 
 * TODO should be added by base protocol
 */
inline int Rf_add_accounting_record_number(AAAMessage* msg, unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	
	return
	Rf_add_avp(msg, x, 4,
		AVP_Accounting_Record_Number,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Create and adds a Service-Context-Id AVP. 
 *
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Rf_add_service_context_id(AAAMessage *msg, str data)
{
	return
	Rf_add_avp(msg, data.s, data.len,
		AVP_Service_Context_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/*
 *******************************************************************************
 * Create and add 3GPP specific AVPs to a AVP list. These AVPs are member AVPs
 * of IMS-Information AVP. See TS 32.299 V7.4.0
 *******************************************************************************
 */

/** 
 * Creates and adds Role-Of-Node AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_role_of_node(AAA_AVP_LIST* list, unsigned int data)
{
	char x[4];
	set_4bytes(x, data);
	
	
	return
	Rf_add_avp_list(list,
		x,4,
		AVP_IMS_Role_Of_Node,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds SIP-Method AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_sip_method(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_SIP_Method,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Event AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_event(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_Event,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Event-Type AVP to an AVP list. 
 *
 * @param outl - points at this IMS-Information AVP.
 * @param inl - AVPs list containing member AVPs. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_ims_information(AAA_AVP_LIST* outl, AAA_AVP_LIST* inl)
{
	str group;
	
	group = cdpb.AAAGroupAVPS(*inl);
	cdpb.AAAFreeAVPList(inl);
	
	return 
	Rf_add_avp_list(outl,
		group.s, group.len,
		AVP_IMS_IMS_Information,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_FREE_DATA,
		__FUNCTION__);
}	



/** 
 * Creates and adds Node-Functionality AVP to an AVP list. 
 * @param list - the AVP list to add to. 
 * @param data - the value for the AVP payload.
 * @return 1 on success or 0 on error 
 */
inline int Rf_add_node_functionality(AAA_AVP_LIST* list, unsigned int data)
{
	char x[4];
	set_4bytes(x, data);
	
	
	return
	Rf_add_avp_list(list,
		x,4,
		AVP_IMS_Node_Functionality,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/* TODO Add User-Session-ID AVP to IMS-Information AVP */

/* TODO Add Calling-Party-Address AVP to IMS-Information AVP */

/* TODO Add Called-Party-Address AVP to IMS-Information AVP */

/* TODO Add Requested-Party-Address AVP to IMS-Information AVP */

/* TODO Add Associated-URI AVP to IMS-Information AVP */

/* TODO Add User-Session-ID AVP to IMS-Information AVP */

/* TODO Add Time-Stamps AVP to IMS-Information AVP */

/* TODO Add Application-Server-Information AVP to IMS-Information AVP */

/* TODO Add Inter-Operator-Identifier AVP to IMS-Information AVP */

/* TODO Add IMS-Charging-Identifier AVP to IMS-Information AVP */

/* TODO Add SDP-Session-Description AVP to IMS-Information AVP */

/* TODO Add SDP-Media-Component AVP to IMS-Information AVP */

/* TODO Add Servd-Party-IP-Address AVP to IMS-Information AVP */

/* TODO Add Server-Capabilities AVP to IMS-Information AVP */

/* TODO Add Trunk-Group-ID AVP to IMS-Information AVP */

/* TODO Add Bearer-Service AVP to IMS-Information AVP */

/* TODO Add Service-Id AVP to IMS-Information AVP */
 
/* TODO Add Service-Specific-Info AVP to IMS-Information AVP */

/* TODO Add Message-Body AVP to IMS-Information AVP */

/* TODO Add Cause-Code AVP to IMS-Information AVP */

/* TODO Add Access-Network-Information AVP to IMS-Information AVP */


/** 
 * Creates and adds Event-Type AVP to an AVP list. 
 *
 * @param outl - points at this Event-Type AVP.
 * @param inl - AVPs list containing member AVPs. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_event_type(AAA_AVP_LIST* outl, AAA_AVP_LIST* inl)
{
	str group;
	
	group = cdpb.AAAGroupAVPS(*inl);
	cdpb.AAAFreeAVPList(inl);
	
	return 
	Rf_add_avp_list(outl,
		group.s, group.len,
		AVP_IMS_Event_Type,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_FREE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Expires AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_expires(AAA_AVP_LIST* list, unsigned int data)
{
	char x[4];
	set_4bytes(x, data);
	
	
	return
	Rf_add_avp_list(list,
		x,4,
		AVP_IMS_Expires,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/** 
 * Creates and adds User-Session-Id AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_user_session_id(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_User_Session_Id,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Calling-Party-Address AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_calling_party_address(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_Calling_Party_Address,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Called-Party-Address AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_called_party_address(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_Called_Party_Address,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}	



/** 
 * Creates and adds Service-Information AVP to a Diameter msg 
 *
 * @param msg - the Diameter message to add to.
 * @param list - point at a list of AVPs 
 * @returns 1 on success or = on error
 */
inline int Rf_add_service_information(AAAMessage* msg, AAA_AVP_LIST* list)
{
	str group;
	
	group = cdpb.AAAGroupAVPS(*list);
	cdpb.AAAFreeAVPList(list);
	
	return
	Rf_add_avp(msg,group.s,group.len,
		AVP_IMS_Service_Information,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_FREE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Associated_URI AVP to a Diameter msg 
 *
 * @param msg - the Diameter message to add to.
 * @param list - point at a list of AVPs 
 * @returns 1 on success or = on error
 */
inline int Rf_add_associated_uri(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_Associated_URI,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds IMS-Charging-Identifier AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_ims_charging_identifier(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_IMS_Charging_identifier,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Originating-IOI AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_originating_ioi(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_Originating_IOI,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Terminating-IOI AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_terminating_ioi(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_Terminating_IOI,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Inter-Operator-Identifier AVP to an AVP list. 
 *
 * @param outl - points at this Event-Type AVP.
 * @param inl - AVPs list containing member AVPs. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_inter_operator_identifier(AAA_AVP_LIST* outl, AAA_AVP_LIST* inl)
{
	str group;
	
	group = cdpb.AAAGroupAVPS(*inl);
	cdpb.AAAFreeAVPList(inl);
	
	return 
	Rf_add_avp_list(outl,
		group.s, group.len,
		AVP_IMS_Inter_Operator_Identifier,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_FREE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Cause-Code AVP to an AVP list. 
 * @param list - the AVP list to add to. 
 * @param data - the value for the AVP payload.
 * @return 1 on success or 0 on error 
 */
inline int Rf_add_cause_code(AAA_AVP_LIST* list, unsigned int data)
{
	char x[4];
	set_4bytes(x, data);
	
	return
	Rf_add_avp_list(list,
		x,4,
		AVP_IMS_Cause_Code,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/** 
 * Creates and adds Access-Network-Information AVP to an AVP list 
 * @param list - the AVP list to add to.
 * @param data - the value for the AVP payload. 
 * @return 1 on successs or 0 on error
 */
inline int Rf_add_access_network_information(AAA_AVP_LIST* list, str data)
{
	return
	Rf_add_avp_list(list,
		data.s,data.len,
		AVP_IMS_Access_Network_Information,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}


/*
 * IP address version 4
 */
inline int Rf_add_served_party_ip_address(AAA_AVP_LIST* list, str data)
{
	char x[6];
	char buf[4];
	char* p;
	int i;
	
	x[0] = 0; 
	x[1] = 1; /* IPv4 */
 	memset(buf,'\0',4);
	for (i=0; i<3; i++) {
		p = strchr(data.s, '.');
		
		strncpy(buf, data.s, p - data.s);
		x[i+2] = atoi(buf);
		data.len = data.len-(p-data.s)-1;
		data.s = p+1;
		memset(buf,'\0',4);
	}
	strncpy(buf, data.s, data.len);
	x[5] = atoi(buf);
	 
	return
	Rf_add_avp_list(list,
		x,6,
		AVP_IMS_Served_Party_IP_Address,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}


inline int Rf_add_event_timestamp(AAAMessage* msg, unsigned int data)
{
	char x[4];
	set_4bytes(x, data);
	
	return
	Rf_add_avp(msg, x,4,
		AVP_Event_Timestamp,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}


inline int Rf_add_sip_request_timestamp(AAA_AVP_LIST* list, unsigned int data)
{
	char x[4];
	set_4bytes(x, data);
	
	return
	Rf_add_avp_list(list,
		x,4,
		AVP_IMS_SIP_Request_Timestamp,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



int Rf_add_sip_response_timestamp(AAA_AVP_LIST* list, unsigned int data)
{
	char x[4];
	set_4bytes(x, data);
	
	return
	Rf_add_avp_list(list,
		x,4,
		AVP_IMS_SIP_Response_Timestamp,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



int Rf_add_time_stamps(AAA_AVP_LIST* outl, AAA_AVP_LIST* inl)
{
	str group;
	
	group = cdpb.AAAGroupAVPS(*inl);
	cdpb.AAAFreeAVPList(inl);
	
	return 
	Rf_add_avp_list(outl,
		group.s, group.len,
		AVP_IMS_Time_Stamps,
		AAA_AVP_FLAG_MANDATORY,
		IMS_vendor_id_3GPP,
		AVP_FREE_DATA,
		__FUNCTION__);

}


