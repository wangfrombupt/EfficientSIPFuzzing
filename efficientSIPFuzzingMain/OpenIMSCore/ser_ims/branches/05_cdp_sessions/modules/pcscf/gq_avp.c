/**
 * $Id: gq_avp.c,v 1.16 2007/03/14 16:18:13 chens Exp $
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
#include "gq_avp.h"
#include "../../mem/shm_mem.h"


/**< Structure with pointers to cdp funcs, global variable defined in mod.c  */
extern struct cdp_binds cdpb;

/*
 * For some reason the next to functions aren't
 * exported in the cdp module.. should be done there!
*/
AAA_AVP*  AAAGetFirstAVP(AAA_AVP_LIST *avpList){
	return avpList->head;
}


/**
 * Add an AVP to a list of AVPs.
 * @param list - the list to add to
 * @param avp - the avp to add
 */
void AAAAddAVPToAVPList(AAA_AVP_LIST *list,AAA_AVP *avp)
{
	

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
}







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
static inline int Gq_add_avp(AAAMessage *m,char *d,int len,int avp_code,
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
static inline int Gq_add_avp_list(AAA_AVP_LIST *list,char *d,int len,int avp_code,
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
static inline str Gq_get_avp(AAAMessage *msg,int avp_code,int vendor_id,
							const char *func)
{
	AAA_AVP *avp;
	str r={0,0};
	
	avp = cdpb.AAAFindMatchingAVP(msg,0,AVP_Result_Code,0,0);
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
inline int Gq_add_destination_realm(AAAMessage *msg, str data)
{
	return 
	Gq_add_avp(msg,data.s,data.len,
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
inline int Gq_add_auth_application_id(AAAMessage *msg, unsigned int data)
{
	char x[4];
	set_4bytes(x, data);	

	return
	Gq_add_avp(msg, x, 4,
			AVP_Auth_Application_Id,
			AAA_AVP_FLAG_MANDATORY,
			0,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
}
 
/**
 * Creates and adds a Media Component Description AVP
 * @param msg - the Diameter message to add to.
 * @param sdpinvite - the SDP body of the INVITE message
 * @param sdp200 - the SDP body of the 200 OK message
 * @param mline - pointer to m= line in sdpinvite
 * @number - the media component number relating to this SDP description
 * @tag - 0 is originating side and 1 is terminating side
 * @return 1 on success or 0 on error
 */
 
 inline int Gq_add_media_component_description(AAAMessage *msg,str sdpinvite,str sdp200,char *mline,int number,int tag)
 {
 	str data;
 	AAA_AVP_LIST list;
 	AAA_AVP *media_component_number,*media_type;
 	AAA_AVP *codec_data1,*codec_data2;
 	AAA_AVP *media_sub_component[Gq_Media_Sub_Components];
 	AAA_AVP *Max_DL,*Max_UL;
 	AAA_AVP *RR,*RS;
 	AAA_AVP *flow_status;
 	
 	bandwidth bwUL,bwDL;
 	
 	
 	char *ptr;
 	char port[Gq_MAX_Char];
 	int type,i,n,a;
 	char x[4];
	
 	
 	/*needed to use AAA_AVP-LIST!!*/
 	list.head=0;	 	
 	list.tail=0;
 	
 	
 	
 	/*media-component-number*/
 	 	
 	set_4bytes(x,number);	
 	media_component_number=cdpb.AAACreateAVP(AVP_IMS_Media_Component_Number,
 											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
 											IMS_vendor_id_3GPP,x,4,
 											AVP_DUPLICATE_DATA);

 	if (media_component_number!=NULL) 
 	{
 		AAAAddAVPToAVPList(&list,media_component_number);
 	} else {
 		LOG(L_INFO, ANSI_RED"INF:"M_NAME"Unable to create media_component_number AVP");
 		return 0;
 	}
 	
 	/*media-sub-component*/
 	
	n=Gq_create_add_media_subcomponents(&list,sdpinvite,sdp200,number,media_sub_component,tag);
 	if(n==-1)
 	{
 		LOG(L_INFO, ANSI_RED"INF:"M_NAME"Unable to create media_sub_components list AVP");
 		cdpb.AAAFreeAVP(&media_component_number);
 		list.head=0;
 		list.tail=0;	
 		return 0;
 	}
 	if (n==0) {
 		/*we don't create a Media Description for this media line*/
 		/*because answerer rejected the offer!*/
 		cdpb.AAAFreeAVP(&media_component_number);
 		list.head=0;
 		list.tail=0;
 		return 1;
 	}
 	
 	
 	
 	/*if n=-1 then its error*/
 	/*if n=0 is because answerer rejected this media offer*/
 	/*or offerer wanted it to be disabled*/
 	 	
 	/*media-type*/
 	 ptr=mline;
 	 ptr+=2; /*skip m=*/
 	 	 
 	 
 	 
 	 if (strncmp(ptr,"audio",5)==0)
 	 {
 	 	type=AVP_IMS_Media_Type_Audio;
 	 } else if(strncmp(ptr,"video",5)==0)
 	 {
 	 	type=AVP_IMS_Media_Type_Video;
 	 }else if(strncmp(ptr,"data",4)==0)
 	 {
 	 	type=AVP_IMS_Media_Type_Data;
 	 }else if(strncmp(ptr,"application",11)==0)
 	 {
 	 	type=AVP_IMS_Media_Type_Application;
 	 }else if(strncmp(ptr,"control",7)==0)
 	 {
 	 	type=AVP_IMS_Media_Type_Control;
 	 }else if(strncmp(ptr,"text",4)==0)
 	 {
 	 	type=AVP_IMS_Media_Type_Text;
 	 }else if(strncmp(ptr,"message",7)==0)
 	 {
 	 	type=AVP_IMS_Media_Type_Message;
 	 } else 
 	 {
 	 	type=AVP_IMS_Media_Type_Other;
 	 }
 	 
 	  	 
 	 set_4bytes(x,type);
 	 media_type=cdpb.AAACreateAVP(AVP_IMS_Media_Type,
 	 								AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
 	 								IMS_vendor_id_3GPP,x,4,
 	 								AVP_DUPLICATE_DATA);
 	 AAAAddAVPToAVPList(&list,media_type);								

 		 								
 	
 	/*Max-Requested-Bandwidth-UL*/
 	/*Max-Requested-Bandwidth-DL*/
 	/*SDP bodies have been check by gq_create_add_media_subcomponents*/
 	
 	i=1;
 	ptr=find_sdp_line(sdp200.s,(sdp200.s+sdp200.len),'m');
 	
 	while(i!=number) 
 	{
 		ptr=find_next_sdp_line(ptr,(sdp200.s+sdp200.len),'m',NULL);
 		i++;	
 	}
 	
 	
 	
 	if(tag==1)
 	{
 		/*in the invite its defined how much bandwidth
 		 * you want in the downlink!*/
 		extract_bandwidth(&bwDL,sdpinvite,mline);
 		extract_bandwidth(&bwUL,sdp200,ptr);
 	} else {
 		extract_bandwidth(&bwDL,sdp200,ptr);
 		extract_bandwidth(&bwUL,sdpinvite,mline);	
 		 		
 	}
 	/*
 	 * IN a SDP b=AS:x line  the x is the amount of bandwidth
 	 * the sender of the SDP body is willing to RECEIVE 
 	 * therefor the next code is right 
 	 * 
 	 * */
	if (bwDL.bAS!=0)
	 {
	 	
 			set_4bytes(x,bwDL.bAS);
 			Max_UL=cdpb.AAACreateAVP(AVP_IMS_Max_Requested_Bandwidth_UL,
											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
											IMS_vendor_id_3GPP,x,4,
											AVP_DUPLICATE_DATA);
			AAAAddAVPToAVPList(&list,Max_UL);
		
	 }
	if (bwUL.bAS!=0)
	{
 			set_4bytes(x,bwUL.bAS);
 			Max_DL=cdpb.AAACreateAVP(AVP_IMS_Max_Requested_Bandwidth_DL,
											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
											IMS_vendor_id_3GPP,x,4,
											AVP_DUPLICATE_DATA);
			AAAAddAVPToAVPList(&list,Max_DL);
			
	}
	
	
	
 	/*Flow-Status*/
 	/*lets follow the specs*/
 		if (tag==0)
 		{
 			extract_token(mline,port,Gq_MAX_Char,2);
 		} else {
 			extract_token(ptr,port,Gq_MAX_Char,2);
 		}
 		if(strncmp(port,"0",1)==0)
 		{
 			set_4bytes(x,AVP_IMS_Flow_Status_Removed);
  		} else {
  			
  			if (tag==1)
  			{
  				a=check_atributes(sdp200,ptr);
  			} else {
  				a=check_atributes(sdpinvite,mline);
  			}
  			
  			if (a==1)
  			{
  				set_4bytes(x,AVP_IMS_Flow_Status_Enabled_Uplink);
  			} else if(a==2)
  			{
  				set_4bytes(x,AVP_IMS_Flow_Status_Enabled_Downlink);
  			} else {
  				set_4bytes(x,AVP_IMS_Flow_Status_Enabled);
  			} 
  			
  			
 		}
 	
 	
 		
 		flow_status=cdpb.AAACreateAVP(AVP_IMS_Flow_Status,
 											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
 											IMS_vendor_id_3GPP,x,4,
 											AVP_DUPLICATE_DATA);
		AAAAddAVPToAVPList(&list,flow_status);
 	
 	
 	/*RR and RS*/
 	
 	
 	x[0]=0; x[1]=0; x[2]=0; x[3]=0;
 	
		
			if(bwUL.bRR!=0) {
				set_4bytes(x,bwUL.bRR);
			} /*else if (bwDL.bRS!=0) {
				set_4bytes(x,bwDL.bRS);
			}*/
		
	RR=0;
	if (x[0]!=0 || x[1]!=0 || x[2]!=0 || x[3]!=0) {
			RR=cdpb.AAACreateAVP(AVP_IMS_RR_Bandwidth,
											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
											IMS_vendor_id_3GPP,x,4,
											AVP_DUPLICATE_DATA);
			AAAAddAVPToAVPList(&list,RR);
			 	 		
	}	
	

 	x[0]=0; x[1]=0; x[2]=0; x[3]=0;
 	
			if(bwUL.bRS!=0) 
			{
				set_4bytes(x,bwUL.bRS);
			} /*else if(bwDL.bRR!=0){
				set_4bytes(x,bwDL.bRR);
			}*/
	
			
 	RS=0;
	if (x[0]!=0 || x[1]!=0 || x[2]!=0 || x[3]!=0)
	 {
		RS=cdpb.AAACreateAVP(AVP_IMS_RS_Bandwidth,
								AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
								IMS_vendor_id_3GPP,x,4,
								AVP_DUPLICATE_DATA);
		AAAAddAVPToAVPList(&list,RS);
		
	}		
 	
 	
 	
 	
 	
 	
 	/*codec-data*/
 	 	
 	
 	if (tag==0)
 	{
 		/*0 means uplink offer*/
 		codec_data1=Gq_create_codec_data(sdpinvite,number,0);
 		AAAAddAVPToAVPList(&list,codec_data1);
 		/*3 means downlink answer*/
 		codec_data2=Gq_create_codec_data(sdp200,number,3);
 		AAAAddAVPToAVPList(&list,codec_data2);
 	} else { 
 		/*2 means downlink offer*/
 		codec_data1=Gq_create_codec_data(sdpinvite,number,2);
 		AAAAddAVPToAVPList(&list,codec_data1);
 		/*1 means uplink answer*/
 		codec_data2=Gq_create_codec_data(sdp200,number,1);
 		AAAAddAVPToAVPList(&list,codec_data2);
 	
 	}
 	/*now group them in one big AVP and free them*/
 	
 	data=cdpb.AAAGroupAVPS(list);
  	 		
  	Gq_add_avp(msg,data.s,data.len,AVP_IMS_Media_Component_Description,
 				AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
 				IMS_vendor_id_3GPP,
 				AVP_DUPLICATE_DATA,
 				__FUNCTION__);
 	
 			
 	cdpb.AAAFreeAVP(&media_component_number);
 	for(i=0;i<n;i++)
 	{
 		cdpb.AAAFreeAVP(&media_sub_component[i]);
 	}
 	
 	if (bwUL.bAS!=0)
	 {	
	 	cdpb.AAAFreeAVP(&Max_UL);	 	
	 }
	if (bwDL.bAS!=0)
	{
		cdpb.AAAFreeAVP(&Max_DL);		
	}
	
	
	cdpb.AAAFreeAVP(&flow_status);
	if (RS!=0)
 	{
 		cdpb.AAAFreeAVP(&RS);
 	}
 	if (RR!=0)
 	{
 		cdpb.AAAFreeAVP(&RR);
 	}
 	
 	cdpb.AAAFreeAVP(&media_type);
 	cdpb.AAAFreeAVP(&codec_data1);
 	cdpb.AAAFreeAVP(&codec_data2);
 	
 	list.tail=0;
 	list.head=0;
 	return 1;
 }
/* Creates and adds Media-Sub-Components to AVP_LIST
 * @param list - pointer to the AVP_LIST
 * @param sdpA - the SDP body of the INVITE
 * @param sdpB - the SDP body of the 200 OK answer for the INVITE
 * @param number - the number of the media component to use (which m= line?)
 * @param media_sub_component -  array of Gq_Media_Sub_Components elements of type *AAA_AVP that were added
 * @param tag - 0 originating side 1 terminating side 
 * returns the number of media_sub_components added on success -1 on error
 * the media_sub_component is given in order to free the AVPS after grouping them!
 */
inline int Gq_create_add_media_subcomponents(AAA_AVP_LIST *list,str sdpA,str sdpB,int number,AAA_AVP **media_sub_component,int tag)
 {

 	
 
	
	char *mlineA,*mlineB,*clineA,*clineB;
 	char portA[Gq_MAX_Char],portB[Gq_MAX_Char];
 	int intportA,intportB;
 	char addressA[Gq_MAX_Char];
 	char addressB[Gq_MAX_Char];
 	int i=0,flows=0;
 	int atributes=0; /* a= lines present?*/
 	char *newline,*rtp;
	int ports=1; /*how many ports does this m line define?*/
		
 		
 		if (!extract_mclines(sdpA,sdpB,&mlineA,&clineA,&mlineB,&clineB,number))
 		{
 			return -1;
 		}
 		   
 		   /*a= lines are also needed*/
 		 
 		 
 		 if (tag==0) {
 		 	atributes=check_atributes(sdpA,mlineA);
 			
 			
 		 } else {
 		 	atributes=check_atributes(sdpB,mlineB);
 		 	
 		 }
 		 
 		  /*1 means sendonly*/
 		  /*2 means recvonly*/
 		  /*0 is no a= line or sendrecv or recvonly*/
 		 	
 		
 		 
 		if (!extract_token(mlineA,portA,Gq_MAX_Char,2))
 		{
 				return -1; /*problem extracting port*/
 		}
 		if (!extract_token(mlineB,portB,Gq_MAX_Char,2))
 		{
 				return -1; /* either no ' ' or Gq_MAX_Char too small*/
 		}
 		
 		/*check if this are ports or multiports!*/
 		
 		if(!is_a_port(portA) || !is_a_port(portB)) 
 		{
 			return -1; /* there was a word there but it isn't a port*/
 		}
 		 		 		
 		if (strncmp(portA,"0",1)!=0 && strncmp(portB,"0",1)==0)
 		{
 			/*this means answerer rejected the offer*/
 			
 			return 0; /*this is more logical*/
 		}
 		
 		/*the next lines have nothing to do with rtp
 		 * i just reused the pointer*/	
 		rtp=index(portA,'/');
 		if (rtp!=NULL)
 		{
 			sscanf(portA,"%*i/%i%*s",&ports);
 			*rtp='\0'; 
 				
 		}	
 		
 		 		
 		
 		if(!extract_token(clineA,addressA,Gq_MAX_Char,3))
 		{
 			return -1;
 		}
 		if(!extract_token(clineB,addressB,Gq_MAX_Char,3))
 		{
 			return -1;
 		}
 		
 		
				
				
 		/* i is the flow number */
 		/*flows is the number of data flows .. for each port 1 data flow*/
 			
 			while(flows<ports && i+2<Gq_Media_Sub_Components)
 			{
 				
 				i++;
 				if (tag!=1)
 				{
 					media_sub_component[i-1]=Gq_create_media_subcomponent(i,"ip",addressA,portA,addressB,portB,"",atributes);
 					AAAAddAVPToAVPList(list,media_sub_component[i-1]);		
 				} else {
 		
 					media_sub_component[i-1]=Gq_create_media_subcomponent(i,"ip",addressB,portB,addressA,portA,"",atributes);
 					AAAAddAVPToAVPList(list,media_sub_component[i-1]);		
 				}
 				flows++;
 			
 				if (1) 
 				{
		 			rtp=strstr(mlineA,"RTP");
		 			newline=index(mlineA,'\n');
		 	
		 			if (rtp!=NULL && rtp < newline)
		 	 		{
		 			i++;
		 			/*please optimize here!*/
		 				sscanf(portA,"%i",&intportA);
		 				sscanf(portB,"%i",&intportB);
		 				intportA++; 
		 				intportB++;
		 				sprintf(portA,"%i",intportA);
		 				sprintf(portB,"%i",intportB);
		 				if (tag!=1)
		 				{
		 					media_sub_component[i-1]=Gq_create_media_subcomponent(i,"ip",addressA,portA,addressB,portB,"",3);
		 					AAAAddAVPToAVPList(list,media_sub_component[i-1]);	
		 				} else {
		 					media_sub_component[i-1]=Gq_create_media_subcomponent(i,"ip",addressB,portB,addressA,portA,"",3);
		 					AAAAddAVPToAVPList(list,media_sub_component[i-1]);	
		 				}		 		
		 			}
	 				sscanf(portA,"%i",&intportA);
	 				sscanf(portB,"%i",&intportB);
	 				intportA++;  
		 			intportB++;
		 			sprintf(portA,"%i",intportA);
		 			sprintf(portB,"%i",intportB);
		 			
		 			/*if its not an RTP flow and it has multiports then the odd ports
		 			 * are used for the next component .. if it is RTP and multiports than 
		 			 * the even ports are for the next component because the odd are used for 
		 			 * RTCP flows*/
 				}						
 											
 			}							
 	  	
 	return (i);
 }
/*Creates a media-sub-component AVP
 * @param number - the flow number
 * @param proto - the protocol of the IPFilterRule
 * @param ipA - ip of the INVITE  (if creating rule for UE that sent INVITE)
 * @param portA - port of the INVITE (if creating rule for UE that sent INVITE)
 * @param ipB - ip of 200 OK (if creating rule for UE that sent INVITE)
 * @param portB - port of 200 OK (if creating rule for UE that sent INVITE)
 * @param options - any options to append to the IPFilterRules
 * @param atributes - indication of atributes 
 * 						0 no atributes , 1 sendonly , 2 recvonly , 3 RTCP flows
 * @param bwUL - bandwidth uplink
 * @param bwDL - bandiwdth downlink
 */ 

 AAA_AVP *Gq_create_media_subcomponent(int number,char *proto, char *ipA,char *portA, char *ipB,char *portB ,char *options,int atributes)
 {
 
 		str data;
 		int len,len2;
 		char whatchar[Gq_MAX_Char4]; /*too big!*/
 		char whatchar2[Gq_MAX_Char4];
 		AAA_AVP *flow_description1,*flow_description2,*flow_number;
 		AAA_AVP *flow_usage;
 		
 		AAA_AVP_LIST list;
 		list.tail=0;
 		list.head=0;
 		char x[4];
 		
 		
 		set_4bytes(x, number);
 				
 		flow_number=cdpb.AAACreateAVP(AVP_IMS_Flow_Number,
 											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
 											IMS_vendor_id_3GPP,x,4,
 											AVP_DUPLICATE_DATA);
		AAAAddAVPToAVPList(&list,flow_number);
		/*first flow is the recieve flow*/
		
		if (atributes==0 || atributes==2 || atributes==3) 
		{
			
			len=sprintf(whatchar,"permit out %s from %s %s to %s %s %s",proto,ipB,"",ipA,portA,options); 											
 			flow_description1=cdpb.AAACreateAVP(AVP_IMS_Flow_Description,
 											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
 											IMS_vendor_id_3GPP,whatchar,len,
 											AVP_DUPLICATE_DATA);
 			AAAAddAVPToAVPList(&list,flow_description1);
 			
		} 
		if (atributes==0 || atributes==1 || atributes==3)
		{
 		/*second flow is the send flow*/									
 			len2=sprintf(whatchar2,"permit in %s from %s %s to %s %s %s",proto,ipA,"",ipB,portB,options);
 			flow_description2=cdpb.AAACreateAVP(AVP_IMS_Flow_Description,
 											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
 											IMS_vendor_id_3GPP,whatchar2,len2,
 											AVP_DUPLICATE_DATA);
			AAAAddAVPToAVPList(&list,flow_description2);
		}
		
 		
 		
 		
 		if (atributes==3)
		{
			set_4bytes(x,AVP_IMS_Flow_Usage_Rtcp);
			flow_usage=cdpb.AAACreateAVP(AVP_IMS_Flow_Usage,
											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
											IMS_vendor_id_3GPP,x,4,
											AVP_DUPLICATE_DATA);
			AAAAddAVPToAVPList(&list,flow_usage);
		} 											
 		
 		
 		
 		
 
 		 	
 		 /*group all AVPS into one big.. and then free the small ones*/	
 		 		
 		data=cdpb.AAAGroupAVPS(list);
 		
 		
 		
 		cdpb.AAAFreeAVP(&flow_number);
 		
 		
 		if (atributes==0 || atributes==2 || atributes==3)
 		{
			cdpb.AAAFreeAVP(&flow_description1);
 		}
 		
 		if (atributes==0 || atributes==1 || atributes==3)
 		{
 			cdpb.AAAFreeAVP(&flow_description2);
 		}
 		
 		
 		
 		
 		if(atributes==3)
 		{
 			cdpb.AAAFreeAVP(&flow_usage);
 		}
 		return (cdpb.AAACreateAVP(AVP_IMS_Media_Sub_Component,
 											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
 											IMS_vendor_id_3GPP,data.s,data.len,
 											AVP_DUPLICATE_DATA));
}

/*
 * Creates a Codec-Data AVP as defined in TS29214 (Rx interface)
 * @param sdp - sdp body of message
 * @param number - the number of the m= line being used
 * @param direction - 0 means uplink offer 1 means uplink answer ,
 * 	2 means downlink offer , 3 downlink answer
 * returns NULL on failure or the pointer to the AAA_AVP on success
 * (this AVP should be freed!)
*/

AAA_AVP* Gq_create_codec_data(str sdp,int number,int direction)
{
		char data[Gq_MAX_Char4];
		char *p,*q,*r;
		int i=1,l;
		
		switch(direction) {
			
			case 0: sprintf(data,"uplink\noffer\n");	
					break;
			case 1: sprintf(data,"uplink\nanswer\n");
					break;
			case 2: sprintf(data,"downlink\noffer\n");
					break;
			case 3: sprintf(data,"downlink\nanswer\n");
					break;
			default: 
					break;
						
		}
		l=strlen(data);
		
		p=find_sdp_line(sdp.s,(sdp.s+sdp.len),'m');
		
		while (p!= NULL && i<number)
		{
			p=find_next_sdp_line(p,(sdp.s+sdp.len),'m',NULL);
			i++;
		} 
		if (p==NULL)
		{
			return NULL;
		}
		q=find_next_sdp_line(p,(sdp.s+sdp.len),'m',(sdp.s+sdp.len));
		
		
		r=index(p,'\n');
		memcpy(data+l,p,r-p+1);
		l+=r-p+1;
		/*the m= line is always copied*/
		
		
		
		
		p=r+1; /* p is always the start of the line*/
		r=index(r+1,'\n'); /*r always the \n char of that line*/
		
		while(p+2<q && (l+r-p+1)<Gq_MAX_Char4)
			{
				/*what about spaces? think it closely*/
				if ((strstr(p,"a=sendonly")!=NULL && strstr(p,"a=sendonly")<r) || 
						(strstr(p,"a=recvonly")!=NULL && strstr(p,"a=recvonly")<r) ||
						(strstr(p,"a=sendrecv")!=NULL && strstr(p,"a=sendrecv")<r) ||
						strncmp(p,"b=RS",4)==0 || strncmp(p,"b=RR",4)==0 || strncmp(p,"b=AS",4)==0)
				{
							p=r+1; 		/*skip this line*/
							r=index(r+1,'\n');
				
						
				} else {
						memcpy(data+l,p,r-p+1);
						l+=r-p+1;
						p=r+1;
						r=index(r+1,'\n');
				}
			}
		data[l-2]='\0';
			
			
		l=strlen(data);
		return (cdpb.AAACreateAVP(AVP_IMS_Codec_Data,
 											AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
 											IMS_vendor_id_3GPP,data,l,
 											AVP_DUPLICATE_DATA));
 											
}




/*Extracts the m= and c= lines for the two SDP bodys of the SDP exchange
 * @param sdpA - body of the INVITE
 * @param sdpB - body of the 200 OK
 * @param mlineA - pointer to beginning of m= in sdpA 
 * @param clineA - pointer to beginning of matching c= in sdpA
 * @param mlineB - pointer to beginning of m= in sdpB
 * @param clineB - pointer to beginning of matching c= in sdpB
 * @param number - the ordinal number of the m= to use in case of more than one
 * returns 1 on success 0 on error
 */
 
int extract_mclines(str sdpA,str sdpB,char **mlineA,char **clineA,char **mlineB,char **clineB,int number)
{
 	
 	char *nclineA,*nclineB; /*next*/
 	char *sclineA=NULL,*sclineB=NULL; /*session*/
 	char *nmlineA,*nmlineB; /*next*/
 	int i;
 	
 	 		
 	
 	
 	*clineA=find_sdp_line(sdpA.s,(sdpA.s+sdpA.len),'c');
 	*clineB=find_sdp_line(sdpB.s,(sdpB.s+sdpB.len),'c');
 	*mlineA=find_sdp_line(sdpA.s,(sdpA.s+sdpA.len),'m');
 	*mlineB=find_sdp_line(sdpB.s,(sdpB.s+sdpB.len),'m');
	
	
 	
	if (*clineA==NULL || *clineB==NULL || *mlineA==NULL || *mlineB==NULL)
 	{
 		/*missing at least one cline and mline in each SDPbody*/
 		LOG(L_ERR, ANSI_RED"ERR:"M_NAME" Malformed SDP body\n");
 		return 0;
 	} 	
 	
 	nclineA=find_next_sdp_line(*clineA,(sdpA.s+sdpA.len),'c',NULL);
 	nclineB=find_next_sdp_line(*clineB,(sdpB.s+sdpB.len),'c',NULL);
 	nmlineA=find_next_sdp_line(*mlineA,(sdpA.s+sdpA.len),'m',NULL);
 	nmlineB=find_next_sdp_line(*mlineB,(sdpB.s+sdpB.len),'m',NULL);
 	
 	
 	
 	if (*clineA < *mlineA) 
 	{
 		sclineA=*clineA;
 	}
 	if (*clineB < *mlineB)
 	{
 		sclineB=*clineB;
 	}
 	
 	
 	
 	if (number > 1)
 	 {
 		for (i=1;i<number;i++)
 		{
 			*mlineA=nmlineA;
 			*mlineB=nmlineB;
 			nmlineA=find_next_sdp_line(*mlineA,(sdpA.s+sdpA.len),'m',NULL);
 			nmlineB=find_next_sdp_line(*mlineB,(sdpB.s+sdpB.len),'m',NULL);
 			
 			if(nclineA >*mlineA && (nclineA < nmlineA || nmlineA== NULL))
 			{
 				// if there is a c line between two m lines or after the last m line
 				// then this c line belongs to the first one 
 			 		*clineA=nclineA;
 			 		nclineA=find_next_sdp_line(*clineA,(sdpA.s+sdpA.len),'c',NULL);
 			} else {
 				// if not then the session description one is the one 
 				*clineA=sclineA;
 			}
 			
 			if(nclineB >*mlineB && (nclineB < nmlineB || nmlineB== NULL))
 			{
 				// if there is a c line between two m lines or after the last m line
 				// then this c line belongs to the first one 
 			 		*clineB=nclineB;
 			 		nclineB=find_next_sdp_line(*clineB,(sdpB.s+sdpB.len),'c',NULL);
 			} else {
 				// if not then the session description one is the one 
 				*clineB=sclineB;
 			}
 		
 		
 		
 		if (*mlineA == NULL || *mlineB == NULL || *clineA == NULL || *clineB == NULL)
 		{
 			LOG(L_ERR,"ERR:"M_NAME":%s: Failed getting m= and c= lines in SDP\n","extract_mclines");
 			return 0;
 		}
 					
 		}
 		// after this we should have mlineA , mlineB , clineA, clineB
 		// with the right values	
 		
 	 }
 	 
 	
 	 return 1;
}

/*
 * Extract the token
 * @param line - pointer to line in SDP body
 * @param token - pointer to buffer to copy the token to
 * @param max - size of the buffer
 * @number - which token .. separated by ' ' or ' ' and then '\n'
 * returns 1 on success, 0 on error
 */
 
  
int extract_token(char *line,char *token,int max,int number)
{
	char *p,*q,*r;
	int i;
	
	p=line;
	for(i=1; i<number;i++)
	{
		p=index(p,' ');
		if (p==NULL)
			return 0;
		while (*p==' ')
		{
			p++;
		}
		
	}
	q=index(p,' ');
	r=index(p,'\n');
	q=q < r? q : r;
	while isspace(*(q-1)) q--;
	if (q-p<max) 
	{
		memcpy(token,p,q-p);
		token[q-p]='\0';
		return 1;
	} else {
		return 0;
	}
	
}

/*
 * Extract the data from b=  lines and add it to the structure
 * @param bw - the pointer to the bandwidth structure to fill
 * @param sdp - the sdp body where to look for b= lines 
 * @param start - the pointer to the starting point to look at
 
 * returns 1 on success 0 if no b= line was found
 */
int extract_bandwidth(bandwidth *bw,str sdp,char *start)
{
	char *b;
	char *m; /*next mline*/
	
	bw->bAS=0; bw->bRS=0; bw->bRR=0;	
	
	b=find_next_sdp_line(start,(sdp.s+sdp.len),'b',NULL);
	while (b!=NULL) {
				
		m=find_next_sdp_line(b,(sdp.s+sdp.len),'m',NULL);
		if (m!=NULL && b>m)
		{
		/*this bandwidth belongs to some other media!*/
			return 0;
		}
		b+=2; /*skip b and =*/
		while (*b==' ')	
		{
			b++;		
		}
		if (*b=='A' && *(b+1)=='S')
		{
			sscanf(b,"AS:%i%*s",&bw->bAS);
					
		} else if( *b=='R') {
			if (*(b+1)=='S') 
			{
				sscanf(b,"RS:%i%*s",&bw->bRS);
				
			} else if (*(b+1)=='R')	{
				sscanf(b,"RR:%i%*s",&bw->bRR);
				
			}
		} 
		/*find next b line*/
		b=find_next_sdp_line(b,(sdp.s+sdp.len),'b',NULL);
			
	}
		
		return 1;
}


/* Check for sendonly or recvonly modifiers in a= lines
 * @param sdpbody - SDP body
 * @param mline - pointer to beginning of m= line
 * returns  0 if no modifier was found,
 *  -1 on error (malformed SDP body)
 * 1 if sendonly , 2 if recvonly
 * */
int check_atributes(str sdpbody,char *mline) 
{
	char *p,*q;
	int s=0;
	
	p=find_sdp_line(sdpbody.s,sdpbody.s+sdpbody.len,'a');
	q=find_sdp_line(sdpbody.s,sdpbody.s+sdpbody.len,'m');
	
	
	if (p==NULL) 
	{
		return 0;
	}
	
	/*see if there is a sessionwide  a= line*/
	if (p<q)
	{
		p+=2; /*skip a=*/
		while(*p==' ')
		{
			p++;
		}
		if(strncmp(p,"sendonly",8)==0)
		{
			s=1;
		} else if(strncmp(p,"recvonly",8)==0)
		{
			s=2;
		}
		
		
	}
	/*see if there is a mediawide a= line*/
	
	p=find_sdp_line(mline,(sdpbody.s+sdpbody.len),'a');
	if (p==NULL)
	{
		return s;
	}
	mline++;
	q=find_next_sdp_line(mline,(sdpbody.s+sdpbody.len),'m',NULL);
	
	if (q!=NULL && p>q) 
	{		
			return s; /*the a= line found belongs to some other m= line*/	
	}
	
	p+=2; /*skip a=*/
	while(*p==' ')
	{
		p++;
	}
	if(strncmp(p,"sendonly",8)==0)
	{
		return 1;
	} else if(strncmp(p,"recvonly",8)==0)
	{
		return 2;
	} else {
		return s;
	}
			
}

/*
 * Check if this token extracted is a port or multiport 
 * @param port - the string to check
 * returns 0 if this is not a port  1 if it is a port or multiport
*/
int is_a_port(char *port) {
	int i,multiport=0;
	for (i=0;i<strlen(port);i++)
	{
		switch(port[i]) {
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '0':
				continue;
			
			case '/':
				if (multiport==0 && i>0)
				{
					multiport=1;
					continue;
			
				} else {
					return 0;
			
				}
			default :
				return 0;
			
		}	
	}
	return 1;
}
/*
 * Check if its an address
 * @param ad  - the pointer to the array of characters representing the address
 * returns 1 if it is an address 0 if not
 * for now any amount of numbers with . or : between them is ok 
*/
/*
int is_an_address(char *ad)
{
	int i,last=0,ip4=0;
	for (i=0;i<strlen(ad);i++)
	{
		switch(ad[i]) {
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '0':
				last=0;
				continue;
			case '.':
				if (!last)
					{
						ip4=1;
						last=1;
						continue;
					} else {
						return 0;
					}
			case ':':
					if (ip4)
					{
						return 0;
					}
					continue;
			default :
					return 0;
		}
	}
	return 1;
}*/		



/**
 * Returns the Result-Code AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline int Gq_get_result_code(AAAMessage *msg, int *data)
{
	str s;
	
	s = Gq_get_avp(msg,
		AVP_Result_Code,
		0,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);

	return 1;
}

