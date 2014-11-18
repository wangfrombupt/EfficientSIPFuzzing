/**
 * $Id: checker.c 571 2008-07-02 12:00:17Z vingarzan $
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
 * IMS Service Control - Initial Filter Criteria Checker
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#include "../scscf/ifc_datastruct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlschemas.h>
#include <libxml/parser.h>
#include <regex.h>
#include "../../str.h"

#include "checker.h"
#include "sip.h"
#include "mod.h"

#include "../../dprint.h"
#include "../../parser/msg_parser.h"

#include "../../mem/mem.h"

#include "../scscf/scscf_load.h"
#include "../tm/tm_load.h"

/** debug flag for IFC matching */
#define IFC_DEBUG 1

#define TRUE 1
#define FALSE 0

extern struct tm_binds isc_tmb;            /**< Structure with pointers to tm funcs 		*/
extern struct scscf_binds isc_scscfb;      /**< Structure with pointers to S-CSCF funcs 	*/

/**
 *	Check if a Service Point Trigger for Header matches the SDP body
 *	@param spt - the service point trigger
 *	@param headers - the headers of the message
 *	@returns - 1 on success, 0 on failure
 */
static int isc_check_headers(ims_spt *spt, struct hdr_field *headers)
{
	struct hdr_field *i;
	char c,ch;
	char buf[256];
	regex_t header_comp,content_comp;
	i = headers;
	/* compile the regex for header name */
	memcpy(buf,spt->sip_header.header.s,spt->sip_header.header.len);
	buf[spt->sip_header.header.len]=0;
	regcomp(&(header_comp),buf,REG_ICASE|REG_EXTENDED);
	
	/* compile the regex for content */
	memcpy(buf,spt->sip_header.content.s,spt->sip_header.content.len);
	buf[spt->sip_header.content.len]=0;
	regcomp(&(content_comp),buf,REG_ICASE|REG_EXTENDED);
	
	DBG("DEBUG:"M_NAME":isc_check_headers: Looking for Header[%.*s(%d)] %.*s \n",
		spt->sip_header.header.len,spt->sip_header.header.s,spt->sip_header.type,
		spt->sip_header.content.len,spt->sip_header.content.s);
	while(i!=NULL){
		ch = i->name.s[i->name.len];
		i->name.s[i->name.len]=0;
		
		if ((spt->sip_header.type>0&&spt->sip_header.type==i->type)|| //matches known type
			(regexec(&(header_comp),i->name.s,0,NULL,0)==0)//or matches the name
		   )		
		{
			
			i->name.s[i->name.len]=ch;
			DBG("DEBUG:"M_NAME":isc_check_headers: Found Header[%.*s(%d)] %.*s \n",
					i->name.len,i->name.s,i->type,i->body.len,i->body.s);
			//if the header should be absent but found it
		
			if (spt->sip_header.content.s==NULL)
				if (spt->condition_negated) {
						regfree(&(header_comp));
						regfree(&(content_comp));	
						return FALSE;
					}
													
			//check regex
			c = i->body.s[i->body.len];
			i->body.s[i->body.len]=0;
		
			if (regexec(&(content_comp),i->body.s,0,NULL,0)==0)//regex match
			{
		
				regfree(&(header_comp));
				regfree(&(content_comp));
				i->body.s[i->body.len]=c;
				return TRUE;
			}
	
			i->body.s[i->body.len]=c;
		}
		else i->name.s[i->name.len]=ch;
		i = i->next;
	}
	
	regfree(&(header_comp));
	regfree(&(content_comp));
	return FALSE;
}

static str sdp={"application/sdp",15};
/**
 *	Check if a Service Point Trigger for Session Description matches the SDP body
 *	@param spt - the service point trigger
 *	@param msg - the message
 *	@returns - 1 on success, 0 on failure
 */
