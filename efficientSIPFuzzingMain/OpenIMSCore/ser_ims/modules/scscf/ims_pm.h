/*
 * $Id: ims_pm.h 320 2007-06-15 14:07:58Z vingarzan $
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
 * \file  ims_pm.h
 * 
 *	X-CSCF IMS Performance Management
 * 
 * Scope: logs raw data for computing metrics as in TS 32.409
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

/*#define WITH_IMS_PM*/

#ifdef WITH_IMS_PM

#ifndef IMS_PM_H_
#define IMS_PM_H_

#include "../../sr_module.h"

#define IMS_PM_DEBUG 1

enum _ims_pm_event_types {
	OP_NOP							= 0,

	OP_NodeStart					= 1,
	OP_NodeStop						= 2,
	
	UR_AttInitReg					= 3,
	UR_SuccInitReg					= 4,
	UR_FailInitReg					= 5,
	UR_MeanInitRegSetupTime		 	= 6,

	UR_AttReReg						= 7,
	UR_SuccReReg					= 8,
	UR_FailReReg					= 9,
	UR_MeanReRegSetupTime 			= 10,

	UR_AttDeRegUe					= 11,
	UR_SuccDeRegUe					= 12,
	UR_FailDeRegUe					= 13,
	UR_MeanDeRegUeSetupTime			= 14,

	UR_AttDeRegHss					= 15,
	UR_SuccDeRegHss					= 16,
	UR_FailDeRegHss					= 17,
	UR_MeanDeRegHssSetupTime		= 18,
	
	UR_AttDeRegCscf					= 19,
	UR_SuccDeRegCscf				= 20,
	UR_FailDeRegCscf				= 21,
	UR_MeanDeRegCscfSetupTime 		= 22,
	
	UR_Att3rdPartyReg				= 23,
	UR_Succ3rdPartyReg				= 24,
	UR_Fail3rdPartyReg				= 25,
	UR_Mean3rdPartyRegSetupTime		= 26,
	
	UR_AttUAR						= 27,
	UR_SuccUAA						= 28,
	UR_FailUAA						= 29,
	UR_MeanUATime					= 30,
	
	UR_AttSAR						= 31,
	UR_SuccSAA						= 32,
	UR_FailSAA						= 33,
	UR_MeanSATime					= 34,

	SC_AttSession					= 35,
	SC_SuccSession					= 36,
	SC_AnsSession					= 37,
	SC_FailSession					= 38,
	SC_NbrSimulAnsSessionMax		= 39,
	
	LIQ_AttLIR						= 40,		
	LIQ_SuccLIA						= 41,		
	LIQ_FailLIA						= 42,
	LIQ_MeanLITime					= 43,		
	
	IC_AttSessionFromOtherNtwkDmn 	= 44,
	IC_403SessionFromOtherNtwkDmn 	= 45,
	IC_AttSessionToOtherNtwkDmn 	= 46,
	IC_403SessionToOtherNtwkDmn 	= 47,
	
	RU_AttInitRegOfVisitUsers		= 48,
	RU_Nbr403InitRegOfVisitUsers	= 49,
	RU_RmgUsersOut					= 50,
	
	MA_AttMAR						= 51,
	MA_SuccMAA						= 52,
	MA_FailMAA						= 53,
	MA_MeanMATime					= 54,
	
	UP_AttPPR						= 55,
	UP_SuccPPA						= 56,
	UP_FailPPA						= 57,
	UP_MeanPPTime					= 58,
	 
	OTHER_Att						= 59,
	OTHER_Succ						= 60,
	OTHER_Fail						= 61,
	OTHER_MeanTime					= 62,
		
	RD_NbrIMPU						= 63,
	RD_NbrContact					= 64,
	RD_NbrSubs						= 65,
	RD_NbrIPSecSA					= 66,
	RD_NbrTLSSA						= 67,
	RD_NbrNATPinHoles				= 68,			
	RD_NbrDialogs					= 69,
	RD_NbrAV						= 70,	

	DBU_NbrImsSubscription			= 71,
	DBU_NbrPriUserId				= 72,
	DBU_NbrSipPubUserId				= 73,
	DBU_NbrTelPubUserId				= 74,
	DBU_NbrPriSrvId					= 75,
	DBU_NbrSipPubSrvId				= 76,
	DBU_NbrTelPubSrvId				= 77,
	
	DBU_NbrRegPubUserId				= 78,
	DBU_NbrUnregPubUserId			= 79,
	DBU_NbrRegPriUsrId				= 80,
		
	UR_AttRTR						= 81,
	UR_SuccRTA						= 82,
	UR_FailRTA						= 83,
	UR_MeanRTTime					= 84,
			
	DTR_AttUDR						= 85,
	DTR_SuccUDA						= 86,
	DTR_FailUDA						= 87,
	DTR_FailUDA_NoReply				= 88,
	DTR_MeanUDTime					= 89,
	
	DTR_AttPUR						= 90,
	DTR_SuccPUA						= 91,
	DTR_FailPUA						= 92,
	DTR_MeanPUTime					= 93,
	
	SUB_AttSNR						= 94,
	SUB_SuccSNA						= 95,
	SUB_FailSNA						= 96,
	SUB_MeanSNTime					= 97,
	
	NOTIF_AttPNR					= 98,
	NOTIF_SuccPNA					= 99,
	NOTIF_FailPNA					=100,
	NOTIF_MeanSNTime				=101,
				
};


#define IMS_PM_LOG(event)  					do {ims_pm_log(event,zero,zero,0,0);	}while(0)  
#define IMS_PM_LOG01(event,pi1)				do {ims_pm_log(event,zero,zero,pi1,0);	}while(0)
#define IMS_PM_LOG10(event,ps1)				do {ims_pm_log(event,ps1,zero,0,0);		}while(0)
#define IMS_PM_LOG11(event,ps1,pi1)			do {ims_pm_log(event,ps1,zero,pi1,0);	}while(0)
#define IMS_PM_LOG12(event,ps1,pi1,pi2)		do {ims_pm_log(event,ps1,zero,pi1,pi2);	}while(0)
#define IMS_PM_LOG21(event,ps1,ps2,pi1)  	do {ims_pm_log(event,ps1,ps2,pi1,0);	}while(0)
#define IMS_PM_LOG22(event,ps1,ps2,pi1,pi2) do {ims_pm_log(event,ps1,ps2,pi1,pi2);	}while(0)


void ims_pm_init(str node_name,char* type, char *file_name);
void ims_pm_destroy();

void ims_pm_log(enum _ims_pm_event_types event,str ps1,str ps2,int pi1,int pi2);


#endif /*IMS_PM_H_*/
#else

#define IMS_PM_LOG(event)  					  
#define IMS_PM_LOG01(event,pi1)				
#define IMS_PM_LOG10(event,ps1)				
#define IMS_PM_LOG11(event,ps1,pi1)			
#define IMS_PM_LOG12(event,ps1,pi1,pi2)		
#define IMS_PM_LOG21(event,ps1,ps2,pi1)  	
#define IMS_PM_LOG22(event,ps1,ps2,pi1,pi2) 


#endif /* WITH_IMS_PM */
