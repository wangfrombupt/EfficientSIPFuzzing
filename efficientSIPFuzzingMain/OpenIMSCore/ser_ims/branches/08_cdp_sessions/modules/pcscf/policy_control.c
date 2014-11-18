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
 * P-CSCF Policy Control Ops
 *
 *
 */

#include "policy_control.h"

#include "../tm/tm_load.h"
#include "mod.h"

#include "pcc.h"
#include "sip.h"

extern struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/
extern struct cdp_binds cdpb;          /**< Structure with pointers to cdp funcs 		*/

extern int pcscf_qos_release7;
extern int pcscf_qos_side;





int P_local_policy(struct sip_msg* msg, char* str1, char* str2) 
{
	LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_local_policy: SDP local policy control\n");
	
	return CSCF_RETURN_TRUE;	
}


/**
 * Checks if this reply is one that is suitable of generating an AAR message...
 * @param msg - SIP response to check
 * @param str1 - Orig or Term , 0 or 1
 * @param str2 - not used
 * 
 * @returns 0 if this reply is not AAR suitable, or 1 if yes
*/


int P_generates_aar(struct sip_msg *msg,char *str1,char *str2)
{
	int tag;
	struct cell *t;
	
	LOG(L_DBG,"P_generates_aar(): starting\n");
	t=tmb.t_gett();
	if (!t) {
		LOG(L_ERR,"P_generates_aar(): unable to get transaction\n");
		return 0;
	}
	if (pcscf_qos_side!=2) {
		tag = cscf_get_mobile_side(msg);
		if (tag==-1) {
			tag=atoi(str1);
		}
		LOG(L_DBG,"P_generates_aar(): decided on side %i\n",tag);
		if (pcscf_qos_side!=tag)
		{
			LOG(L_ERR,"P_generates_aar(): not on the right side\n");
			return CSCF_RETURN_FALSE;
		}
		
	}
	if ((t->method.len==5 && memcmp(t->method.s,"PRACK",5)==0)||(t->method.len==6 && (memcmp(t->method.s,"INVITE",6)==0||memcmp(t->method.s,"UPDATE",6)==0)))
	{
		if (cscf_get_content_len(msg)!=0 && cscf_get_content_len(t->uas.request)!=0)
		{
			// Need to check SDP validity
				return CSCF_RETURN_TRUE;
		} 
	} 
		return CSCF_RETURN_FALSE;
}


/**
 * P_AAR() will be called, if a SIP 183 Session Progress comes back from the callee.  
 * It retrieves relevant SIP headers from SDP offer and answer from INVITE and 183
 * respectively. It converts these SIP headers to corresponding AVPs and creates
 * a AAR and sends it to the PDF and waits for the answer returned by the PDF.
 * 
 * @param msg - The SIP response  
 * @param str1 - not used
 * @param str2 - not used 
 * @returns 1 on Diameter success or 0 on failure   
 */
int P_AAR(struct sip_msg* msg, char* str1, char* str2)
{	
	struct cell *t;
	AAAMessage* aaa;
	int result = AAA_SUCCESS;
	
	LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_AAR: CALLED\n");
	if (msg->first_line.type == SIP_REQUEST) {
		LOG(L_ERR, ANSI_WHITE"ERR:"M_NAME": P_AAR: must be called on SIP reply\n");
		return CSCF_RETURN_FALSE;
	}

	/* Get the SIP request from this transaction */
	t=tmb.t_gett();
	if (!t) {
		LOG(L_ERR, ANSI_WHITE"ERR:"M_NAME": P_AAR: cannot get the transaction\n"); 
		return CSCF_RETURN_FALSE;
	}
	
	if (!(strncmp(t->method.s,"INVITE",6)==0)&&!(strncmp(t->method.s,"UPDATE",6)==0)&&!(strncmp(t->method.s,"PRACK",5)==0))
	{
		
		//we dont apply QoS if its not a reply to an INVITE! or UPDATE or PRACK!
		return CSCF_RETURN_TRUE;
	}

	/* Create an AAR based on request and reply and send it to PDF */
	aaa = PCC_AAR(t->uas.request, msg, str1);
	//cdpb.AAAPrintMessage(aaa);
	
	if (!aaa) goto error;
	result = PCC_AAA(aaa);
	LOG(L_INFO,"recieved an AAA with result code %i\n",result);
	cdpb.AAAFreeMessage(&aaa);
	//LOG(L_INFO, ANSI_WHITE"INF: rc %d\n", result);
	if (result >= 2000 && result < 3000 ) {
		return CSCF_RETURN_TRUE;
	} else {
		 return CSCF_RETURN_FALSE; // if its not a success then that means i want to reject this call!
	} 
	/*
	 * This behaviour is wrong, if its a reINVITE  then the rules already exist, the PCRF may
	 * not install the news but remain with the old ones and in that case it will send a DIAMETER_UNABLE_TO_COMPLY
	 * so we need to catch that posibility here maybe looking at the Error-Message
	*/
	
error:
	return CSCF_RETURN_TRUE; // default policy is if PDF/PCRF not working or errors , then leave everything flow
}



/**
 * P_STR() will be called, if PCSCF receives BYE, CANCEL, 3xx, 4xx, 5xx or 6xx.
 * It terminates the created auth session with the PDF.
 *  
 */
int P_STR(struct sip_msg* msg, char* str1, char* str2)
{
	// get session id from sip msg   
	// Gq_STR(session_id) terminate the session
	AAAMessage* sta;
	LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_STR:\n");
	sta = PCC_STR(msg,str1);
	// if you really want the STA just declare a ResponseHandler for it because its never going
	// to arrive here.. or never again
	
	if (sta) cdpb.AAAFreeMessage(&sta);
	return CSCF_RETURN_TRUE;
}
