/*
 * $Id: third_party_reg.c 502 2007-11-30 12:32:57Z placido $
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
	r_third_party_registration r;
	int expires=0;
	str req_uri ={0,0};
	str to ={0,0};
	str pvni ={0,0};
	str pani ={0,0};
	str cv ={0,0};

	struct hdr_field *hdr;

	LOG(L_INFO,"INFO:"M_NAME":isc_third_party_reg: Enter\n");

	/* Set Request Uri to IFC matching server name */
	req_uri.len = m->server_name.len;
	req_uri.s = m->server_name.s;

	/* Get To header*/
	to = cscf_get_public_identity(msg);


	/*TODO - check if the min/max expires is in the acceptable limits
	 * this does not work correctly if the user has multiple contacts
	 * and register/deregisters them individually!!!
	 */
	expires = cscf_get_max_expires(msg);
	
	/* Get P-Visited-Network-Id header */
	pvni = cscf_get_visited_network_id(msg, &hdr);
	/* Get P-Access-Network-Info header */
	pani = cscf_get_access_network_info(msg, &hdr);
	
	/* Get P-Charging-Vector header */
	/* Just forward the charging header received from P-CSCF */
	/* Todo: implement also according to TS 24.229, chap 5.4.1.7 */
	cv =   cscf_get_charging_vector(msg, &hdr);

	if (req_uri.s){
		
        memset(&r,0,sizeof(r_third_party_registration));

        r.req_uri = req_uri;
        r.to = to;
        r.from = isc_my_uri_sip;
        r.pvni = pvni;
        r.pani = pani;
		r.cv = cv;
        r.service_info = m->service_info;
         
		if (expires<=0) r_send_third_party_reg(&r,0);
		else r_send_third_party_reg(&r,expires+isc_expires_grace);
		return ISC_RETURN_TRUE;
	}else{
		return ISC_RETURN_FALSE;
	}	
}




static str method={"REGISTER",8};
static str event_hdr={"Event: registration\r\n",21};
static str max_fwds_hdr={"Max-Forwards: 10\r\n",18};
static str expires_s={"Expires: ",9};
static str expires_e={"\r\n",2};
static str contact_s={"Contact: <",10};
static str contact_e={">\r\n",3};

static str p_visited_network_id_s={"P-Visited-Network-ID: ",22};
static str p_visited_network_id_e={"\r\n",2};

static str p_access_network_info_s={"P-Access-Network-Info: ",23};
static str p_access_network_info_e={"\r\n",2};

static str p_charging_vector_s={"P-Charging-Vector: ",19};
static str p_charging_vector_e={"\r\n",2};
static str body_s={"<ims-3gpp version=\"1\"><service-info>",36};
static str body_e={"</service-info></ims-3gpp>",26};


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
        str b={0,0};

        LOG(L_DBG,"DBG:"M_NAME":r_send_third_party_reg: REGISTER to <%.*s>\n",
                r->req_uri.len,r->req_uri.s);

        h.len = event_hdr.len+max_fwds_hdr.len;
        h.len += expires_s.len + 12 + expires_e.len;

        h.len += contact_s.len + isc_my_uri_sip.len + contact_e.len;

        if (r->pvni.len) h.len += p_visited_network_id_s.len +
                p_visited_network_id_e.len + r->pvni.len;

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
        
        STR_APPEND(h,max_fwds_hdr);

        STR_APPEND(h,expires_s);
        sprintf(h.s+h.len,"%d",expires);
        h.len += strlen(h.s+h.len);
        STR_APPEND(h,expires_e);

        STR_APPEND(h,contact_s);
        STR_APPEND(h,isc_my_uri_sip);
        STR_APPEND(h,contact_e);

        if (r->pvni.len) {
                STR_APPEND(h,p_visited_network_id_s);
                STR_APPEND(h,r->pvni);
                STR_APPEND(h,p_visited_network_id_e);
        }

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
        LOG(L_CRIT,"SRV INFO:<%.*s>\n",r->service_info.len, r->service_info.s);
		if (r->service_info.len){
			b.len = body_s.len+r->service_info.len+body_e.len;
	        b.s = pkg_malloc(b.len);
	        if (!b.s){
				LOG(L_ERR,"ERR:"M_NAME":r_send_third_party_reg: Error allocating %d bytes\n",b.len);
				b.len = 0;
	            return 0;
	        }
	
	        b.len = 0;
	        STR_APPEND(b,body_s);
	        STR_APPEND(b,r->service_info);	        
	        STR_APPEND(b,body_e);	        
		}
		

        if (isc_tmb.t_request(&method, &(r->req_uri), &(r->to), &(r->from), &h, &b, 0,
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

