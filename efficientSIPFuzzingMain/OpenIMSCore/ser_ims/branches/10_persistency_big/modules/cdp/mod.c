/**
 * $Id: mod.c 355 2007-06-28 15:41:19Z vingarzan $
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
 * CDiameterPeer SER module interface and definitions.
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#include "mod.h"

#include "diameter_peer.h"
#include "config.h"
#include "cdp_load.h"

MODULE_VERSION


char* config_file="DiameterPeer.xml"; 	/**< default DiameterPeer configuration filename */

extern dp_config *config; 				/**< DiameterPeer configuration structure */

/**
 * Exported functions. This is the API available for use from other SER modules.
 * If you require more, please add them here.
 * <p>
 * - load_cdp() - find and load the CDiameterPeer function bindings
 * <p>
 * - AAACreateRequest() - create a diameter request #AAAMessage
 * - AAACreateResponse() - create a diameter response #AAAMessage
 * - AAAFreeMessage() - free up the memory used in a Diameter message
 * <p>
 * - AAASendMessage() - asynchronously send a message
 * - AAASendMessageToPeer() - asynchronously send a message to a forced peer
 * - AAASendRecvMessage() - synchronously send a message and get the response
 * - AAASendRecvMessageToPeer() - synchronously send a message and get the response to a forced peer 
 * <p>
 * - AAACreateSession() - create a diameter #AAASessionId
 * - AAADropSession() - drop a diameter #AAASessionId
 * <p>
 * - AAACreateTransaction() - create a diameter #AAATransaction
 * - AAADropTransaction() - drop a diameter #AAATransaction
 * <p>
 * - AAACreateAVP() - create an #AAA_AVP
 * - AAAAddAVPToMessage() - add an #AAA_AVP to a #AAAMessage
 * - AAAFindMatchingAVP() - find an #AAA_AVP inside a #AAAMessage
 * - AAAGetNextAVP() - get the next #AAA_AVP from the #AAAMessage
 * - AAAFreeAVP() - free the memory taken by the #AAA_AVP
 * - AAAGroupAVPS() - group a #AAA_AVP_LIST of #AAA_AVP into a grouped #AAA_AVP 
 * - AAAUngroupAVPS() - ungroup a grouped #AAA_AVP into a #AAA_AVP_LIST of #AAA_AVP
 * - AAAFindMatchingAVPList() - find an #AAA_AVP inside a #AAA_AVP_LIST
 * - AAAFreeAVPList() - free the memory taken by the all members of #AAA_AVP_LIST
 * <p>
 * - AAAAddRequestHandler() - add a #AAARequestHandler_f callback to request being received
 * - AAAAddResponseHandler() - add a #AAAResponseHandler_f callback to responses being received
 */
