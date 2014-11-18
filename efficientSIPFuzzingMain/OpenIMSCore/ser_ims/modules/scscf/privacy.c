/**
 * $Id: $
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
 * S-CSCF Module - RFC 3323/3325 Privacy Functions 
 *
 * Copyright (C) 2007 PT Inovacao
 *
 */

#include "../tm/tm_load.h"
#include "mod.h"
#include "sip.h"
#include "privacy.h"

extern struct tm_binds tmb;

static str asserted_identity={"P-Asserted-Identity",19};
static str privacy={"Privacy",7};

/**
 * Applies Privacy for P-Asserted-Identity.
 * @param msg - the SIP message
 * @returns #CSCF_RETURN_TRUE if privacy applied or #CSCF_RETURN_FALSE if not
 */
int apply_privacy_id(struct sip_msg* msg){
	struct hdr_field *hdr;

	LOG(L_DBG, "DBG:"M_NAME": apply_privacy_id\n");
	hdr = cscf_get_next_header(msg,asserted_identity,NULL);
	if (!hdr) return CSCF_RETURN_FALSE;
	do {
		cscf_del_header(msg,hdr);
		hdr=cscf_get_next_header(msg,asserted_identity,hdr);
	} while (hdr);

	return CSCF_RETURN_TRUE;
}

/**
 * Privacy reply tm callback. Applies Privacy if needed.
 * Currently only supports "id" type.
 * Registered by S_privacy_hook.
 */
void privacy_reply_cb(struct cell *t, int type, struct tmcb_params *ps) {
	S_apply_privacy(ps->rpl, 0, 0);
}

/**
 * Applies Privacy if needed.
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if privacy applied or #CSCF_RETURN_FALSE if not
 */
int S_apply_privacy(struct sip_msg* msg, char* str1, char* str2) {
	char *c;
	int ret = CSCF_RETURN_FALSE;
	
	str privacy_content = cscf_get_headers_content(msg,privacy);

	if (!privacy_content.len){ 
		ret = CSCF_RETURN_FALSE;
		goto done;
	}
	
	LOG(L_DBG, "DBG:"M_NAME":S_apply_privacy\n");
	c = strtok(privacy_content.s," \t\r\n;,");
	while (c){
		if (strlen(c)==2 && strncasecmp(c,"id",2)==0){
			ret = apply_privacy_id(msg);
			break;
		}
		c = strtok(0," \t\r\n;,");
	}
	
done:	
	if (privacy_content.s) pkg_free(privacy_content.s);
	return ret;
}

/**
 * Registers tm callback to apply privacy to replies.
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE
 */
int S_privacy_hook(struct sip_msg* msg, char* str1, char* str2) {
	unsigned int a,b;
	
	cscf_get_transaction(msg,&a,&b);
	tmb.register_tmcb(msg,0,TMCB_RESPONSE_IN,privacy_reply_cb,0);
	
	return CSCF_RETURN_TRUE;
}

