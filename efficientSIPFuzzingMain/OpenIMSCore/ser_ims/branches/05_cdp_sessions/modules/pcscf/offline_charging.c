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
 * 
 */
#include "offline_charging.h"

#include "../../dset.h"
#include "../tm/tm_load.h"
#include "mod.h"
#include "rf.h"
#include "rf_avp.h"

extern struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/
extern struct cdp_binds cdpb;          /**< Structure with pointers to cdp funcs 		*/


/*
 * P_ACR_event makes an ACR/ACA exchange with the CDF based on 
 * a pair of SIP request and response. 
 * 
 * stateless Diameter transaction
 */
int P_ACR_event(struct sip_msg* msg, char* str1, char* str2)
{
	struct cell *t;
	AAAMessage* aca;
	int result = AAA_SUCCESS;
	
	struct sip_msg* req;
	
	LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_ACR_event\n");
	
	if (msg->first_line.type == SIP_REQUEST) {
		LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_ACR: called on SIP request\n");
		aca = Rf_ACR_event(msg, NULL);
	} else {
		/* Get the SIP request from this transaction */
		t=tmb.t_gett();
		LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_ACR: called on SIP reply\n");
		if (!t) {
			LOG(L_ERR, ANSI_WHITE"ERR:"M_NAME": P_ACR: cannot get the transaction\n"); 
			goto error;
		}
		
		/* Create an ACR based on request (the forwarded one with additional 
		 * headers like P-Charging-Vector, etc.) and reply 
		 */
		req=pkg_malloc(sizeof(struct sip_msg));
		memset(req,0, sizeof(struct sip_msg)); /* init everything to 0 */
		req->buf = t->uac[t->on_branch].request.buffer;
		req->len = t->uac[t->on_branch].request.buffer_len;
		if (parse_msg(req->buf,req->len, req)==-1) goto error;
		aca = Rf_ACR_event(req, msg); /* create ACR and sent it to CDF */
	}
	
	//cdpb.AAAPrintMessage(aca);
	
	if (!aca) goto error;
	//result = Rf_ACA(aca);
	
	//LOG(L_INFO, ANSI_WHITE"INF: rc %d\n", result);
	//if (result == AAA_SUCCESS) return 1;
	
	pkg_free(req);
	return 0;

error:
	pkg_free(req);
	return -1;
}



/*
 * P_ACR_start makes an ACR/ACA exchange with the CDF based on 
 * a pair of SIP request and response. 
 * 
 * It initializes a new stateful Diameter session, which will be terminated 
 * by P_ACR_stop.
 */
int P_ACR_start(struct sip_msg* msg, char* str1, char* str2) {
	
	struct cell *t;
	AAAMessage* aca;
	int result = AAA_SUCCESS;
	
	struct sip_msg* req;
	
	LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_ACR_start\n");
	
	if (msg->first_line.type == SIP_REQUEST) {
		LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_ACR_start: called on SIP request\n");
		aca = Rf_ACR_event(msg, NULL);
	} else {
		/* Get the SIP request from this transaction */
		t=tmb.t_gett();
		LOG(L_INFO, ANSI_WHITE"INF:"M_NAME":P_ACR_start: called on SIP reply\n");
		if (!t) {
			LOG(L_ERR, ANSI_WHITE"INF:"M_NAME":P_ACR_start: cannot get the transaction\n"); 
			goto error;
		}
		
		/* Create an ACR based on request (the forwarded one with additional 
		 * headers like P-Charging-Vector, etc.) and reply 
		 */
		req=pkg_malloc(sizeof(struct sip_msg));
		memset(req,0, sizeof(struct sip_msg)); /* init everything to 0 */
		req->buf = t->uac[t->on_branch].request.buffer;
		req->len = t->uac[t->on_branch].request.buffer_len;
		if (parse_msg(req->buf,req->len, req)==-1) goto error;
		aca = Rf_ACR_start(req, msg); /* create ACR and sent it to CDF */
	}
	
	//cdpb.AAAPrintMessage(aca);
	
	if (!aca) goto error;
	//result = Rf_ACA(aca);
	
	//LOG(L_INFO, ANSI_WHITE"INF: rc %d\n", result);
	//if (result == AAA_SUCCESS) return 1;
	
	pkg_free(req);
	return 0;

error:
	pkg_free(req);
	return -1;
	

	return 0;
}

int P_ACR_interim(struct sip_msg* msg, char* str1, char* str2) {
	LOG(L_INFO, "trigger ACR[interim] exchange with CDF");

	return 0;
}

int P_ACR_stop(struct sip_msg* msg, char* str1, char* str2) {
	LOG(L_INFO, "trigger ACR[stop] exchange with CDF");

	return 0;
}

int P_ACA_event(struct sip_msg* msg, AAAMessage* aca) {
	return 0;
}
int P_ACA_start(struct sip_msg* msg, AAAMessage* aca) {
	return 0;
}
int P_ACA_interim(struct sip_msg* msg, AAAMessage* aca) {
	return 0;
}
int P_ACA_stop(struct sip_msg* msg, AAAMessage* aca) {
	return 0;
}
