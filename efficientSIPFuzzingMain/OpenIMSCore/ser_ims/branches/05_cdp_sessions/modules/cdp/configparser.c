/**
 * $Id: configparser.c 2 2006-11-14 22:37:20Z vingarzan $
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
 * CDiameterPeer - Configuration file parser.
 * 
 * The file is kept as dtd. See configdtd.h for the DTD and ConfigExample.xml.
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include "config.h"


#include <libxml/parser.h>
#include <stdio.h>
#include <string.h>

extern int errno;
	
static xmlValidCtxt	cvp;	/**< XML Validation context */

/**
 * Initializes the libxml parser
 * @returns 1 always
 */
static inline int parser_init()
{
	cvp.userData = (void*)stderr;
	cvp.error = (xmlValidityErrorFunc) fprintf;
	cvp.warning = (xmlValidityWarningFunc) fprintf;
	return 1;
}

/**
 * Destroys the parser 
 */
static inline void parser_destroy()
{
	xmlCleanupParser();
}

/**
 * Trim the quotes from a string and duplicate it.
 * @param dest - destination for the untrimmed and duplicated string
 * @param src - source string
 */
static inline void quote_trim_dup(str *dest, char *src)
{
	int i=0;
	dest->s=0;
	dest->len=0;
	if (!src) return;
	dest->len = strlen(src);
	if (src[0]=='\"') {i++;dest->len--;}
	if (src[dest->len-1]=='\"') {dest->len--;}

	dest->s = shm_malloc(dest->len+1);
	if (!dest->s) {
		LOG_NO_MEM("shm",dest->len);
		dest->len=0;
		return;
	}
	memcpy(dest->s,src+i,dest->len);
	dest->s[dest->len]=0;
}

/**
 * Parses a DiameterPeer configuration file.
 * @param filename - path to the file
 * @returns the dp_config* structure containing the parsed configuration  
 */
