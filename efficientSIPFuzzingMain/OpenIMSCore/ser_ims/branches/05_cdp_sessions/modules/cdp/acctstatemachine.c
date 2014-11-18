/**
 * $Id$
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
 * CDiameterPeer Diameter base protocol state-machine definition 
 * 
 *  \author Joao Filipe Placido joao-f-placido -at- ptinovacao dot pt
 * 
 * Copyright (C) 2007 PT Inovacao
 * 
 */

#include "acctstatemachine.h"
#include "diameter.h"
#include "timer.h"


/** Strings for the accounting client states */
char *acc_cli_states[]={"Idle","PendingS","PendingE","PendingB","Open","PendingI","PendingL"};
/** Strings for the accounting client events */
char *acc_cli_events[]={"AccEv_Start","AccEv_Event","AccEv_BufferedStart","AccEv_Rcv_Suc_ACA_Start","AccEv_SndFail","AccEv_Rcv_Failed_ACA_Start","AccEv_Stop",
	"AccEv_Interim","AccEv_Rcv_Suc_ACA_Interim","AccEv_Rcv_Failed_ACA_Interim","AccEv_Rcv_Suc_ACA_Event","AccEv_Rcv_Failed_ACA_Event",
	"AccEv_Rcv_Suc_ACA_Stop","AccEv_Rcv_Failed_ACA_Stop"};

extern callback_f interim_interval_timer;

/**
 * Diameter client accounting session state-machine processing.
 * This function gets called for every accounting event. It updates the states and can trigger
 * other events.
 * 
 * @param s - the session for which the event happened
 * @param event - the event that happened
 * @param req - if a Diameter message should be sent this is it, or NULL if not
 * @param ans - if a Diameter message was received this is it, or NULL if not
 * @param session_locked - if the session lock is already aquired
 * @returns 1 on success, 0 on error. Also the accounting session states are updated
 */