static int isc_check_session_desc(ims_spt *spt, struct sip_msg *msg)
{
	int len;
	char *body,c;
	char *x;
	regex_t comp;

	if (msg->content_type==NULL) return FALSE;
	if (str2icmp(msg->content_type->body,sdp)!=0) return FALSE;
	DBG("DEBUG:"M_NAME":ifc_check_session_desc:      Found Content-Type == appliction/sdp\n");
	//check for sdp line
	body = get_body(msg);
	if (body==0) return FALSE;
	if  (msg->content_length->parsed==NULL) {
		parse_content_length(msg->content_length->body.s,
			msg->content_length->body.s+msg->content_length->body.len,&len);
		msg->content_length->parsed=(void*)(long)len;
	} else
		len = (long)msg->content_length->parsed;
	c = body[len];
	body[len]=0;
	x = pkg_malloc(spt->session_desc.line.len+2+spt->session_desc.content.len);
	sprintf(x,"%.*s=%.*s",
		spt->session_desc.line.len,spt->session_desc.line.s,
		spt->session_desc.content.len,spt->session_desc.content.s);
	/* compile the whole  regexp */	
	regcomp(&(comp),x,REG_ICASE|REG_EXTENDED);	
	if (regexec(&(comp),body,0,NULL,0)==0)//regex match
	{
		body[len]=c;
		DBG("DEBUG:"M_NAME":ifc_check_session_desc:      Found Session Desc. > %s\n",body);
		pkg_free(x);
		return TRUE;
	}
	body[len]=c;
	pkg_free(x);
	return FALSE;
}


/**
 *	Check if a Service Point Trigger matches a message 
 *	@param spt - the service point trigger
 *	@param msg - the message
 *	@param direction - if filter criteria is for originating/terminating/terminating_unregistered
 *	@param registration_type - if the message is initial/re/de registration
 *	@returns - 1 on success, 0 on failure
 */
static int isc_check_spt(ims_spt *spt,struct sip_msg *msg,char direction,char registration_type)
{
	int r=FALSE;
	switch(spt->type){
		case IFC_REQUEST_URI:
			DBG("DEBUG:"M_NAME":ifc_check_spt:             SPT type %d -> RequestURI == %.*s ?\n",spt->type,
				spt->request_uri.len,spt->request_uri.s);
			DBG("DEBUG:"M_NAME":ifc_check_spt:               Found Request URI %.*s \n",msg->first_line.u.request.uri.len,
					msg->first_line.u.request.uri.s);
			r=(str2icmp(spt->request_uri,msg->first_line.u.request.uri)==0);
			break;
		case IFC_METHOD:
			DBG("DEBUG:"M_NAME":ifc_check_spt:             SPT type %d -> Method == %.*s ?\n",spt->type,
				spt->method.len,spt->method.s);
			DBG("DEBUG:"M_NAME":ifc_check_spt:               Found method %.*s \n",msg->first_line.u.request.method.len,
				msg->first_line.u.request.method.s);
			r=(str2icmp(spt->method,msg->first_line.u.request.method)==0);
			if (r &&
				spt->method.len==8 && 
				strncasecmp(spt->method.s,"REGISTER",8)==0 &&				
				!(spt->registration_type==0 ||(registration_type & spt->registration_type))
				)	r = 0;							
			break;
		case IFC_SIP_HEADER:
			DBG("DEBUG:"M_NAME":ifc_check_spt:             SPT type %d -> Header[%.*s]  %%= %.*s ?\n",spt->type,
				spt->sip_header.header.len,spt->sip_header.header.s,
				spt->sip_header.content.len,spt->sip_header.content.s);
			if (parse_headers(msg,HDR_EOH_F,0)!=0) {
				LOG(L_ERR,"ERROR:ifc_checker: can't parse all headers \n");
				r=FALSE;
			} else r=isc_check_headers(spt,msg->headers);
			break;
		case IFC_SESSION_CASE:
			DBG("DEBUG:"M_NAME":ifc_check_spt:             SPT type %d -> Session Case  == %d ?\n",spt->type,
				spt->session_case);
			DBG("DEBUG:"M_NAME":ifc_check_spt:               Found session_case %d \n",direction);
			r = (direction==spt->session_case);
			break;
		case IFC_SESSION_DESC:
			DBG("DEBUG:"M_NAME":ifc_check_spt:             SPT type %d -> Session Desc.[%.*s]  %%= %.*s ?\n",spt->type,
				spt->session_desc.line.len,spt->session_desc.line.s,
				spt->session_desc.content.len,spt->session_desc.content.s);

			if (parse_headers(msg,HDR_CONTENTTYPE_F|HDR_CONTENTLENGTH_F,0)!=0) {
				LOG(L_ERR,"ERROR:ifc_checker: can't parse all headers \n");
				r=FALSE;
			}
			r=isc_check_session_desc(spt,msg);
			break;
		default:
			LOG(L_ERR,"ERROR:ifc_checker: unknown spt type %d \n",spt->type);
			return FALSE;
	}
	if (spt->condition_negated)	return !r;
	else return r;
}

