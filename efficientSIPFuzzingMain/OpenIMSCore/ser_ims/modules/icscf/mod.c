/**
 * $Id: mod.c 408 2007-07-24 15:16:36Z vingarzan $
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
 * Interrogating-CSCF - SER module interface
 * 
 * Scope:
 * - Exports parameters and functions
 * - Initialization functions
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 

#include "mod.h"

#include "../../sr_module.h"
#include "../../timer.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"

#include "db.h"
#include "nds.h"
#include "scscf_list.h"
#include "registration.h"
#include "location.h"
#include "cx.h"
#include "sip.h"

#include "thig_ims_enc.h"
#include "thig.h"

#include "ims_pm_icscf.h"

MODULE_VERSION

static int icscf_mod_init(void);
static int icscf_mod_child_init(int rank);
static void icscf_mod_destroy(void);


/** parameters storage */
char* icscf_name="icscf.open-ims.test";					/**< name of the I-CSCF */
char* icscf_thig_name="thig@icscf.open-ims.test";		/**< name of the I-CSCF for THIG */
char* icscf_thig_host="127.0.0.1";						/**< host for THIG */
int   icscf_thig_port=5060;								/**< port for THIG */
char* icscf_thig_param="thigenc";						/**< THIG parameter name */

char* icscf_db_url="mysql://icscf:heslo@localhost/hssdata";	/**< DB URL */
char* icscf_db_nds_table="nds_trusted_domains";				/**< NDS table in DB */
char* icscf_db_scscf_table="s_cscf";						/**< S-CSCF table in db */
char* icscf_db_capabilities_table="s_cscf_capabilities";	/**< S-CSCF capabilities table in db */

char* icscf_forced_hss_peer="";								/**< Forced Diameter Peer FQDN (HSS) */

int icscf_hash_size=128;									/**< size of the hash for storing S-CSCF lists	*/

char* icscf_default_realm="open-ims.test";					/**< default realm for LIR if none available	*/

static char* icscf_route_on_term_user_unknown=0;					/**< script route to run for Initial request after HSS replies with User Unknown to the LIR for terminating user (default none)*/
int route_on_term_user_unknown_n=-1;				


/* P-Charging-Vector parameters */
extern char* cscf_icid_value_prefix;			/**< hexadecimal prefix for the icid-value - must be unique on each node */
extern unsigned int* cscf_icid_value_count;		/**< to keep the number of generated icid-values 	*/
extern gen_lock_t* cscf_icid_value_count_lock;	/**< to lock acces on the above counter				*/
extern char* cscf_icid_gen_addr;				/**< address of the generator of the icid-value 	*/
extern char* cscf_orig_ioi;						/**< name of the Originating network 				*/
extern char* cscf_term_ioi;						/**< name of the Terminating network 				*/

/* fixed parameter storage */
str icscf_name_str;			/**< fixed name of the I-CSCF */
str icscf_thig_name_str;	/**< fixed name of the I-CSCF for THIG */
str icscf_thig_host_str;	/**< fixed host for THIG */
str icscf_thig_port_str;	/**< fixed port for THIG */
str icscf_thig_param_str;	/**< fixed THIG parameter name */
str icscf_thig_path_str;	/**< fixed Path header */
str icscf_thig_rr_str;		/**< fixed Record-route header */
str icscf_default_realm_str;/**< fixed default realm */
str icscf_forced_hss_peer_str;				/**< fixed forced Diameter Peer FQDN (HSS) */

extern str cscf_icid_value_prefix_str;				/**< fixed hexadecimal prefix for the icid-value - must be unique on each node */
extern str cscf_icid_gen_addr_str;					/**< fixed address of the generator of the icid-value */
extern str cscf_orig_ioi_str;						/**< fixed name of the Originating network 			*/
extern str cscf_term_ioi_str;						/**< fixed name of the Terminating network 			*/

/* twofish encryption variables (THIG) **/
keyInstance    ki;			/**< key information, including tables */
cipherInstance ci;			/**< keeps mode (ECB, CBC) and IV */

#ifdef WITH_IMS_PM
	/** IMS PM parameters storage */
	char* ims_pm_node_type="I-CSCF";
	char* ims_pm_logfile="/opt/OpenIMSCore/default_ims_pm.log";
#endif /* WITH_IMS_PM */