int acct_cli_sm_process(AAAAcctSession *s,acc_event_t event,AAAMessage *req, AAAMessage* ans, int session_locked)
{
	//acc_event_t next_event;
	//int msg_received=0;
	AAAReturnCode rt = AAA_ERR_FAILURE;
		
	if (!session_locked) s_lock(s->hash);
	LOG(L_DBG,"DBG:acct_cli_sm_process(): AccountingSession %.*s \tState %s \tEvent %s\n",
		s->sID->len,s->sID->s,acc_cli_states[s->state],acc_cli_events[event-101]);

	switch (s->state){
		case ACC_ST_IDLE:
			switch (event){
				case ACC_EV_START:
					s->state = ACC_ST_PENDING_S;
					s->aii = 0;
					
					// send msg asynchronously
					//rt = AAASendMessage(msg, s->peer_fqdn, 0/* timeout_cb TODO*/, 0);
					//if (rt!=AAA_ERR_SUCCESS) acct_cli_sm_process(s, ACC_EV_SNDFAIL, 0, 1);
					
					// send msg synchronously
					// TODO
					ans = AAASendRecvMessage(req, s->peer_fqdn);
					if (!ans) {
						acct_cli_sm_process(s, ACC_EV_SNDFAIL, req, 0, 1);
						break;
					}
					s->state = ACC_ST_OPEN;
					
					unsigned int aii = 0;
					AAA_AVP  *aii_avp = AAAFindMatchingAVP(ans, 0, AVP_Acct_Interim_Interval, 0, AAA_FORWARD_SEARCH);
					if (aii_avp) {
						memcpy(&aii, aii_avp->data.s, 4); // 32 bit arch...
						
						
						
						/* register the acc interim interval timer */
						add_timer(aii,0,interim_interval_timer,s);
					}
					s->aii = aii;
					
					break;	
				case ACC_EV_EVENT:
					s->state = ACC_ST_PENDING_E;
					// send msg asynchronously
					//rt = AAASendMessage(msg, s->peer_fqdn, 0/* timeout_cb TODO*/, 0);
					//if (rt!=AAA_ERR_SUCCESS) acct_cli_sm_process(s, ACC_EV_SNDFAIL, 0, 1);
					
					// send msg synchronously
					ans = AAASendRecvMessage(req, s->peer_fqdn);
					if (!ans) {
						acct_cli_sm_process(s, ACC_EV_SNDFAIL, req, 0, 1);
						break;
					}
					s->state = ACC_ST_IDLE; // TODO
					break;
				case ACC_EV_BUFFEREDSTART:
					s->state = ACC_ST_PENDING_B;
					break;
				default:
					LOG(L_ERR,"ERROR:acct_cli_sm_process(): In state %s invalid event %s\n",
						acc_cli_states[s->state],acc_cli_events[event-101]);
					goto error;
			}
			break;		
		case ACC_ST_PENDING_S:
			switch(event){
				case ACC_EV_RCV_SUC_ACA_START:
					s->state = ACC_ST_OPEN;
					break;	
				case ACC_EV_SNDFAIL:
					// TODO: Store Start Record... if no buffer space available and realtime!=GRANT_AND_LOSE "disconnect user", s->state = Idle;
					s->state = ACC_ST_OPEN;
					break;					
				case ACC_EV_RCV_FAILED_ACA_START:
					// TODO: if no buffer space available and realtime!=GRANT_AND_LOSE "disconnect user", s->state = Idle;
					s->state = ACC_ST_OPEN;
					break;
				case ACC_EV_STOP:
					// TODO: Store Stop Record...
					//s->state = PendingS;
				default:
					LOG(L_ERR,"ERROR:acct_cli_sm_process(): In state %s invalid event %s\n",
						acc_cli_states[s->state],acc_cli_events[event-101]);
					goto error;
			}
			break;
			
		case ACC_ST_PENDING_E:
			switch(event){
				case ACC_EV_RCV_SUC_ACA_EVENT:
					s->state = ACC_ST_IDLE;
					// TODO: AAADropAcctSession(s);
					break;
				case ACC_EV_SNDFAIL:
					// TODO: Store Event Record... 
					s->state = ACC_ST_IDLE;
					// TODO: AAADropAcctSession(s);
					break;
				case ACC_EV_RCV_FAILED_ACA_EVENT:
					s->state = ACC_ST_IDLE;
					// TODO: AAADropAcctSession(s);
					break;
				default:
					LOG(L_ERR,"ERROR:acct_cli_sm_process(): In state %s invalid event %s\n",
						acc_cli_states[s->state],acc_cli_events[event-101]);
					goto error;
			}
			break;	
		case ACC_ST_PENDING_B:
			switch(event){
				case ACC_EV_RCV_SUC_ACA_EVENT:
				case ACC_EV_RCV_SUC_ACA_START:
				case ACC_EV_RCV_SUC_ACA_STOP:
				case ACC_EV_RCV_SUC_ACA_INTERIM:
					// TODO: delete record
					s->state = ACC_ST_IDLE;
					// TODO: AAADropAcctSession(s);
					break;
				case ACC_EV_SNDFAIL:
					s->state = ACC_ST_IDLE;
					break;
				case ACC_EV_RCV_FAILED_ACA_EVENT:
				case ACC_EV_RCV_FAILED_ACA_START:
				case ACC_EV_RCV_FAILED_ACA_STOP:
				case ACC_EV_RCV_FAILED_ACA_INTERIM:
					// TODO: delete record
					s->state = ACC_ST_IDLE;
					// TODO: AAADropAcctSession(s);
					break;
				default:
					LOG(L_ERR,"ERROR:acct_cli_sm_process(): In state %s invalid event %s\n",
						acc_cli_states[s->state],acc_cli_events[event-101]);
					goto error;
			}
			break;
		case ACC_ST_OPEN:
			switch(event){
				case ACC_EV_INTERIM:
					s->state = ACC_ST_PENDING_I;
					// send req asynchronously
					rt = AAASendMessage(req, s->peer_fqdn, 0/*timeout_cb TODO*/, 0);
					if (rt!=AAA_ERR_SUCCESS) acct_cli_sm_process(s, ACC_EV_SNDFAIL, 0, 0, 1);
					break;
				case ACC_EV_STOP:
					s->state = ACC_ST_PENDING_L;
					// send req asynchronously
					rt = AAASendMessage(req, s->peer_fqdn, 0/*timeout_cb TODO*/, 0);
					if (rt!=AAA_ERR_SUCCESS) acct_cli_sm_process(s, ACC_EV_SNDFAIL, 0, 0, 1);
					break;
				default:
					LOG(L_ERR,"ERROR:acct_cli_sm_process(): In state %s invalid event %s\n",
						acc_cli_states[s->state],acc_cli_events[event-101]);
					goto error;
			}
			break;
		case ACC_ST_PENDING_I:
			switch (event){
				case ACC_EV_RCV_SUC_ACA_INTERIM:
					s->state = ACC_ST_OPEN;
					break;
				case ACC_EV_SNDFAIL:
					s->state = ACC_ST_OPEN;
					// TODO: try to store interim record, if no buffer space available and realtime!=GRANT_AND_LOSE "disconnect user", s->state = Idle;
					break;
				case ACC_EV_RCV_FAILED_ACA_INTERIM:
					// TODO: if realtime!=GRANT_AND_LOSE "disconnect user", s->state = Idle;
					s->state = ACC_ST_OPEN;
					break;
				case ACC_EV_STOP:
					// TODO: store Stop record
					//s->state = PendingI;
					break;
				default:
					LOG(L_ERR,"ERROR:acct_cli_sm_process(): In state %s invalid event %s\n",
						acc_cli_states[s->state],acc_cli_events[event-101]);
					goto error;
			}
			break;			
		case ACC_ST_PENDING_L:
			switch (event){
				case ACC_EV_RCV_SUC_ACA_STOP:
					s->state = ACC_ST_IDLE;
					// TODO: AAADropAcctSession(s);
					break;
				case ACC_EV_SNDFAIL:
					// TODO: try to store Stop record
					s->state = ACC_ST_IDLE;
					// TODO: AAADropAcctSession(s);
					break;
				case ACC_EV_RCV_FAILED_ACA_STOP:
					s->state = ACC_ST_IDLE;
					// TODO: AAADropAcctSession(s);
					break;
				default:
					LOG(L_ERR,"ERROR:acct_cli_sm_process(): In state %s invalid event %s\n",
						acc_cli_states[s->state],acc_cli_events[event-101]);
					goto error;
			}
			break;				
	}
	if (!session_locked) s_unlock(s->hash);
	
//	if (msg_received)
//		Rcv_Process(s,msg);
	
	return 1;	
error:
	if (!session_locked) s_unlock(s->hash);
	return 0;	
}