/**
 *	Check if an entire filter criteria matches a message 
 *	@param fc - the filter criteria
 *	@param msg - the message
 *	@param direction - if filter criteria is for originating/terminating/terminating_unregistered
 *	@param registration_type - if the message is initial/re/de registration
 *	@returns - 1 on success, 0 on failure
 */
static int isc_check_filter_criteria(ims_filter_criteria *fc,struct sip_msg *msg,char direction,
		char registration_type)
{
	int i,partial,total,inside,outside,group;
	ims_trigger_point *t;
	t = fc->trigger_point;
	
	/* If the trigger is missing -> always fwd */	
	if (t==NULL) return TRUE;
	/* This shouldn't happen */
	if (msg==NULL) return FALSE;
	
	if (t->condition_type_cnf==IFC_CNF) {//CNF
		inside=TRUE;outside=FALSE;partial=FALSE;total=TRUE;
	} else {//DNF
		inside=FALSE;outside=TRUE;partial=TRUE;total=FALSE;
	}
	//printf("checking trigger \n");
	DBG("DEBUG:"M_NAME":ifc_checker_trigger: Starting expression check: \n");
	group = t->spt[0].group;
	for(i=0;i<t->spt_cnt;i++) {
		//printf("checking spt %d/%d \n",i,t->spt_cnt);
		if (group!=t->spt[i].group) {//jump to other group
			total = t->condition_type_cnf==IFC_CNF?
					total && partial:
					total || partial;
			if (total==outside) {
				DBG("DEBUG:"M_NAME":ifc_checker_trigger: Total compromised, aborting...\n");
			    return outside; // will never match from now on, so get out
			}

			group = t->spt[i].group;
			partial=isc_check_spt(t->spt+i,msg,direction,registration_type);
			DBG("DEBUG:"M_NAME":ifc_checker_trigger:  - group %d => %d. \n",group,partial);
		} else {//in same group
			partial = t->condition_type_cnf==IFC_CNF?
					partial || isc_check_spt(t->spt+i,msg,direction,registration_type):
					partial && isc_check_spt(t->spt+i,msg,direction,registration_type);
		}

		if (partial==inside) {				// can't change partial from now, so next group
			DBG("DEBUG:"M_NAME":ifc_checker_trigger:       - group compromised, skipping to next group\n");
			while(i+1<t->spt_cnt&&t->spt[i+1].group==group)
			 i++;
			continue;
		}
	}
	total = t->condition_type_cnf==IFC_CNF?
			total && partial:
			total || partial;
	DBG("DEBUG:"M_NAME":ifc_checker_trigger: Check finished => %d\n",total);
	return total;
}


/**
 * Create a new matching instance
 * @param fc - filter criteria that match
 * @param index - index of the filter that matches
 * @returns the new isc_match* structure or NULL on error 
 */
static inline isc_match* isc_new_match(ims_filter_criteria *fc,int index)
{
	isc_match *r=0;
	
	r = pkg_malloc(sizeof(isc_match));
	if (!r){
		LOG(L_ERR,"ERR:"M_NAME":isc_new_match(): error allocating %d bytes\n",sizeof(isc_match));
		return 0;
	}
	memset(r,0,sizeof(isc_match));
	if (fc->application_server.server_name.len) {
		r->server_name.s = pkg_malloc(fc->application_server.server_name.len);
		if (!r->server_name.s){
			LOG(L_ERR,"ERR:"M_NAME":isc_new_match(): error allocating %d bytes\n",fc->application_server.server_name.len);
			return 0;
		}
		r->server_name.len = fc->application_server.server_name.len;
		memcpy(r->server_name.s,fc->application_server.server_name.s,fc->application_server.server_name.len);
	}	
	r->default_handling = fc->application_server.default_handling;
	if (fc->application_server.service_info.len) {
		r->service_info.s = pkg_malloc(fc->application_server.service_info.len);
		if (!r->service_info.s){
			LOG(L_ERR,"ERR:"M_NAME":isc_new_match(): error allocating %d bytes\n",fc->application_server.service_info.len);
			return 0;
		}
		r->service_info.len = fc->application_server.service_info.len;
		memcpy(r->service_info.s,fc->application_server.service_info.s,fc->application_server.service_info.len);
	}
	r->index = index;
	return r;
}


/**
 *	Free up all memory taken by a isc_match.
 * @param m - match to deallocate
 */
