/*
 * IMS offline charging (Rf) module
 *
 * $Id$
 *  
 * Copyright (C) 2004-2007 FhG Fokus
 * Copyright (C) 2007 PT Inovacao
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
 * IMS offline charging (Rf) - SER module interface
 * 
 * Scope:
 * - Exports parameters and functions
 * - Initialization functions
 * - tm callbacks
 * 
 *  \author Joao Filipe Placido joao-f-placido -at- ptinovacao dot pt
 * 
 * Copyright (C) 2007 PT Inovacao
 * 
 */

#include <stdio.h>
#include <string.h>
#include <time.h>



#include "mod.h"
#include "../tm/t_hooks.h"
#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"
#include "../tm/h_table.h"
#include "../../parser/msg_parser.h"

//#include "../../usr_avp.h"
//#include "../../id.h"

//#include "../../parser/parse_rr.h"
//#include "../../trim.h"
//#include "../acc_syslog/attrs.h"
#include "offline_charging.h"  
#include "rf.h"
#include "sip.h"

MODULE_VERSION

struct tm_binds tmb;							/**< Structure with pointers to tm funcs 				*/
struct cdp_binds cdpb;							/**< Structure with pointers to cdp funcs				*/

static int mod_init( void );
static int mod_child_init(int rank);
static void mod_destroy(void);

static int fix_rf_flag( modparam_t type, void* val);
//static int fix_rf_missed_flag( modparam_t type, void* val);

static int rf_flag = 0;            	/* Flag that marks transactions to be accounted */
//static char* log_fmt = ALL_LOG_FMT; /* Formating string that controls what information will be collected and accounted */

/* parameters storage */
int interim_acct_interval=300;     /**< Default interim accounting interval = 300 s	*/
char* cdf_peer="cdf.open-ims.test";/**< FQDN of the Diameter Peer (CDF) */


struct offline_charging_flag cflag; 	/**< Charging flag */

/* fixed parameter storage */
str cdf_peer_str;					/**< fixed FQDN of the Diameter Peer (CDF) */
/* offline charging flag */ 
unsigned int cf_ietf = 0;					 
unsigned int cf_3gpp = 0;    



int * callback_singleton;				/**< Rf callback singleton 								*/
int * shutdown_singleton;				/**< Shutdown singleton 								*/

/*  
static int acc_rf_request0(struct sip_msg *rq, char *p1, char *p2);
//static int acc_rf_missed0(struct sip_msg *rq, char *p1, char *p2);
static int acc_rf_request1(struct sip_msg *rq, char *p1, char *p2);
//static int acc_rf_missed1(struct sip_msg *rq, char *p1, char *p2);
*/

/** 
 * Exported functions.
 * 
 */
static cmd_export_t cmds[] = {
/*	{"acc_rf_log",    acc_rf_request0, 0, 0,               REQUEST_ROUTE | FAILURE_ROUTE},  */
//	{"acc_rf_missed", acc_rf_missed0,  0, 0,               REQUEST_ROUTE | FAILURE_ROUTE},
/*	{"acc_rf_log",    acc_rf_request1, 1, fixup_var_int_1, REQUEST_ROUTE | FAILURE_ROUTE},  */
//	{"acc_rf_missed", acc_rf_missed1,  1, fixup_var_int_1, REQUEST_ROUTE | FAILURE_ROUTE},
	// TODO: export functions to script
	{0, 0, 0, 0, 0}
};

/** 
 * Exported parameters. 
 * - interim_acct_interval
 */	
static param_export_t params[] = {
	{"interim_acct_interval", 		PARAM_INT, &interim_acct_interval},
	{"rf_flag",		PARAM_INT, &rf_flag         	},
	{"rf_flag",		PARAM_STRING|PARAM_USE_FUNC, fix_rf_flag},
//	{"log_fmt",		PARAM_STRING, &log_fmt          },

   	/* address of cdf */
	{"cdf_peer",	PARAM_STRING, &cdf_peer	},
	
    {"cflag_ietf",				PARAM_INT,      &cf_ietf},
    {"cflag_3gpp",				PARAM_INT,		&cf_3gpp},
	{0, 0, 0}
};

/** module exports */
struct module_exports exports= {
	"acc_rf",
	cmds,     /* exported functions */
	0,        /* RPC methods */
	params,   /* exported params */
	mod_init, /* initialization module */
	0,	  /* response function */
	mod_destroy,        /* destroy function */
	0,	  /* oncancel function */
	mod_child_init       /* per-child init function */
};



/* fixes rf_flag param (resolves possible named flags) */
static int fix_rf_flag( modparam_t type, void* val)
{
	return fix_flag(type, val, "acc_rf", "rf_flag", &rf_flag);
}

/*
 * Return true if accounting is enabled and the
 * transaction is marked for accounting
 */
