/*
 * $Id: third_party_reg.c 430 2007-08-01 13:18:42Z vingarzan $
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
 * Serving-CSCF - Third party register towards AS
 * 
 *  \author Erling Klaeboe klaboe -at- colibria dot com
 * 
 */
 
#include "third_party_reg.h"

#include "mod.h" 
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../parser/parse_uri.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../scscf/scscf_load.h"
#include "sip.h"
#include "ims_pm.h"

extern struct tm_binds isc_tmb; /**< Structure with pointers to tm funcs 		*/

extern str isc_my_uri;			/**< Uri of myself to loop the message in str   */
extern str isc_my_uri_sip;		/**< Uri of myself to loop the message in str with leading "sip:" */

extern int isc_expires_grace;	/**< expires value to add to the expires in the 3rd party register
										 to prevent expiration in AS */


extern struct scscf_binds isc_scscfb;      /**< Structure with pointers to S-CSCF funcs 	*/

/**
 * Handle third party registration
 * @param msg - the SIP REGISTER message
 * @param m  - the isc_match that matched with info about where to forward it 
 * @param mark  - the isc_mark that should be used to mark the message
 * @returns #ISC_RETURN_TRUE if allowed, #ISC_RETURN_FALSE if not
 */
int isc_third_party_reg(struct sip_msg *msg, isc_match *m,isc_mark *mark)
{
	r_third_party_registration *r;
//	int expires_hdr=0;
	int expires=0;
	str req_uri ={0,0};
	str to ={0,0};
	str pani ={0,0};
	str cv ={0,0};

	struct hdr_field *hdr;

	LOG(L_INFO,"INFO:"M_NAME":isc_third_party_reg: Enter\n");

	/* Set Request Uri to IFC matching server name */
	req_uri.len = m->server_name.len;
	req_uri.s = m->server_name.s;

	/* Get To header*/
	to = cscf_get_public_identity(msg);

//	/* Get Expire header */
//	expires_hdr = cscf_get_expires_hdr(msg);
//	if (expires_hdr == -1) expires_hdr = isc_scscfb.registration_default_expires;
//	if (expires_hdr > 0) {
//		if (expires_hdr < isc_scscfb.registration_min_expires)
//			expires_hdr = isc_scscfb.registration_min_expires;
//		if (expires_hdr > isc_scscfb.registration_max_expires) 
//			expires_hdr = isc_scscfb.registration_max_expires;                
//	}

	/* Get Expires from registrar */
//	expires_hdr = cscf_get_expires_hdr(msg);
//	if (expires_hdr==0) expires = 0;
//	else {
//		expires = isc_scscfb.get_r_public_expires(to);
//		if (expires==-999){
//			expires = expires_hdr;
//		}
//	}	

	/*TODO - check if the min/max expires is in the acceptable limits
	 * this does not work correctly if the user has multiple contacts
	 * and register/deregisters them individually!!!
	 */
	expires = cscf_get_max_expires(msg);
	
	/* Get P-Access-Network-Info header */
	pani = cscf_get_access_network_info(msg, &hdr);
	
	/* Get P-Charging-Vector header */
	/* Just forward the charging header received from P-CSCF */
	/* Todo: implement also according to TS 24.229, chap 5.4.1.7 */
	cv =   cscf_get_charging_vector(msg, &hdr);

	if (req_uri.s){
		r = new_r_third_party_reg(req_uri, to, isc_my_uri_sip, pani, cv);
		if (!r){
			LOG(L_ERR,"ERR:"M_NAME":isc_third_party_reg: Error creating new third party registration\n");
			return ISC_RETURN_FALSE;
		}
		if (expires<=0) r_send_third_party_reg(r,0);
		else r_send_third_party_reg(r,expires+isc_expires_grace);
		free_r_registration(r);
		return ISC_RETURN_TRUE;
	}else{
		return ISC_RETURN_FALSE;
	}	
}


/**
 * Creates a third party registration based on the given parameters.
 * @param req_uri - the AS to send third party register to
 * @param to - the To header
 * @param from - the From header
 * @param pani - P-Access-Network-Info header
 * @param cv - P-Charging-Vector header to use
 * @returns the r_third_party_registration or NULL on error
 */

r_third_party_registration* new_r_third_party_reg(str req_uri, str to, str from, str pani, str cv)
{
        r_third_party_registration *r=0;

	LOG(L_DBG,"DBG:"M_NAME":new_r_third_party_reg: Enter\n");

        r = shm_malloc(sizeof(r_third_party_registration));
        if (!r){
                LOG(L_ERR,"ERR:"M_NAME":new_r_third_party_reg: Error allocating %d bytes\n",
                        sizeof(r_third_party_registration));
                goto error;
        }
        memset(r,0,sizeof(r_third_party_registration));

        STR_SHM_DUP(r->req_uri,req_uri,"new_r_third_party_reg");
        if (!r->req_uri.s) goto error;

        STR_SHM_DUP(r->to,to,"new_r_third_party_reg");
        if (!r->to.s) goto error;

        STR_SHM_DUP(r->from,from,"new_r_third_party_reg");
        if (!r->from.s) goto error;

        STR_SHM_DUP(r->pani,pani,"new_r_third_party_reg");
        if (!r->pani.s) goto error;

        STR_SHM_DUP(r->cv,cv,"new_r_third_party_reg");
        if (!r->cv.s) goto error;

        return r;
error:
out_of_memory:
        free_r_registration(r);
        return 0;
}