dp_config* parse_dp_config(char* filename)
{
	FILE *f=0;
	dp_config *x=0;
	xmlDocPtr doc=0;
	xmlNodePtr root=0,child=0;
	xmlChar *xc=0;
	int k;

	parser_init();

	if (!filename){
		LOG(L_ERR,"ERROR:parse_dp_config(): filename parameter is null\n");
		goto error;
	}
	f = fopen(filename,"r");
	if (!f){
		LOG(L_ERR,"ERROR:parse_dp_config(): Error opening <%s> file > %s\n",filename,strerror(errno));
		goto error;
	}
	fclose(f);
	
	
	x = new_dp_config();
	if (!f){
		LOG(L_ERR,"ERROR:parse_dp_config(): Error opening <%s> file > %s\n",filename,strerror(errno));
		goto error;
	}

	doc = xmlParseFile(filename);
	if (!doc){
		LOG(L_ERR,"ERR:parse_dp_config():  This is not a valid XML file <%s>\n",
			filename);
		goto error;
	}

	root = xmlDocGetRootElement(doc);
	if (!root){
		LOG(L_ERR,"ERR:parse_dp_config():  Empty XML <%s>\n",filename);
		goto error;
	}

	k = strlen(root->name);
	if (k>12) k = 12;
	if (strncasecmp(root->name,"DiameterPeer",k)!=0){
		LOG(L_ERR,"ERR:parse_dp_config(): XML Root is not <DiameterPeer>\n");
		goto error;
	}

	xc = xmlGetProp(root,"FQDN");
	quote_trim_dup(&(x->fqdn),xc);
	quote_trim_dup(&(x->identity),xc);

	xc = xmlGetProp(root,"Realm");
	quote_trim_dup(&(x->realm),xc);
	
	xc = xmlGetProp(root,"Vendor_Id");
	x->vendor_id = atoi(xc);

	xc = xmlGetProp(root,"Product_Name");
	quote_trim_dup(&(x->product_name),xc);

	xc = xmlGetProp(root,"AcceptUnknownPeers");
	x->accept_unknown_peers = atoi(xc);
	
	xc = xmlGetProp(root,"DropUnknownOnDisconnect");
	x->drop_unknown_peers = atoi(xc);
	
	xc = xmlGetProp(root,"Tc");
	x->tc = atoi(xc);

	xc = xmlGetProp(root,"Workers");
	x->workers = atoi(xc);

	xc = xmlGetProp(root,"QueueLength");
	x->queue_length = atoi(xc);

	for(child = root->children; child; child = child->next)
		if (child->type == XML_ELEMENT_NODE)
	{
		if (strlen(child->name)==4 && strncasecmp(child->name,"Peer",4)==0){
			//PEER
			x->peers_cnt++;		
		}
		if (strlen(child->name)==8 && strncasecmp(child->name,"Acceptor",8)==0){
			//Acceptor
			x->acceptors_cnt++;		
		}
		if (strlen(child->name)==4 && (strncasecmp(child->name,"Auth",4)==0||
			strncasecmp(child->name,"Acct",4)==0)){
			//Application
			x->applications_cnt++;		
		}		
	}
	x->peers = shm_malloc(x->peers_cnt*sizeof(peer_config));
	if (!x->peers){
		LOG_NO_MEM("shm",x->peers_cnt*sizeof(peer_config));
		goto error;
	}
	memset(x->peers,0,x->peers_cnt*sizeof(peer_config));
	x->peers_cnt=0;
	x->acceptors = shm_malloc(x->acceptors_cnt*sizeof(acceptor_config));
	if (!x->acceptors){
		LOG_NO_MEM("shm",x->acceptors_cnt*sizeof(acceptor_config));
		goto error;
	}
	memset(x->acceptors,0,x->acceptors_cnt*sizeof(acceptor_config));
	x->acceptors_cnt=0;
	x->applications = shm_malloc(x->applications_cnt*sizeof(app_config));
	if (!x->applications){
		LOG_NO_MEM("shm",x->applications_cnt*sizeof(app_config));
		goto error;
	}
	memset(x->applications,0,x->applications_cnt*sizeof(app_config));
	x->applications_cnt=0;

	for(child = root->children; child; child = child->next)
		if (child->type == XML_ELEMENT_NODE)
	{
		if (strlen(child->name)==4 && strncasecmp(child->name,"Peer",4)==0){
			//PEER
			xc = xmlGetProp(child,"FQDN");
			quote_trim_dup(&(x->peers[x->peers_cnt].fqdn),xc);
			xc = xmlGetProp(child,"Realm");
			quote_trim_dup(&(x->peers[x->peers_cnt].realm),xc);			
			xc = xmlGetProp(child,"port");
			x->peers[x->peers_cnt].port = atoi(xc);						
			x->peers_cnt++;		
		}
		if (strlen(child->name)==8 && strncasecmp(child->name,"Acceptor",8)==0){
			//Acceptor
			xc = xmlGetProp(child,"bind");			
			quote_trim_dup(&(x->acceptors[x->acceptors_cnt].bind),xc);			
			xc = xmlGetProp(child,"port");
			x->acceptors[x->acceptors_cnt].port = atoi(xc);						
			x->acceptors_cnt++;		
		}
		if (strlen(child->name)==4 && (strncasecmp(child->name,"Auth",4)==0||
			strncasecmp(child->name,"Acct",4)==0)){
			//Application
			xc = xmlGetProp(child,"id");			
			x->applications[x->applications_cnt].id = atoi(xc);						
			xc = xmlGetProp(child,"vendor");
			x->applications[x->applications_cnt].vendor = atoi(xc);						
			if (child->name[1]=='u'||child->name[1]=='U')
				x->applications[x->applications_cnt].type = DP_AUTHORIZATION;						
			else
				x->applications[x->applications_cnt].type = DP_ACCOUNTING;										
			x->applications_cnt++;		
		}		
	}
	
	if (doc) xmlFreeDoc(doc);	
	parser_destroy();
	return x;
error:
	if (doc) xmlFreeDoc(doc);
	parser_destroy();
	if (x) free_dp_config(x);
	return 0;	
}