static inline int is_acc_on(struct sip_msg *rq)
{
	return rf_flag && isflagset(rq, rf_flag) == 1;
}


static inline void preparse_req(struct sip_msg *rq)
{
	     /* try to parse from for From-tag for accounted transactions;
	      * don't be worried about parsing outcome -- if it failed,
	      * we will report N/A. There is no need to parse digest credentials
	      * here even if we account them, because the authentication function
	      * will do it before us and if not then we will account n/a.
	      */
	parse_headers(rq, HDR_CALLID_F | HDR_FROM_F | HDR_TO_F | HDR_CSEQ_F | HDR_ROUTE_F, 0 );
	//parse_from_header(rq);
}






static void log_reply(struct cell* t, struct sip_msg* reply, unsigned int code, time_t req_time)
{
	str tag;
	AAAAcctMessageType acct_record_type = AAA_ACCT_EVENT;
	//AAAMessage *acr=0;
	AAAAcctSession *acc_s;
	struct sip_msg *req = (t->uas).request;
	char send=0;

	if (code < 200) return;
	else if (code == 200) {
		switch (req->first_line.u.request.method.len) {
			case 6:
				if (!memcmp("INVITE", req->first_line.u.request.method.s, 6)) {
					tag = get_to(req)->tag_value;
					if (tag.s == 0 || tag.len == 0) {
						acct_record_type = AAA_ACCT_START;
						send=1;
						LOG(L_INFO,"acc_rf:log_reply(): 200 OK to INVITE\n");
					} else {
						acct_record_type = AAA_ACCT_INTERIM;
						send=1;
						LOG(L_INFO,"acc_rf:log_reply(): 200 OK to re-INVITE\n");
					}
				} else if (!memcmp("NOTIFY", req->first_line.u.request.method.s, 6)) {
					acct_record_type = AAA_ACCT_EVENT;
					send=1;
					LOG(L_INFO,"acc_rf:log_reply(): 200 OK to NOTIFY\n");
				} else if (!memcmp("UPDATE", req->first_line.u.request.method.s, 6)) {
					acct_record_type = AAA_ACCT_INTERIM;
					send=1;
					LOG(L_INFO,"acc_rf:log_reply(): 200 OK to UPDATE\n");
				}
				break;
			case 7:
				if (!memcmp("MESSAGE", req->first_line.u.request.method.s, 7) ||
					!memcmp("PUBLISH", req->first_line.u.request.method.s, 7)) {
					acct_record_type = AAA_ACCT_EVENT;
					send=1;
					LOG(L_INFO,"acc_rf:log_reply(): 200 OK to MESSAGE or PUBLISH\n");
				}
				break;
			case 8:
				if (!memcmp("REGISTER", req->first_line.u.request.method.s, 8)) {
					acct_record_type = AAA_ACCT_EVENT;
					send=1;
					LOG(L_INFO,"acc_rf:log_reply(): 200 OK to REGISTER\n");
				}
				break;
			case 9:
				if (!memcmp("SUBSCRIBE", req->first_line.u.request.method.s, 9)) {
					// we are accounting only initial SUBSCRIBE
					tag = get_to(req)->tag_value;
					if (tag.s == 0 || tag.len == 0) {
						
						// TODO: ignore reg event subscriptions
						//if ((reply && reply != FAKED_REPLY)) {
						//	parse_headers(ps->rpl, HDR_EVENT_F, 0);
						//}
						
						
						acct_record_type = AAA_ACCT_EVENT;
						send=1;
						LOG(L_INFO,"acc_rf:log_reply(): 200 OK to SUBSCRIBE\n");
					}
				}
				break;
			default:
				LOG(L_INFO,"acc_rf:log_reply(): 200 OK to ?\n");
		}
	} else if (code > 200) {
			acct_record_type = AAA_ACCT_EVENT;
			send=1; 
			LOG(L_INFO,"acc_rf:log_reply(): reply with code > 200\n");
	}

	if (send) {		 
		switch (acct_record_type) {
			case AAA_ACCT_INTERIM:
				LOG(L_INFO,"INFO:"M_NAME":log_reply: Sending Accounting Interim...\n");
				str dlgid = cscf_get_call_id(req, 0);
				acc_s = cdpb.AAAGetAcctSession(&dlgid);
				Rf_ACR_interim(req, acc_s);
		  		break;
		  	case AAA_ACCT_START:
		  		LOG(L_INFO,"INFO:"M_NAME":log_reply: Sending Accounting Start...\n");
				Rf_ACR_start(req);
				break;
			case AAA_ACCT_EVENT: 
				LOG(L_INFO,"INFO:"M_NAME":log_reply: Sending Accounting Event...\n");
				Rf_ACR_event(req);
				break;
			default:
				LOG(L_ERR, "BUG:"M_NAME":log_reply: Bad acct_record_type\n");
		}
	}
}




