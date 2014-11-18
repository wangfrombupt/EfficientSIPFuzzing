/**
 * $Id: authstatemachine.c 513 2008-01-04 12:13:01Z albertoberlios $
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
 * CDiameterPeer Session Handling - Authorization State Machine
 * 
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * \author Shengyao Chen shc -at- fokus dot fraunhofer dot de
 * \author Joao Filipe Placido joao-f-placido -at- ptinovacao dot pt
 * \author Alberto Diez Albaladejo Alberto.Diez at fokus dot fraunhofer dot de
 */

#include <time.h>
 
#include "authstatemachine.h"
#include "diameter_ims.h"

// all this 4 includes are here because of what i do in Send_ASA
#include "peer.h"
#include "peermanager.h"
#include "routing.h"
#include "receiver.h"

char *auth_states[]={"Idle","Pending","Open","Discon"};
char *auth_events[]={};

extern dp_config *config;	// because i want to use tc for the expire times...

int get_result_code(AAAMessage* msg)
{
	AAA_AVP *avp;
	AAA_AVP_LIST list;
	list.head=0;
	list.tail=0;
	int rc=-1;
	
        if (!msg) goto error;
        
        for (avp=msg->avpList.tail;avp;avp=avp->prev)
		{
		
			if (avp->code==AVP_Result_Code)
			{
				rc = get_4bytes(avp->data.s);	
				goto finish;
			} else if (avp->code==AVP_Experimental_Result)
			{
				list=AAAUngroupAVPS(avp->data);
				for(avp=list.head;avp;avp=avp->next)
				{
					if (avp->code==AVP_IMS_Experimental_Result_Code)
						{
							rc = get_4bytes(avp->data.s);
							AAAFreeAVPList(&list);
							goto finish;
						}
			 	}
				AAAFreeAVPList(&list);
			 
					
			}
	
		}
finish:
		return rc;
error:
        LOG(L_ERR, "ERR:get_result_code(): no AAAMessage or Result Code not found\n");
        return -1;
}

/*
 * Alberto Diez changes the default behaviour on error is going to be to return the default state
 * that is  STATE_MAINTAINED
 * this is because in the Rx specification 3GPP TS 29214 v7.1.0 (2007-06)
 * the AVP Auth Session State is not included in any message exchange, 
 * therefor we could as well not even check for it, but diameter rfc says to look at this
 * 
*/

int get_auth_session_state(AAAMessage* msg)
{
        if (!msg) goto error;
        AAA_AVP* rc = AAAFindMatchingAVP(msg, 0, AVP_Auth_Session_State, 0, 0);
        if (!rc) goto error;
        return get_4bytes(rc->data.s);

error:
        LOG(L_ERR, "ERR:get_auth_session_state(): no AAAMessage or Auth Session State not found\n");
        return STATE_MAINTAINED;
}


/**
 * stateful client state machine
 * @param auth - AAAAuthSession which uses this state machine
 * @param ev   - Event
 * @param msg  - AAAMessage
 */