/** 
 * Exported functions.
 * - I_NDS_check_trusted() - check if the message comes from a trusted domain
 * - I_NDS_is_trusted() - check if the message comes from a trusted domain
 * - I_NDS_strip_headers() - strip untrusted headers for requests coming from untrusted domains 
 * <p>
 * - I_trans_in_processing() - if this transaction is already in processing
 * - I_UAR() - trigger an UAR and fwd the message based on the UAA  
 * - I_LIR() - trigger an LIR and fwd the message based on the UAA
 * - I_scscf_select() - select the next unused S-CSCF from the list for this request
 * - I_scscf_drop() - drop the list of S-CSCFs for this request
 * <p>
 * - I_THIG_add_Path() - add Path header for THIG encryption
 * - I_THIG_add_RR() - add Record-Route header for THIG encryption
 * - I_THIG_encrypt_header() - encrypt a specific header for THIG 
 * - I_THIG_encrypt_all_headers() - encrypt all sensitive headers for THIG 
 * - I_THIG_decrypt_header() - decrypt a specific header for THIG 
 * - I_THIG_decrypt_all_headers() - decrypt all sensitive headers for THIG
 * <p>
 * - icid_value_prefix - prefix for the ICID in the P-Charging-Vector header
 * - icid_gen_addr - ICID Gen Addr. in the P-Charging-Vector header
 * - orig_ioi - Originating IOI in the P-Charging-Vector header
 * - term_ioi - Terminating IOI in the P-Charging-Vector header
 */