/* these wrappers parse all what may be needed; they don't care about
 * the result -- accounting functions just display "unavailable" if there
 * is nothing meaningful
 */
/*static int acc_rf_request0(struct sip_msg *rq, char* p1, char* p2)
{
	preparse_req(rq);
	return log_request(rq, GET_RURI(rq), rq->to, 0, time(0));
}*/


/* these wrappers parse all what may be needed; they don't care about
 * the result -- accounting functions just display "unavailable" if there
 * is nothing meaningful
 */
/*static int acc_rf_missed0(struct sip_msg *rq, char* p1, char* p2)
{
	preparse_req(rq);
	return log_request(rq, GET_RURI(rq), rq->to, 0, time(0));
}*/

/* these wrappers parse all what may be needed; they don't care about
 * the result -- accounting functions just display "unavailable" if there
 * is nothing meaningful
 */
/*static int acc_rf_request1(struct sip_msg *rq, char* p1, char* p2)
{
    int code;
    preparse_req(rq);
    if (get_int_fparam(&code, rq, (fparam_t*)p1) < 0) {
	code = 0;
    }
    return log_request(rq, GET_RURI(rq), rq->to, code, time(0));
}*/


/* these wrappers parse all what may be needed; they don't care about
 * the result -- accounting functions just display "unavailable" if there
 * is nothing meaningful
 */
/*static int acc_rf_missed1(struct sip_msg *rq, char* p1, char* p2)
{
    int code;
    preparse_req(rq);
    if (get_int_fparam(&code, rq, (fparam_t*)p1) < 0) {
	code = 0;
    }
    return log_request(rq, GET_RURI(rq), rq->to, code, time(0));
}*/



/* initiate a report if we previously enabled MC accounting for this t */
static void failure_handler(struct cell *t, int type, struct tmcb_params* ps)
{
	// validation
	if (t->uas.request == 0) {
		DBG("DBG:acc_rf:failure_handler: No uas.request, skipping local transaction\n");
		return;
	}
/*
	if (is_invite(t) && ps->code >= 300) {

	}
*/
}


/* initiate a report if we previously enabled accounting for this t */
static void replyout_handler(struct cell* t, int type, struct tmcb_params* ps)
{
	if (t->uas.request == 0) {
		DBG("DBG:acc:replyout_handler: No uas.request, local transaction, skipping\n");
		return;
	}

	 /* acc_onreply is bound to TMCB_REPLY which may be called
	  * from _reply, like when FR hits; we should not miss this
	  * event for missed calls either
	  */
	//failure_handler(t, type, ps);
	
//	if (!should_acc_reply(t, ps->code)) return;
	//if (is_acc_on(t->uas.request)) {
	LOG(L_INFO,"acc_rf:replyout_handler! reply with code=%d\n", ps->code);
	log_reply(t, ps->rpl, ps->code, (time_t)*(ps->param));
	//}
}


/* parse incoming replies before cloning */
static void replyin_handler(struct cell *t, int type, struct tmcb_params* ps)
{
	//validation
	if (t->uas.request == 0) {
		LOG(L_ERR, "ERROR:acc_rf:replyin_handler:replyin_handler: 0 request\n");
		return;
	}

	// don't parse replies in which we are not interested 
	// missed calls enabled ? 
	//if (((is_invite(t) && ps->code >= 300 && is_mc_on(t->uas.request))
	 //    || should_acc_reply(t, ps->code))
	  //  && (ps->rpl && ps->rpl != FAKED_REPLY)) {
	//	parse_headers(ps->rpl, HDR_TO_F, 0);
	//}
	
	LOG(L_INFO,"acc_rf:replyin_handler! reply with code=%d\n", ps->code);
	//log_reply(t, ps->rpl, ps->code, (time_t)*(ps->param));
}