static str method={"REGISTER",8};
static str event_hdr={"Event: registration\r\n",21};
static str content_len_hdr={"Content-Length: 0\r\n",19};
static str max_fwds_hdr={"Max-Forwards: 10\r\n",18};
static str expires_s={"Expires: ",9};
static str expires_e={"\r\n",2};
static str contact_s={"Contact: <",10};
static str contact_e={">\r\n",3};

static str p_access_network_info_s={"P-Access-Network-Info: ",23};
static str p_access_network_info_e={"\r\n",2};

static str p_charging_vector_s={"P-Charging-Vector: ",19};
static str p_charging_vector_e={"\r\n",2};

#ifdef WITH_IMS_PM
	static str zero={0,0};
#endif
/**
 * Send a third party registration
 * @param r - the register to send for
 * @param expires - expires time
 * @returns true if OK, false if not
 */

int r_send_third_party_reg(r_third_party_registration *r,int expires)
{
        str h={0,0};

        LOG(L_DBG,"DBG:"M_NAME":r_send_third_party_reg: REGISTER to <%.*s>\n",
                r->req_uri.len,r->req_uri.s);

        h.len = event_hdr.len+content_len_hdr.len+max_fwds_hdr.len;
        h.len += expires_s.len + 12 + expires_e.len;

        h.len += contact_s.len + isc_my_uri_sip.len + contact_e.len;

        if (r->pani.len) h.len += p_access_network_info_s.len +
                p_access_network_info_e.len + r->pani.len;

        if (r->cv.len) h.len += p_charging_vector_s.len +
                p_charging_vector_e.len + r->cv.len;

        h.s = pkg_malloc(h.len);
        if (!h.s){
                LOG(L_ERR,"ERR:"M_NAME":r_send_third_party_reg: Error allocating %d bytes\n",h.len);
		h.len = 0;
                return 0;
        }

        h.len = 0;
        STR_APPEND(h,event_hdr);
        
        STR_APPEND(h,content_len_hdr);
        STR_APPEND(h,max_fwds_hdr);

        STR_APPEND(h,expires_s);
        sprintf(h.s+h.len,"%d",expires);
        h.len += strlen(h.s+h.len);
        STR_APPEND(h,expires_e);

        STR_APPEND(h,contact_s);
        STR_APPEND(h,isc_my_uri_sip);
        STR_APPEND(h,contact_e);

        if (r->pani.len) {
                STR_APPEND(h,p_access_network_info_s);
                STR_APPEND(h,r->pani);
                STR_APPEND(h,p_access_network_info_e);
        }

        if (r->cv.len) {
                STR_APPEND(h,p_charging_vector_s);
                STR_APPEND(h,r->cv);
                STR_APPEND(h,p_charging_vector_e);
        }

        if (isc_tmb.t_request(&method, &(r->req_uri), &(r->to), &(r->from), &h, 0, 0,
                 r_third_party_reg_response, &(r->req_uri))<0)
        {
                LOG(L_ERR,"ERR:"M_NAME":r_send_third_party_reg: Error sending in transaction\n");
                goto error;
        }
		#ifdef WITH_IMS_PM
			IMS_PM_LOG(UR_Att3rdPartyReg);
		#endif
        if (h.s) pkg_free(h.s);
        return 1;

error:
        if (h.s) pkg_free(h.s);
        return 0;
}


/**
 * Response callback for third party register
 */

void r_third_party_reg_response(struct cell *t,int type,struct tmcb_params *ps)
{
        str req_uri;
        int expires;
        LOG(L_DBG,"DBG:"M_NAME":r_third_party_reg_response: code %d\n",ps->code);
        if (!ps->rpl) {
                LOG(L_ERR,"INF:"M_NAME":r_third_party_reg_response: No reply\n");
                return;
        }
        
     	#ifdef WITH_IMS_PM
			if (ps->code>=200 && ps->code<300) 
				IMS_PM_LOG01(UR_Succ3rdPartyReg,ps->code);
			else if (ps->code>=300) IMS_PM_LOG01(UR_Fail3rdPartyReg,ps->code);
		#endif
		     
        if (ps->code>=200 && ps->code<300){
                if (ps->rpl)
                        expires = cscf_get_expires_hdr(ps->rpl);
                else
                        return;
                req_uri = *((str*) *(ps->param));
        }else
        if (ps->code==404){
        }else{
                LOG(L_INFO,"INF:"M_NAME":r_third_party_reg_response: code %d\n",ps->code); 
        }
}

/**
 * Frees up space taken by a registration
 */
void free_r_registration(r_third_party_registration *r)
{
        if (r){

                if (r->req_uri.s) shm_free(r->req_uri.s);
		if (r->to.s) shm_free(r->to.s);
        	if (r->from.s) shm_free(r->from.s);
		if (r->pani.s) shm_free(r->pani.s);
		if (r->cv.s) shm_free(r->cv.s);
                shm_free(r);
        }
}