static cmd_export_t icscf_cmds[]={
	{"I_NDS_check_trusted", 		I_NDS_check_trusted, 		0, 0, REQUEST_ROUTE}, 
	{"I_NDS_is_trusted", 			I_NDS_is_trusted, 			0, 0, REQUEST_ROUTE}, 
	{"I_NDS_strip_headers", 		I_NDS_strip_headers, 		0, 0, REQUEST_ROUTE}, 

	{"I_trans_in_processing", 		I_trans_in_processing, 		0, 0, REQUEST_ROUTE}, 
	{"I_UAR", 						I_UAR, 						1, 0, REQUEST_ROUTE}, 
	{"I_LIR", 						I_LIR, 						0, 0, REQUEST_ROUTE}, 	
	{"I_scscf_select",				I_scscf_select,				1, 0, REQUEST_ROUTE|FAILURE_ROUTE}, 
	{"I_scscf_drop",				I_scscf_drop,				0, 0, REQUEST_ROUTE|ONREPLY_ROUTE|FAILURE_ROUTE}, 
	
	{"I_THIG_add_Path", 			I_THIG_add_Path,			0, 0, REQUEST_ROUTE},
	{"I_THIG_add_RR", 				I_THIG_add_RR,				0, 0, REQUEST_ROUTE},
	{"I_THIG_encrypt_header", 		I_THIG_encrypt_header,		1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"I_THIG_encrypt_all_headers",	I_THIG_encrypt_all_headers,	0, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"I_THIG_decrypt_header", 		I_THIG_decrypt_header,		1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"I_THIG_decrypt_all_headers", 	I_THIG_decrypt_all_headers,	0, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	
	{"I_add_p_charging_vector",		I_add_p_charging_vector, 	0, 0, REQUEST_ROUTE},
	
	{"I_originating",				I_originating, 				0, 0, REQUEST_ROUTE},
	
	{0, 0, 0, 0, 0}
};

/**
 * Exported parameters.
 * - name - name of the I-CSCF
 * - db_url - URL of the database containing NDS and S-CSCF information
 * - db_nds_table - name of the table containing the NDS information 
 * - db_scscf_table - name of the table containing the S-CSCF information 
 * - db_capabilities_table - name of the table containing the S-CSCF capabilities information
 * <p>
 * - thig_name - name of the I-CSCF with THIG
 * - thig_host - IP address of the I-CSCF with THIG
 * - thig_port - port of the I-CSCF with THIG
 * - thig_param - name of the URI parameter to encode data into
 */	
static param_export_t icscf_params[]={ 
	{"name", 					STR_PARAM, &icscf_name},
	{"db_url", 					STR_PARAM, &icscf_db_url},
	{"db_nds_table", 			STR_PARAM, &icscf_db_nds_table},
	{"db_scscf_table", 			STR_PARAM, &icscf_db_scscf_table},
	{"db_capabilities_table", 	STR_PARAM, &icscf_db_capabilities_table},
	{"forced_hss_peer", 		STR_PARAM, &icscf_forced_hss_peer},
	{"hash_size", 				INT_PARAM, &icscf_hash_size},	

	{"default_realm", 			STR_PARAM, &icscf_default_realm},	

	{"thig_name", 				STR_PARAM, &icscf_thig_name},
	{"thig_host", 				STR_PARAM, &icscf_thig_host},
	{"thig_port", 				INT_PARAM, &icscf_thig_port},
	{"thig_param",				STR_PARAM, &icscf_thig_param},

	{"icid_value_prefix",		STR_PARAM, &cscf_icid_value_prefix},
	{"icid_gen_addr",			STR_PARAM, &cscf_icid_gen_addr},
	{"orig_ioi",				STR_PARAM, &cscf_orig_ioi},
	{"term_ioi",				STR_PARAM, &cscf_term_ioi},
	
	{"route_on_term_user_unknown", 	STR_PARAM, &icscf_route_on_term_user_unknown},

#ifdef WITH_IMS_PM
	{"ims_pm_node_type",		STR_PARAM, &ims_pm_node_type},
	{"ims_pm_logfile",			STR_PARAM, &ims_pm_logfile},
#endif /* WITH_IMS_PM */

	{0,0,0} 
};

/** module exports */
struct module_exports exports = {
	"icscf", 
	icscf_cmds,
	0,
	icscf_params,
	
	icscf_mod_init,		/* module initialization function */
	0,				/* response function*/
	icscf_mod_destroy,	/* destroy function */
	0,				/* onbreak function */
	icscf_mod_child_init	/* per-child init function */
};


/** Global variables and imported functions */
int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 
										/**< link to the stateless reply function in sl module */

struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/
struct cdp_binds cdpb;            /**< Structure with pointers to cdp funcs 		*/


static str s_path_s={"Path: <sip:",11};
static str s_path_e={";lr>\r\n",6};
static str s_rr_s={"Record-Route: <sip:",19};
static str s_rr_e={";lr>\r\n",6};

static int fix_parameters()
{
	icscf_name_str.s = icscf_name;
	icscf_name_str.len = strlen(icscf_name);
	
	icscf_thig_name_str.s = icscf_thig_name;
	icscf_thig_name_str.len = strlen(icscf_thig_name);

	icscf_thig_host_str.s = icscf_thig_host;
	icscf_thig_host_str.len = strlen(icscf_thig_host);
	
	icscf_thig_port_str.s = pkg_malloc(12);
	if (!icscf_thig_port_str.s){
		LOG(L_ERR,"ERR:"M_NAME":fix_parameters: error allocating %d bytes\n",12);
		return 0;
	}
	sprintf(icscf_thig_port_str.s,"%d",icscf_thig_port);
	icscf_thig_port_str.len = strlen(icscf_thig_port_str.s);

	
	icscf_thig_param_str.s = icscf_thig_param;
	icscf_thig_param_str.len = strlen(icscf_thig_param);
	
	icscf_thig_path_str.len = s_path_s.len+icscf_thig_name_str.len+s_path_e.len;
	icscf_thig_path_str.s = pkg_malloc(icscf_thig_path_str.len);
	if (!icscf_thig_path_str.s){
		LOG(L_ERR,"ERR:"M_NAME":fix_parameters: error allocating %d bytes\n",icscf_thig_path_str.len);
		return 0;
	}
	icscf_thig_path_str.len = 0;
	STR_APPEND(icscf_thig_path_str,s_path_s);
	STR_APPEND(icscf_thig_path_str,icscf_thig_name_str);
	STR_APPEND(icscf_thig_path_str,s_path_e);

	icscf_thig_rr_str.len = s_rr_s.len+icscf_thig_name_str.len+s_rr_e.len;
	icscf_thig_rr_str.s = pkg_malloc(icscf_thig_rr_str.len);
	if (!icscf_thig_rr_str.s){
		LOG(L_ERR,"ERR:"M_NAME":fix_parameters: error allocating %d bytes\n",icscf_thig_rr_str.len);
		return 0;
	}
	icscf_thig_rr_str.len = 0;
	STR_APPEND(icscf_thig_rr_str,s_rr_s);
	STR_APPEND(icscf_thig_rr_str,icscf_thig_name_str);
	STR_APPEND(icscf_thig_rr_str,s_rr_e);
				
	icscf_forced_hss_peer_str.s = icscf_forced_hss_peer;
	icscf_forced_hss_peer_str.len = strlen(icscf_forced_hss_peer);

	icscf_default_realm_str.s = icscf_default_realm; 
	icscf_default_realm_str.len = strlen(icscf_default_realm);

	cscf_icid_value_prefix_str.s = cscf_icid_value_prefix;
	cscf_icid_value_prefix_str.len = strlen(cscf_icid_value_prefix);

	cscf_icid_gen_addr_str.s = cscf_icid_gen_addr;
	cscf_icid_gen_addr_str.len = strlen(cscf_icid_gen_addr);
	
	cscf_orig_ioi_str.s = cscf_orig_ioi;
	cscf_orig_ioi_str.len = strlen(cscf_orig_ioi);
	
	cscf_term_ioi_str.s = cscf_term_ioi;
	cscf_term_ioi_str.len = strlen(cscf_term_ioi);
	
	return 1;
}

static int icscf_mod_init(void)
{
	load_tm_f load_tm;
	load_cdp_f load_cdp;
	LOG(L_INFO,"INFO:"M_NAME":mod_init: Initialization of module\n");
	/* fix the parameters */
	if (!fix_parameters()) goto error;

	#ifdef WITH_IMS_PM
		ims_pm_init(icscf_name_str,ims_pm_node_type, ims_pm_logfile);
		ims_pm_init_icscf();
	#endif /* WITH_IMS_PM */
	
	cscf_icid_value_count = shm_malloc(sizeof(unsigned int));
	*cscf_icid_value_count = 0;
	cscf_icid_value_count_lock = lock_alloc();
	cscf_icid_value_count_lock = lock_init(cscf_icid_value_count_lock);
		
	/* load the send_reply function from sl module */
    sl_reply = find_export("sl_send_reply", 2, 0);
	if (!sl_reply) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: This module requires sl module\n");
		goto error;
	}
	
	/* bind to the db module */
	if ( icscf_db_bind( icscf_db_url ) < 0 ) goto error;
	
	/* bind to the tm module */
	if (!(load_tm = (load_tm_f)find_export("load_tm",NO_SCRIPT,0))) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: Can not import load_tm. This module requires tm module\n");
		goto error;
	}
	if (load_tm(&tmb) == -1)
		goto error;

	/* bind to the cdp module */
	if (!(load_cdp = (load_cdp_f)find_export("load_cdp",NO_SCRIPT,0))) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: Can not import load_cdp. This module requires cdp module\n");
		goto error;
	}
	if (load_cdp(&cdpb) == -1)
		goto error;

	/* cache the trusted domain names and capabilities */
	I_NDS_get_trusted_domains();
	I_get_capabilities();
			
	if (!i_hash_table_init(icscf_hash_size)){
		LOG(L_ERR, "ERR"M_NAME":mod_init: Error initializing the Hash Table for stored S-CSCF lists\n");
		goto error;
	}	
