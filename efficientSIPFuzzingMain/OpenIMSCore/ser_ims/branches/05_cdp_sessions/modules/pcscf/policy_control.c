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

#include "policy_control.h"

#include "../tm/tm_load.h"
#include "mod.h"

#include "gq.h"
#include "rx.h"

extern struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/
extern struct cdp_binds cdpb;          /**< Structure with pointers to cdp funcs 		*/

extern int pcscf_release7;

int P_local_policy(struct sip_msg* msg, char* str1, char* str2) 
{
	LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_local_policy: SDP local policy control\n");
	
	return 1;	
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
	
	LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_AAR: and release %i\n",pcscf_release7);
	if (msg->first_line.type == SIP_REQUEST) {
		LOG(L_ERR, ANSI_WHITE"ERR:"M_NAME": P_AAR: must be called on SIP reply\n");
		return 0;
	}

	/* Get the SIP request from this transaction */
	t=tmb.t_gett();
	if (!t) {
		LOG(L_ERR, ANSI_WHITE"ERR:"M_NAME": P_ARR: cannot get the transaction\n"); 
		return 0;
	}
	//LOG(L_INFO,"INF:"M_NAME"\n%.*s\n\n",t->method.len,t->method.s);
	if (strncmp(t->method.s,"INVITE",6)!=0)
	{
		/*we dont apply QoS if its not a reply to an INVITE!*/
		return 1;
	}
	
	/* Create an AAR based on request and reply and send it to PDF */
	if (!pcscf_release7)
	{
		aaa = Gq_AAR(t->uas.request, msg, atoi(str1));
	} else {
		aaa = Rx_AAR(t->uas.request,msg,atoi(str1));
	}
	//cdpb.AAAPrintMessage(aaa);
	
	if (!aaa) goto error;
	if (!pcscf_release7)
	{
		result = Gq_AAA(aaa);
	} else {
		result = Rx_AAA(aaa);
	}
	//LOG(L_INFO, ANSI_WHITE"INF: rc %d\n", result);
	if (result == AAA_SUCCESS) return 1;

error:
	return -1;
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
	if (!pcscf_release7)
	{
		sta = Gq_STR(msg, atoi(str1));
	} else {
		sta = Rx_STR(msg,atoi(str1));
	}
	/*of course here comes some processing and probably freeing the message*/
	
	return 1;
}