void isc_free_match(isc_match *m)
{
	if (m) {
		if (m->server_name.s) pkg_free(m->server_name.s);
		if (m->service_info.s) pkg_free(m->service_info.s);			
		pkg_free(m);
	}
	DBG("DEBUG:"M_NAME":isc_match_free: match position freed\n");
}



/**
 * Find the next match and fill up the ifc_match structure with the position of the match
 * @param uri - URI of the user for which to apply the IFC
 * @param direction - direction of the session
 * @param skip - how many IFCs to skip because already matched
 * @param msg - the SIP initial request to check on 
 * @return - TRUE if found, FALSE if none found, end of search space 
 */
isc_match* isc_checker_find(str uri,char direction,int skip,struct sip_msg *msg,int registered)
{
	int expires;
	char registration_type;
	str domain={0,0};
	int i,j,k,cnt,si,sj,next;
	r_public *p;
	ims_service_profile	*sp;
	ims_filter_criteria *fc;
	isc_match *r;
	
	
	if (skip==0) LOG(L_INFO,"INFO:"M_NAME":isc_checker_find: starting search\n");
	else LOG(L_INFO,"INFO:"M_NAME":isc_checker_find: resuming search from %d\n",skip);

	expires = isc_get_expires(msg);
	if (!registered) registration_type = IFC_INITIAL_REGISTRATION;
	else if (expires>0) registration_type = IFC_RE_REGISTRATION;
		 else registration_type = IFC_DE_REGISTRATION;
	
	/* get the user profile from the scscf module */	
	i=0;

	while(i<uri.len && uri.s[i]!='@') 
		i++;
	domain.s = uri.s+i+1;
	domain.len = uri.len - i - 1;

	p = isc_scscfb.get_r_public(uri);

	if (!p) return 0;
	LOG(L_DBG,"isc_checker_find(): got a r_public for the user %.*s\n",uri.len,uri.s);
	if (!p->s) {
		LOG(L_DBG,"isc_checker_find() : got an user without a subscription\n");
		isc_scscfb.r_unlock(p->hash);
		return 0;
	}
	
	/* find the starting fc as the skip-th one*/			
	cnt=0;
	si=0;sj=0;
	while(si<p->s->service_profiles_cnt){
		next = cnt + p->s->service_profiles[si].filter_criteria_cnt;
		if (cnt<=skip && skip<next){
			sj = skip-cnt;
			cnt += sj;
			break;
		}
		cnt = next;
		si++;
	}
	
	/* iterate through the rest and check for matches */
	i = si;
	while(i<p->s->service_profiles_cnt){
		sp = p->s->service_profiles +i;
		k=0;
		for(j=0;j<sp->public_identities_cnt;j++)
		{
			
			if (p->s->wpsi)
			{
					// here i should regexec again!
					// to check this , but anyway if i already got p
					// from the get_r_public , that is already checked...
					// or not if there is no wildcardPSI but ... then ...
					//isc_check_wpsi_match();
					k = 1;
					break;	
							
			} else {
			if (sp->public_identities[j].public_identity.len == uri.len &&
				strncasecmp(sp->public_identities[j].public_identity.s,uri.s,uri.len)==0){
					k = 1;
					break;
				}
			}
		}

		if (!k){/* this sp is not for this id */
			
			cnt += sp->filter_criteria_cnt;
		}else{
			
			for(j=sj;j<sp->filter_criteria_cnt;j++){
				fc = sp->filter_criteria+j;
				if (fc->profile_part_indicator){				
					if (( (registered==IMS_USER_REGISTERED) && (*fc->profile_part_indicator))||
						( (registered==IMS_USER_UNREGISTERED) && !(*fc->profile_part_indicator))) {
						DBG("DEBUG:"M_NAME":isc_checker_find: this one is not good... ppindicator wrong \n");											
						cnt++;
						continue;
					}
				}
				
				if (isc_check_filter_criteria(fc,msg,direction,registration_type)){
					LOG(L_INFO,"INFO:"M_NAME":isc_checker_find: MATCH -> %.*s (%.*s) handling %d \n",
						fc->application_server.server_name.len,
						fc->application_server.server_name.s,
						fc->application_server.service_info.len,
						fc->application_server.service_info.s,
						fc->application_server.default_handling );		
					r = isc_new_match(fc,cnt);	
					isc_scscfb.r_unlock(p->hash);

					return r;	
				}else{
					cnt++;
					continue;
				}
			}	
		}	
		i++;
		sj = 0;
	}
	isc_scscfb.r_unlock(p->hash);

	return 0;
}