//	//TODO - only for S-CSCF - I-CSCF does not respond to request
////	AAAAddRequestHandler(&CxRequestHandler,0);
//	/* don't register response callback as we always set callback per transactions 
//	 *  and we're not interested in other responses */
//	/*AAAAddResponseHandler(&CxAnswerHandler,0);*/
//
	/** initialize twofish variables and tables (THIG)**/
	srand((unsigned) time(NULL));
	thig_key_and_cipher_init(&ki,&ci);
	LOG(L_INFO,"Twofish encryption ready\n");
	
	int route_no;
	/* try to fix the icscf_route_on_term_user_unknown route */
	if (icscf_route_on_term_user_unknown){
		route_no=route_get(&main_rt, icscf_route_on_term_user_unknown);
		if (route_no==-1){
			LOG(L_ERR, "ERR"M_NAME":mod_init: failed to fix route \"%s\": route_get() failed\n",
					icscf_route_on_term_user_unknown);
			return -1;
		}
		if (main_rt.rlist[route_no]==0){
			LOG(L_ERR, "ERR"M_NAME":mod_init: icscf_route_on_term_user_unknown \"%s\" is empty / doesn't exist\n",
					icscf_route_on_term_user_unknown);
		}
		route_on_term_user_unknown_n=route_no;
	}	
	
	return 0;
error:
	return -1;
}

static int icscf_mod_child_init(int rank)
{
	LOG(L_INFO,"INFO:"M_NAME":mod_init: Initialization of module in child [%d] %s \n",
		rank,pt[process_no].desc);
	/* don't do anything for main process and TCP manager process */
	if ( rank == PROC_MAIN || rank == PROC_TCP_MAIN )
		return 0;
	
	/* db child init */
	icscf_db_init( icscf_db_url, 
		icscf_db_nds_table,
		icscf_db_scscf_table,
		icscf_db_capabilities_table);
		
	return 0;
}

static void icscf_mod_destroy(void)
{
	LOG(L_INFO,"INFO:"M_NAME":mod_destroy: child exit\n");
	i_hash_table_destroy();
	#ifdef WITH_IMS_PM
		ims_pm_destroy();	
	#endif /* WITH_IMS_PM */		
}





/**
 * Inserts the P-Charging-Vector header
 * P-Charging-Vector:
 * @param msg - the SIP message to add to
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok or #CSCF_RETURN_FALSE on error
 */
int I_add_p_charging_vector(struct sip_msg *msg,char *str1,char*str2)
{
	return cscf_add_p_charging_vector(msg);
}
