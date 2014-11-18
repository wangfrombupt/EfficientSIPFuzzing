/*
 * $Id: ims_pm.c 435 2007-08-20 11:51:14Z vingarzan $
 *  
 * Copyright (C) 2004-200FhG Fokus
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
 * \file  ims_pm.c
 * 
 *	X-CSCF IMS Performance Management
 * 
 * Scope: logs raw data for computing metrics as in TS 32.409
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#include "ims_pm.h"


#ifdef WITH_IMS_PM

#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "mod.h"
#include "../../script_cb.h"
#include "../tm/tm_load.h"

#include "sip.h"

str log_prefix;

str log_header={"#\n#\n# Time,Node Type, Node Name, Event, String parameter 1, String Parameter 2, Integer parameter 1, Integer parameter 2\n",0};

int fd_log=0;
FILE *f_log=0;

str ims_pm_event_types[] = {
	{"OP.NOP",6},
	{"OP.NodeStart",12},
	{"OP.NodeStop",11},
	
	{"UR.AttInitReg",13},
	{"UR.SuccInitReg",14},
	{"UR.FailInitReg",14},
	{"UR.MeanInitRegSetupTime",23},

	{"UR.AttReReg",11},
	{"UR.SuccReReg",12},
	{"UR.FailReReg",12},
	{"UR.MeanReRegSetupTime",21},

	{"UR.AttDeRegUe",13},
	{"UR.SuccDeRegUe",14},
	{"UR.FailDeRegUe",14},
	{"UR.MeanDeRegUeSetupTime",23},

	{"UR.AttDeRegHss",14},
	{"UR.SuccDeRegHss",15},
	{"UR.FailDeRegHss",15},
	{"UR.MeanDeRegHssSetupTime",24},

	{"UR.AttDeRegCscf",15},
	{"UR.SuccDeRegCscf",16},
	{"UR.FailDeRegCscf",16},
	{"UR.MeanDeRegCscfSetupTime",25},

	{"UR.Att3rdPartyReg",17},
	{"UR.Succ3rdPartyReg",18},
	{"UR.Fail3rdPartyReg",18},
	{"UR.Mean3rdPartyRegSetupTime",27},

	{"UR.AttUAR",9},
	{"UR.SuccUAA",10},
	{"UR.FailUAA",10},
	{"UR.MeanUATime",13},

	{"UR.AttSAR",9},
	{"UR.SuccSAA",10},
	{"UR.FailSAA",10},
	{"UR.MeanSATime",13},

	{"SC.AttSession",13},
	{"SC.SuccSession",14},
	{"SC.AnsSession",13},
	{"SC.FailSession",14},
	{"SC.NbrSimulAnsSessionMax",25},
	
	{"LIQ.AttLIR",10},
	{"LIQ.SuccLIA",11},
	{"LIQ.FailLIA",11},
	{"LIQ.MeanLITime",14},
		
	{"IC.AttSessionFromOtherNtwkDmn",29},
	{"IC.403SessionFromOtherNtwkDmn",29},
	{"IC.AttSessionToOtherNtwkDmn",27},
	{"IC.403SessionToOtherNtwkDmn",27},
	
	{"RU.AttInitRegOfVisitUsers",25},	
	{"RU.Nbr3InitRegOfVisitUsers",26},
	{"RU.RmgUsersOut",14},				
	
	{"MA.AttMAR",9},					
	{"MA.SuccMAA",10},					
	{"MA.FailMAA",10},					
	{"MA.MeanMATime",13},
	
	{"UP.AttPPR",9},					
	{"UP.SuccPPA",10},				
	{"UP.FailPPA",10},					
	{"UP.MeanPPTime",13},				
	 
	{"OTHER.Att",9},					
	{"OTHER.Succ",10},					
	{"OTHER.Fail",10},					
	{"OTHER.MeanTime",14},				
		
	{"RD.NbrIMPU",10},			
	{"RD.NbrContact",13},
	{"RD.NbrSubs",10},
	{"RD.NbrIPSecSA",13},
	{"RD.NbrTLSSA",11},
	{"RD.NbrNATPinHoles",17},
	{"RD.NbrDialogs",13},
	{"RD.NbrAV",8},
	
	{"DBU.NbrImsSubscription",22},
	{"DBU.NbrPriUserId",16},
	{"DBU.NbrSipPubUserId",19},
	{"DBU.NbrTelPubUserId",19},
	{"DBU.NbrPriSrvId",15},
	{"DBU.NbrSipPubSrvId",18},
	{"DBU.NbrTelPubSrvId",18},
	
	{"DBU.NbrRegPubUserId",19},
	{"DBU.NbrUnregPubUserId",21},
	{"DBU.NbrRegPriUsrId",18},
		
	{"DTR.AttUDR",10},
	{"DTR.SuccUDA",11},
	{"DTR.FailUDA",11},
	{"DTR.FailUDA.NoReply",19},
	{"DTR.MeanUDTime",14},
	
	{"DTR.AttPUR",10},
	{"DTR.SuccPUA",11},
	{"DTR.FailPUA",11},
	{"DTR.MeanPUTime",14},
	
	{"SUB.AttSNR",10},
	{"SUB.SuccSNA",11},
	{"SUB.FailSNA",11},
	{"SUB.MeanSNTime",14},
	
	{"NOTIF.AttPNR",12},
	{"NOTIF.SuccPNA",13},
	{"NOTIF.FailPNA",13},
	{"NOTIF.MeanSNTime",16},
			
			
	{0,0}
};


static str zero={0,0};

void ims_pm_init(str node_name,char* type, char *file_name)
{
	log_prefix.len = 1+strlen(type)+1+node_name.len+1;
	log_prefix.s = pkg_malloc(log_prefix.len);
	if (!log_prefix.s) {
		LOG(L_ERR,"ERR:"M_NAME":ims_pm_init(): Error alocating %d bytes\n",log_prefix.len);
		log_prefix.len=0;
		return;
	}
	log_prefix.len=0;
	
	log_prefix.s[log_prefix.len++]=',';
	
	memcpy(log_prefix.s+log_prefix.len,type,strlen(type));			
	log_prefix.len+=strlen(type);
	
	log_prefix.s[log_prefix.len++]=',';
	
	memcpy(log_prefix.s+log_prefix.len,node_name.s,node_name.len);
	log_prefix.len+=node_name.len;
	
	log_prefix.s[log_prefix.len++]=',';
	
	fd_log = open(file_name,O_WRONLY|O_APPEND|O_CREAT|O_NONBLOCK);
	if (fd_log<0){
		LOG(L_ERR,"ERR:"M_NAME":ims_pm_init(): Error opening fd logger file %s : %s\n",file_name,strerror(errno));
		fd_log = 0;
		return;		
	}
	f_log = fdopen(fd_log,"a");	
	if (!f_log){
		LOG(L_ERR,"ERR:"M_NAME":ims_pm_init(): Error opening logger file %s : %s\n",file_name,strerror(errno));
		close(fd_log);
		fd_log=0;
		return;
	}
	
	log_header.len = strlen(log_header.s);
	
	if (write(fd_log,log_header.s,log_header.len)<0){
		LOG(L_ERR,"ERR:"M_NAME":ims_pm_init(): Error writing to IMS PM log file: %s\n",strerror(errno));
	}
	IMS_PM_LOG(OP_NodeStart);	
	
}

void ims_pm_destroy()
{
	IMS_PM_LOG(OP_NodeStop);	
	if (fd_log) close(fd_log);
}

void ims_pm_log(enum _ims_pm_event_types event,str ps1,str ps2,int pi1,int pi2)
{
	fprintf(f_log,"%u%.*s%.*s,%.*s,%.*s,%d,%d\n",
		(unsigned int)time(0),
		log_prefix.len,log_prefix.s,
		ims_pm_event_types[event].len,ims_pm_event_types[event].s,
		ps1.len,ps1.s,
		ps2.len,ps2.s,
		pi1,
		pi2);
/*	LOG(L_CRIT,"%u%.*s%.*s,%.*s,%.*s,%d,%d\n",
		(unsigned int)time(0),
		log_prefix.len,log_prefix.s,
		ims_pm_event_types[event].len,ims_pm_event_types[event].s,
		ps1.len,ps1.s,
		ps2.len,ps2.s,
		pi1,
		pi2);*/
	#if IMS_PM_DEBUG
		fflush(f_log);
	#endif
}



#endif /* WITH_IMS_PM */