static cmd_export_t cdp_cmds[] = {
	{"load_cdp",					(cmd_function)load_cdp, 				NO_SCRIPT, 0, 0},
	
	{"AAACreateRequest",			(cmd_function)AAACreateRequest,			NO_SCRIPT, 0, 0},
	{"AAACreateResponse",			(cmd_function)AAACreateResponse, 		NO_SCRIPT, 0, 0},
	{"AAAFreeMessage",				(cmd_function)AAAFreeMessage,			NO_SCRIPT, 0, 0},

	{"AAASendMessage",				(cmd_function)AAASendMessage,			NO_SCRIPT, 0, 0},
	{"AAASendMessageToPeer",		(cmd_function)AAASendMessageToPeer,		NO_SCRIPT, 0, 0},
	{"AAASendRecvMessage",			(cmd_function)AAASendRecvMessage, 		NO_SCRIPT, 0, 0},
	{"AAASendRecvMessageToPeer",	(cmd_function)AAASendRecvMessageToPeer,	NO_SCRIPT, 0, 0},


	{"AAACreateSession",			(cmd_function)AAACreateSession,			NO_SCRIPT, 0, 0},
	{"AAADropSession",				(cmd_function)AAADropSession, 			NO_SCRIPT, 0, 0},

	{"AAACreateTransaction",		(cmd_function)AAACreateTransaction,		NO_SCRIPT, 0, 0},
	{"AAADropTransaction",			(cmd_function)AAADropTransaction, 		NO_SCRIPT, 0, 0},

	{"AAACreateAVP",				(cmd_function)AAACreateAVP, 			NO_SCRIPT, 0, 0},
	{"AAAAddAVPToMessage",			(cmd_function)AAAAddAVPToMessage,		NO_SCRIPT, 0, 0},
	{"AAAFindMatchingAVP",			(cmd_function)AAAFindMatchingAVP, 		NO_SCRIPT, 0, 0},
	{"AAAGetNextAVP",				(cmd_function)AAAGetNextAVP, 			NO_SCRIPT, 0, 0},
	{"AAAFreeAVP",					(cmd_function)AAAFreeAVP, 				NO_SCRIPT, 0, 0},
	{"AAAGroupAVPS",				(cmd_function)AAAGroupAVPS, 			NO_SCRIPT, 0, 0},
	{"AAAUngroupAVPS",				(cmd_function)AAAUngroupAVPS,			NO_SCRIPT, 0, 0},
	{"AAAFindMatchingAVPList",		(cmd_function)AAAFindMatchingAVPList,	NO_SCRIPT, 0, 0},
	{"AAAFreeAVPList",				(cmd_function)AAAFreeAVPList,			NO_SCRIPT, 0, 0},

	{"AAAAddRequestHandler",		(cmd_function)AAAAddRequestHandler, 	NO_SCRIPT, 0, 0},
	{"AAAAddResponseHandler",		(cmd_function)AAAAddResponseHandler,	NO_SCRIPT, 0, 0},
		
	{ 0, 0, 0, 0, 0 }
};


/**
 * Exported parameters.
 * - config_file - Configuration filename. See configdtd.h for the structure and ConfigExample.xml.
 */
static param_export_t cdp_params[] = {	
	{ "config_file",PARAM_STRING,&config_file}, /**< configuration filename */
	{ 0, 0, 0 }
};


/**
 * Exported module interface
 */
struct module_exports exports = {
	"cdp",
	cdp_cmds,                       /**< Exported functions */
	0,
	cdp_params,                     /**< Exported parameters */
	cdp_init,                   /**< Module initialization function */
	(response_function) 0,
	(destroy_function) cdp_exit,
	0,
	(child_init_function) cdp_child_init /**< per-child init function */
};




/** 
 * Module init function.
 * 
 * - Initializes the diameter peer using the provided configuration file.
 * - Registers with pt the required number of processes.
 */
static int cdp_init( void )
{
	LOG(L_INFO,"INFO:"M_NAME":cdp_init(): CDiameterPeer initializing\n");
	if (!diameter_peer_init(config_file)){
		LOG(L_CRIT,"ERR:"M_NAME":cdp_init(): error initializing the diameter peer\n");
		return 1;
	}
	register_procs(2+config->workers + 2 * config->peers_cnt);
	return 0;
}

/**
 *	Child init function.
 * - starts the DiameterPeer by forking the processes
 * @param rank - id of the child 
 */
static int cdp_child_init( int rank )
{
	if (rank == PROC_MAIN) { 
		LOG(L_INFO,"INFO:"M_NAME":cdp_child_init(): CDiameterPeer starting ...\n");		
		diameter_peer_start(0);
		LOG(L_INFO,"INFO:"M_NAME":cdp_child_init(): ... CDiameterPeer started\n");		
	}
	
	return 0;
}


/**
 *	Module termination function.
 * - stop the DiameterPeer processes in a civilized manner
 */
static int cdp_exit( void )
{
	LOG(L_INFO,"INFO:"M_NAME":cdp_exit(): CDiameterPeer stoping ...\n");		
	diameter_peer_destroy();
	LOG(L_INFO,"INFO:"M_NAME":cdp_exit(): ... CDiameterPeer stoped\n");		
	return 0;
}