/* prepare message and transaction context for later accounting */
void on_req(struct cell* t, int type, struct tmcb_params *ps)
{
	time_t req_time;
	//AAAMessage *acr=0;
	AAAAcctSession *acc_s = 0;

	/* do some parsing in advance */
	preparse_req(ps->req);

	if (ps->req->first_line.u.request.method.len == 3) {
		if (!memcmp("BYE", ps->req->first_line.u.request.method.s, 3)) {
			str dlgid = cscf_get_call_id(ps->req, 0);
			acc_s = cdpb.AAAGetAcctSession(&dlgid);
			Rf_ACR_stop(ps->req, acc_s);
			//Rf_ACR_stop(ps->req);
			LOG(L_INFO,"acc_rf:on_req(): BYE request\n");
			return;
		}
	} else if (ps->req->first_line.u.request.method.len == 6) {
		if (!memcmp("CANCEL", ps->req->first_line.u.request.method.s, 3)) {
			Rf_ACR_event(ps->req);
			LOG(L_INFO,"acc_rf:on_req(): CANCEL request\n");
			return;
		}
	}
	
	/* Pass the timestamp of the request as a parameter to callbacks */
	req_time = time(0);

//	if (is_acc_on(ps->req)/* || is_mc_on(ps->req)*/) {
		if (tmb.register_tmcb(0, t, TMCB_RESPONSE_OUT, replyout_handler, (void*)req_time) <= 0) {
			LOG(L_ERR, "ERROR:acc:on_req: Error while registering TMCB_RESPONSE_OUT callback\n");
			return;
		}

		if (tmb.register_tmcb(0, t, TMCB_ON_FAILURE_RO, failure_handler, (void*)req_time) <= 0) {
			LOG(L_ERR, "ERROR:acc:on_req: Error while registering TMCB_ON_FAILURE_RO callback\n");
			return;
		}

		if (tmb.register_tmcb(0, t, TMCB_RESPONSE_IN, replyin_handler, (void*)req_time) <= 0) {
			LOG(L_ERR, "ERROR:acc:on_req: Error while registering TMCB_RESPONSE_IN callback\n");
			return;
		}


		/* also, if that is INVITE, disallow silent t-drop */
/*		if (ps->req->REQ_METHOD == METHOD_INVITE) {
			DBG("DEBUG: noisy_timer set for accounting\n");
			t->flags |= T_NOISY_CTIMER_FLAG;
		}
*/
//	}
}


static int mod_init(void)
{
	load_tm_f load_tm;
	load_cdp_f load_cdp;

	callback_singleton=shm_malloc(sizeof(int));
	*callback_singleton=0;
	shutdown_singleton=shm_malloc(sizeof(int));
	*shutdown_singleton=0;

	LOG(L_INFO,"INFO:"M_NAME":mod_init: Initialization of module\n");
	/* fix the parameters */
	cdf_peer_str.s = cdf_peer;
	cdf_peer_str.len = strlen(cdf_peer);
	
	/* Set offline charging flag */
	cflag.cf_ietf = (cf_ietf) ? cf_ietf : CF_IETF_DEFAULT;
	cflag.cf_3gpp = (cf_3gpp) ? cf_3gpp : CF_3GPP_DEFAULT;
	
	LOG(L_INFO,"INFO:"M_NAME":mod_init: cf_3gpp=%x\n",cf_3gpp);

	/* import the TM auto-loading function */
	if ( !(load_tm=(load_tm_f)find_export("load_tm", NO_SCRIPT, 0))) {
		LOG(L_ERR, "ERROR:acc:mod_init: can't import load_tm\n");
		return -1;
	}
	/* let the auto-loading function load all TM stuff */
	if (load_tm( &tmb )==-1) return -1;
	
	/* bind to the cdp module */
	if (!(load_cdp = (load_cdp_f)find_export("load_cdp",NO_SCRIPT,0))) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: Can not import load_cdp. This module requires cdp module\n");
		goto error;
	}
	if (load_cdp(&cdpb) == -1)
		goto error;
	
//	if (verify_fmt(log_fmt)==-1) return -1;

	/* register callbacks*/
	/* listen for all incoming requests  */
	if (tmb.register_tmcb( 0, 0, TMCB_REQUEST_IN, on_req, 0 ) <= 0) {
		LOG(L_ERR,"ERROR:acc:mod_init: cannot register TMCB_REQUEST_IN "
		    "callback\n");
		return -1;
	}

	/* register response callback */
	//cdpb.AAAAddResponseHandler(&RfAnswerHandler,0);

	return 0;
error:
	return -1;
}


extern gen_lock_t* process_lock;		/* lock on the process table */

/**
 * Initializes the module in child.
 */
static int mod_child_init(int rank)
{
	LOG(L_INFO,"INFO:"M_NAME":mod_child_init: Initialization of module in child [%d] \n",
		rank);
	/* don't do anything for main process and TCP manager process */
	if ( rank == PROC_MAIN || rank == PROC_TCP_MAIN )
		return 0;

	
	/* init the diameter callback - must be done just once */
	lock_get(process_lock);
	if((*callback_singleton)==0){
		*callback_singleton=1;
		cdpb.AAAAddRequestHandler(RfRequestHandler,NULL);
	}
	lock_release(process_lock);
	/* Init the user data parser */
	//if (!parser_init(scscf_user_data_dtd,scscf_user_data_xsd)) return -1;
		
	return 0;
}


/**
 * Destroys the modules 
 */
static void mod_destroy(void)
{
	LOG(L_INFO,"INFO:"M_NAME":mod_destroy: child exit\n");
//	lock_get(process_lock);
//	if((*shutdown_singleton)==0){
//		*shutdown_singleton=1;
//	}
//	lock_release(process_lock);
}