inline void auth_client_statefull_sm_process(cdp_session_t* s, int event, AAAMessage* msg)
{
	cdp_auth_session_t *x;
	int rc;	
	
	
	if (!s) {
		switch (event) {
			case AUTH_EV_RECV_ASR:
				Send_ASA(0,msg);
				break;				
			default:
				LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d with no session!\n",
				event);				
		}
		return;
	}
	x = &(s->u.auth);
	
	// I dont want the session to expire!
	x->timeout+=config->tc*30;
	x->lifetime=x->timeout+config->tc*32;
	
	switch(x->state){
		case AUTH_ST_IDLE:
			switch (event) {
				case AUTH_EV_SEND_REQ:
					s->application_id = msg->applicationId;
					s->u.auth.state = AUTH_ST_PENDING;
					//LOG(L_INFO,"state machine: i was in idle and i am going to pending\n");
					break;				
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_PENDING:
			if (event == AUTH_EV_RECV_ANS && msg && !is_req(msg)){
				rc = get_result_code(msg);
				if (rc>=2000 && rc<3000 && get_auth_session_state(msg)==STATE_MAINTAINED) 
					event = AUTH_EV_RECV_ANS_SUCCESS;
				else
					event = AUTH_EV_RECV_ANS_UNSUCCESS;
			}
			
			switch(event){
				case AUTH_EV_RECV_ANS_SUCCESS:
					x->state = AUTH_ST_OPEN;
					//LOG(L_INFO,"state machine: i was in pending and i am going to open\n");
					break;
				case AUTH_EV_RECV_ANS_UNSUCCESS:
					x->state = AUTH_ST_DISCON;
					//LOG(L_INFO,"state machine: i was in pending and i am going to discon\n");
					Send_STR(s,msg);
					break;										
				default:
					LOG(L_ERR,"ERR:auth_client_stateless_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_OPEN:
			if (event == AUTH_EV_RECV_ANS && msg && !is_req(msg)){
				rc = get_result_code(msg);
				if (rc>=2000 && rc<3000 && get_auth_session_state(msg)==STATE_MAINTAINED) 
					event = AUTH_EV_RECV_ANS_SUCCESS;
				else
					event = AUTH_EV_RECV_ANS_UNSUCCESS;
			}

			switch (event) {
				case AUTH_EV_SEND_REQ:
					// if the request is STR i should move to Discon .. 
					// this is not in the state machine but I (Alberto Diez) need it
					if (msg->commandCode==IMS_STR) 
						s->u.auth.state = AUTH_ST_DISCON;
					else
						s->u.auth.state = AUTH_ST_OPEN;
					break;				
				case AUTH_EV_RECV_ANS_SUCCESS:
					x->state = AUTH_ST_OPEN;
					//LOG(L_INFO,"state machine: i was in open and i am going to open\n");
					break;
				case AUTH_EV_RECV_ANS_UNSUCCESS:
					x->state = AUTH_ST_DISCON;
					//LOG(L_INFO,"state machine: i was in open and i am going to discon\n");
					break;										
				case AUTH_EV_SESSION_TIMEOUT:
				case AUTH_EV_SERVICE_TERMINATED:
				case AUTH_EV_SESSION_GRACE_TIMEOUT:
					x->state = AUTH_ST_DISCON;
					//LOG(L_INFO,"state machine: i was in open and i am going to discon\n");
					Send_STR(s,msg);
					
					break;		
				case AUTH_EV_SEND_ASA_SUCCESS:
					x->state = AUTH_ST_DISCON;
					//LOG(L_INFO,"state machine: i was in open and i am going to discon\n");
					Send_STR(s,msg);
					
					break;
				case AUTH_EV_SEND_ASA_UNSUCCESS:
					x->state = AUTH_ST_OPEN;
					//LOG(L_INFO,"state machine: i was in open and i am going to open\n");
					break;	
				case AUTH_EV_RECV_ASR: 
					// two cases , client will comply or will not
					// our client is very nice and always complys.. because
					// our brain is in the PCRF... if he says to do this , we do it
					// Alberto Diez , (again this is not Diameter RFC)
					x->state = AUTH_ST_DISCON;
					Send_ASA(s,msg);
					Send_STR(s,msg);
					break;			
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_DISCON:
			switch (event) {
				case AUTH_EV_RECV_ASR:
					x->state = AUTH_ST_DISCON;
					//LOG(L_INFO,"state machine: i was in discon and i am going to discon\n");
					Send_ASA(s,msg);
					break;
				// Just added this because it might happen if the other peer doesnt 
				// send a valid STA, then the session stays open forever...
				// We dont accept that... we have lifetime+grace_period for that
				// This is not in the Diameter RFC ...	
				case AUTH_EV_SESSION_GRACE_TIMEOUT:
				// thats the addition
				case AUTH_EV_RECV_STA:
					x->state = AUTH_ST_IDLE;
					//LOG(L_INFO,"state machine: about to clean up\n");
					AAAFreeMessage(&msg);
					// If I register a ResponseHandler then i Free the STA there not here..
					// but i dont have interest in that now..
					
					// I dont know if putting this here or in Session_Cleanup!!! has to be decided
					Session_Cleanup(s,NULL);
					break; 	
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		default:
			LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received event %d while in invalid state %d!\n",
				event,x->state);
	}
}


/**
 * Authorization Server State-Machine - Statefull
 */
inline void auth_server_statefull_sm_process(cdp_session_t* s, int event, AAAMessage* msg)
{
	cdp_auth_session_t *x;
	
	if (!s) return;
	x = &(s->u.auth);
	
	// I dont want the session to expire!
	x->timeout+=config->tc*30;
	x->lifetime=x->timeout+config->tc*2;

	switch(x->state){
		case AUTH_ST_IDLE:
			switch (event) {
				case AUTH_EV_RECV_REQ:
					// The RequestHandler will generate a Send event for the answer
					// and we will only then now if the user is authorised or not
					// if its not authorised it will move back to idle and cleanup the session 
					// so no big deal
					// but this is not the Diameter RFC...
					x->state = AUTH_ST_OPEN;
					break;
				case AUTH_EV_SEND_STA:
					x->state = AUTH_ST_IDLE;
					Session_Cleanup(s,msg);
					break;
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_OPEN:
		
		
			if (event == AUTH_EV_SEND_ANS && msg && !is_req(msg)){
				int rc = get_result_code(msg);
				if (rc>=2000 && rc<3000)
					event = AUTH_EV_SEND_ANS_SUCCESS;
				else
					event = AUTH_EV_SEND_ANS_UNSUCCESS;
			}
		
		
			switch (event) {
				case AUTH_EV_SEND_ANS_SUCCESS:
					x->state = AUTH_ST_OPEN;
					break;
				case AUTH_EV_SEND_ANS_UNSUCCESS:
					x->state = AUTH_ST_IDLE;
					Session_Cleanup(s,msg);					
					break;
				case AUTH_EV_SEND_ASR:
					x->state = AUTH_ST_DISCON;
					break;
				case AUTH_EV_SESSION_TIMEOUT:
				case AUTH_EV_SESSION_GRACE_TIMEOUT:
					x->state=AUTH_ST_IDLE;
					Session_Cleanup(s,msg);
				case AUTH_EV_SEND_STA:
					x->state = AUTH_ST_IDLE;
					Session_Cleanup(s,msg);
					break;
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_DISCON:
			switch (event) {
				case AUTH_EV_RECV_ASA_SUCCESS:
					x->state=AUTH_ST_IDLE;
					Session_Cleanup(s,msg);
				case AUTH_EV_RECV_ASA_UNSUCCESS:
					Send_ASR(s,msg);
					// how many times will this be done?
					x->state=AUTH_ST_DISCON;
				case AUTH_EV_SEND_STA:
					x->state = AUTH_ST_IDLE;
					Session_Cleanup(s,msg);
					break;
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;

		default:
			LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received event %d while in invalid state %d!\n",
				event,x->state);
	}	
}




/** 
 * Authorization Client State-Machine - Stateless
 */ 
inline void auth_client_stateless_sm_process(cdp_session_t* s, int event, AAAMessage *msg)
{
	cdp_auth_session_t *x;
	int rc;
	if (!s) return;
	x = &(s->u.auth);
	switch(x->state){
		case AUTH_ST_IDLE:
			switch(event){
				case AUTH_EV_SEND_REQ:
					x->state = AUTH_ST_PENDING;
					break;
				default:
					LOG(L_ERR,"ERR:auth_client_stateless_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		case AUTH_ST_PENDING:
			if (!is_req(msg)){
				rc = get_result_code(msg);
				if (rc>=2000 && rc<3000 && get_auth_session_state(msg)==NO_STATE_MAINTAINED) 
					event = AUTH_EV_RECV_ANS_SUCCESS;
				else
					event = AUTH_EV_RECV_ANS_UNSUCCESS;
			}
			switch(event){
				case AUTH_EV_RECV_ANS_SUCCESS:
					x->state = AUTH_ST_OPEN;
					break;
				case AUTH_EV_RECV_ANS_UNSUCCESS:
					x->state = AUTH_ST_IDLE;
					break;					
				default:
					LOG(L_ERR,"ERR:auth_client_stateless_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		case AUTH_ST_OPEN:
			switch(event){
				case AUTH_EV_SESSION_TIMEOUT:
					x->state = AUTH_ST_IDLE;
					break;
				case AUTH_EV_SERVICE_TERMINATED:
					x->state = AUTH_ST_IDLE;
					break;					
				default:
					LOG(L_ERR,"ERR:auth_client_stateless_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		default:
			LOG(L_ERR,"ERR:auth_client_stateless_sm_process(): Received event %d while in invalid state %d!\n",
				event,x->state);
	}	
}

/**
 * Authorization Server State-Machine - Stateless
 */
inline void auth_server_stateless_sm_process(cdp_session_t* auth, int event, AAAMessage* msg)
{
	/* empty - no state change, anyway */
/*
	cdp_auth_session_t *x;
	int rc;
	if (!s) return;
	x = &(s->u.auth);
	switch(x->state){
		case AUTH_ST_IDLE:
			switch(event){				
				default:
					LOG(L_ERR,"ERR:auth_server_stateless_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_state[x->state]);				
			}
			break;
		default:
			LOG(L_ERR,"ERR:auth_server_stateless_sm_process(): Received event %d while in invalid state %d!\n",
				event,x->state);
	}
*/
}

void Send_ASA(cdp_session_t* s, AAAMessage* msg)
{
	
	
	AAAMessage *asa;
	char x[4];
	AAA_AVP *avp;	
	LOG(L_INFO,"Send_ASA():  sending ASA\n");
	if (!s) {
	//send an ASA for UNKNOWN_SESSION_ID - use AAASendMessage()
	// msg is the ASR recieved
		asa = AAANewMessage(IMS_ASA,0,0,msg);
		if (!asa) return;	

	
		set_4bytes(x,AAA_SUCCESS);
		AAACreateAndAddAVPToMessage(asa,AVP_Result_Code,AAA_AVP_FLAG_MANDATORY,0,x,4);
	
		AAASendMessage(asa,0,0);

	}else{
	// send... many cases... maybe not needed.
	// for now we do the same
	asa = AAANewMessage(IMS_ASA,0,0,msg);
	if (!asa) return;	

	
	
	
	set_4bytes(x,AAA_SUCCESS);
	AAACreateAndAddAVPToMessage(asa,AVP_Result_Code,AAA_AVP_FLAG_MANDATORY,0,x,4);
	
	
	avp = AAAFindMatchingAVP(msg,0,AVP_Origin_Host,0,0);
	
		
	if (avp) 
	{
			// This is because AAASendMessage is not going to find a route to the 
			// the PCRF because TS 29.214 says no Destination-Host and no Auth-Application-Id
			// in the ASA
		LOG(L_INFO,"sending ASA to peer %.*s\n",avp->data.len,avp->data.s); 
		
			peer *p;
			p = get_peer_by_fqdn(&avp->data);
			if (!peer_send_msg(p,asa))
			{
				AAAFreeMessage(&asa);	
			} else { 
				LOG(L_INFO,"success sending ASA\n");
			}
	}else if (!AAASendMessage(asa,0,0)) {
		LOG(L_ERR,"Send_ASA() : error sending ASA\n");
	}
	
	}	
}



void Send_STR(cdp_session_t* s, AAAMessage* msg)
{
	AAAMessage *str=0;
	AAA_AVP *avp=0;
	char x[4];
	LOG(L_DBG,"Send_STR() : sending STR\n");
	str = AAACreateRequest(s->application_id,IMS_STR,Flag_Proxyable,s);
	
	if (!str) {
		LOG(L_ERR,"ERR:Send_STR(): error creating STR!\n");
		return;
	}
	
	
	set_4bytes(x,s->application_id);
	avp = AAACreateAVP(AVP_Auth_Application_Id,AAA_AVP_FLAG_MANDATORY,0,x,4,AVP_DUPLICATE_DATA);
	AAAAddAVPToMessage(str,avp,str->avpList.tail);
	
	set_4bytes(x,4); // Diameter_administrative
	avp = AAACreateAVP(AVP_Termination_Cause,AAA_AVP_FLAG_MANDATORY,0,x,4,AVP_DUPLICATE_DATA);
	AAAAddAVPToMessage(str,avp,str->avpList.tail);
	//todo - add all the other avps
	peer *p;
			p = get_routing_peer(str);
			if (!p) {
				LOG(L_ERR,"unable to get routing peer in Send_STR \n");
				AAAFreeMessage(&str);
				return;
			}
			
			if (!peer_send_msg(p,str))
			{
				AAAFreeMessage(&str);	
			} else { 
				LOG(L_DBG,"success sending STR\n");
			}
	 
}

void Send_ASR(cdp_session_t* s, AAAMessage* msg)
{
	AAAMessage *asr=0;
	AAA_AVP *avp=0;
	char x[4];
	LOG(L_DBG,"Send_ASR() : sending ASR\n");
	asr = AAACreateRequest(s->application_id,IMS_ASR,Flag_Proxyable,s);
	
	if (!asr) {
		LOG(L_ERR,"ERR:Send_ASR(): error creating ASR!\n");
		return;
	}
	
	
	set_4bytes(x,s->application_id);
	avp = AAACreateAVP(AVP_Auth_Application_Id,AAA_AVP_FLAG_MANDATORY,0,x,4,AVP_DUPLICATE_DATA);
	AAAAddAVPToMessage(asr,avp,asr->avpList.tail);
	
	set_4bytes(x,3); // Not specified
	avp = AAACreateAVP(AVP_IMS_Abort_Cause,AAA_AVP_FLAG_MANDATORY,0,x,4,AVP_DUPLICATE_DATA);
	AAAAddAVPToMessage(asr,avp,asr->avpList.tail);
	//todo - add all the other avps
	peer *p;
			p = get_routing_peer(asr);
			if (!p) {
				LOG(L_ERR,"unable to get routing peer in Send_ASR \n");
				AAAFreeMessage(&asr);
			}
			
			if (!peer_send_msg(p,asr))
			{
				AAAFreeMessage(&asr);	
			} else { 
				LOG(L_DBG,"success sending ASR\n");
			}
	 
}

void Session_Cleanup(cdp_session_t* s, AAAMessage* msg)
{
	// Here we should drop the session ! and free everything related to it
	// but the generic_data thing should be freed by the callback function registered
	// when the auth session was created
	AAASessionCallback_f *cb;
	LOG(L_INFO,"cleaning up session %.*s\n",s->id.len,s->id.s);
	if (s->cb) {
		cb = s->cb;
		(cb) (AUTH_EV_SERVICE_TERMINATED,s->cb_param,s);
	}
	
	AAADropAuthSession(s);
}
