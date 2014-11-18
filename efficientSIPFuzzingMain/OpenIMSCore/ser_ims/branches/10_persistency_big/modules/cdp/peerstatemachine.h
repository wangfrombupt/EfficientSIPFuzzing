/**
 * $Id: peerstatemachine.h 2 2006-11-14 22:37:20Z vingarzan $
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
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#ifndef __STATEMACHINE_H
#define __STATEMACHINE_H


#include "utils.h"

#include "peer.h"
#include "diameter.h"

int sm_process(peer *p,peer_event_t event,AAAMessage *msg,int peer_locked,int sock);

peer_state_t I_Snd_Conn_Req(peer *p);
void Cleanup(peer *p,int sock);
void Error(peer *p, int sock);
void I_Snd_CER(peer *p);
int Process_CEA(peer *p,AAAMessage *cea);
void I_Disc(peer *p);
void R_Disc(peer *p);
int Process_DWR(peer *p,AAAMessage *dwr);
void Process_DWA(peer *p,AAAMessage *dwa);
void Snd_DWR(peer *p);
void Snd_DWA(peer *p,AAAMessage *dwr,int result_code,int sock);
void Snd_DPR(peer *p);
void Snd_DPA(peer *p,AAAMessage *dpr,int result_code,int sock);
void R_Accept(peer *p,int sock);
void R_Reject(peer *p,int sock);
int Process_CER(peer *p,AAAMessage *cer);
void Snd_CEA(peer *p,AAAMessage *cer,int result_code,int sock);
int Elect(peer *p,AAAMessage *cer);

void Snd_Message(peer *p, AAAMessage *msg);
void Rcv_Process(peer *p, AAAMessage *msg);

#endif
