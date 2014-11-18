/**
 * $Id: db.c 2 2006-11-14 22:37:20Z vingarzan $
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
 * Interrogating-CSCF - Database operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "db.h"

#include "../../db/db.h"
#include "../../mem/shm_mem.h"

#include "mod.h"

static db_func_t dbf;						/**< db function bindings*/
extern char * icscf_db_url;					/**< DB URL */
extern char * icscf_db_nds_table;			/**< NDS table in DB */
extern char * icscf_db_scscf_table;			/**< S-CSCF table in db */
extern char * icscf_db_capabilities_table;	/**< S-CSCF capabilities table in db */

static db_con_t *hdl_nds=0;					/**< handle for the NDS table */
static db_con_t *hdl_scscf=0;				/**< handle for the S-CSCF table */
static db_con_t *hdl_capabilities=0;		/**< handle for the S-CSCF capabilities table */



/**
 *  Bind to the database module.
 * @param db_url - URL of the database
 * @returns 0 on success, -1 on error
 */
int icscf_db_bind(char* db_url)
{
	if (bind_dbmod(icscf_db_url, &dbf)) {
		LOG(L_CRIT, "CRIT:"M_NAME":icscf_db_bind: cannot bind to database module! "
		"Did you forget to load a database module ?\n");
		return -1;
	}
	return 0;
}


/**
 *  Init the database connection 
 * @param db_url - URL of the database
 * @param db_table_nds - name of the NDS table
 * @param db_table_scscf - name of the S-CSCF table
 * @param db_table_capabilities - name of the S-CSCF capabilities table
 * @returns 0 on success, -1 on error
 */
int icscf_db_init(char* db_url,
	char* db_table_nds,
	char* db_table_scscf,
	char* db_table_capabilities)
{
	if (dbf.init==0){
		LOG(L_CRIT, "BUG:"M_NAME":icscf_db_init: unbound database module\n");
		return -1;
	}
	/* NDS */
	hdl_nds=dbf.init(db_url);
	if (hdl_nds==0){
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot initialize database "
			"connection\n");
		goto error;
	}	
	if (dbf.use_table(hdl_nds, db_table_nds)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot select table \"%s\"\n",db_table_nds);
		goto error;
	}
	/* S_CSCF */
	hdl_scscf=dbf.init(db_url);
	if (hdl_scscf==0){
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot initialize database "
			"connection\n");
		goto error;
	}	
	if (dbf.use_table(hdl_scscf, db_table_scscf)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot select table \"%s\"\n",db_table_scscf);
		goto error;
	}
	/* Capabilities */
	hdl_capabilities=dbf.init(db_url);
	if (hdl_capabilities==0){
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot initialize database "
			"connection\n");
		goto error;
	}	
	if (dbf.use_table(hdl_capabilities, db_table_capabilities)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot select table \"%s\"\n",db_table_capabilities);
		goto error;
	}

	return 0;

error:
	if (hdl_nds){
		dbf.close(hdl_nds);
		hdl_nds=0;
	}
	if (hdl_scscf){
		dbf.close(hdl_scscf);
		hdl_nds=0;
	}
	if (hdl_capabilities){
		dbf.close(hdl_capabilities);
		hdl_nds=0;
	}
	return -1;
}

/**
 *  Close the database connection.
 */
void icscf_db_close()
{
	if (hdl_nds && dbf.close){
		dbf.close(hdl_nds);
		hdl_nds=0;
	}
}

/**
 * Simply check if the database connection was initialized and connect if not.
 * @param db_hdl - database handle to test
 * @returns 1 if connected, 0 if not
 */
static inline int icscf_db_check_init(db_con_t *db_hdl)
{
	if (db_hdl) return 1;
	return (icscf_db_init( icscf_db_url,
		icscf_db_nds_table,
		icscf_db_scscf_table,
		icscf_db_capabilities_table)==0);		
}

/**
 *  Get the NDS list from the database.
 * @param d - array of string to fill with the db contents
 * @returns 1 on success, 0 on error 
 */
int icscf_db_get_nds(str *d[])
{
	db_key_t   keys_ret[] = {"trusted_domain"};
	db_res_t   * res = 0 ;	
	str s;
	int i;

	if (!icscf_db_check_init(hdl_nds))
		goto error;

	DBG("DBG:"M_NAME":icscf_db_get_nds: fetching list of NDS for I-CSCF \n");

	if (dbf.query(hdl_nds, 0, 0, 0, keys_ret, 0, 1, NULL, & res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: db_query failed\n");
		goto error;
	}

	if (res->n == 0) {
		DBG("DBG:"M_NAME":icscf_db_get_nds: I-CSCF has no NDS trusted domains in db\n");
		*d=shm_malloc(sizeof(str));
		if (*d==NULL){
			LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: failed shm_malloc for 0 domains\n");
			goto error;
		}	
		(*d)[0].s=0;
		(*d)[0].len=0;
	}
	else {
		*d=shm_malloc(sizeof(str)*(res->n+1));
		if (*d==NULL){
			LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: failed shm_malloc for %d domains\n",
				res->n);
			goto error;
		}	
		for(i=0;i<res->n;i++){
			s.s = (char*) res->rows[i].values[0].val.string_val;
			s.len = strlen(s.s);
			(*d)[i].s = shm_malloc(s.len);
			if ((*d)[i].s==NULL) {
				LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: failed shm_malloc for %d bytes\n",
					s.len);
				(*d)[i].len = 0;
			}else{
				(*d)[i].len = s.len;
				memcpy((*d)[i].s,s.s,s.len);
			}
		}
		(*d)[res->n].s=0;
		(*d)[res->n].len=0;
	}

	LOG(L_INFO, "INF:"M_NAME":icscf_db_get_nds: Loaded %d trusted domains\n",
		res->n);

	dbf.free_result( hdl_nds, res);
	return 1;
error:
	if (res)
		dbf.free_result( hdl_nds, res);
	*d=shm_malloc(sizeof(str));
	if (*d==NULL)
		LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: failed shm_malloc for 0 domains\n");
	else {
		(*d)[0].s=0;
		(*d)[0].len=0;
	}
	return 0;
}


/**
 *  Get the S-CSCF names from the database and create the S-CSCF set.
 * @param cap - array of scscf_capabilities to fill with the db contents for the S-CSCF names
 * @returns 1 on success, 0 on error 
 */
int icscf_db_get_scscf(scscf_capabilities *cap[])
{
	db_key_t   keys_ret[] = {"id","s_cscf_uri"};
	db_key_t   key_ord = "id";
	db_res_t   * res = 0 ;	
	int i;

	*cap = 0;
		
	if (!icscf_db_check_init(hdl_scscf))
		goto error;

	DBG("DBG:"M_NAME":icscf_db_get_scscf: fetching S-CSCFs \n");

	if (dbf.query(hdl_scscf, 0, 0, 0, keys_ret, 0, 2, key_ord, & res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_scscf: db_query failed\n");
		goto error;
	}

	if (res->n == 0) {
		LOG(L_ERR,"ERR:"M_NAME":icscf_db_get_scscf:  no S-CSCFs found\n");
		goto error;
	}
	else {
		*cap = shm_malloc(sizeof(scscf_capabilities)*res->n);
		if (!(*cap)) {
			LOG(L_ERR,"ERR:"M_NAME":icscf_db_get_scscf: Error allocating %d bytes\n",
				sizeof(scscf_capabilities)*res->n);
			goto error;
		}
		memset((*cap),0,sizeof(scscf_capabilities)*res->n);
		for(i=0;i<res->n;i++){
			(*cap)[i].id_s_cscf = res->rows[i].values[0].val.int_val;
			(*cap)[i].scscf_name.len = strlen(res->rows[i].values[1].val.string_val);
			(*cap)[i].scscf_name.s = shm_malloc((*cap)[i].scscf_name.len);
			if (!(*cap)[i].scscf_name.s){
				LOG(L_ERR,"ERR:"M_NAME":icscf_db_get_scscf: Error allocating %d bytes\n",
					(*cap)[i].scscf_name.len);
				(*cap)[i].scscf_name.len=0;
				goto error;
			}
			memcpy((*cap)[i].scscf_name.s,res->rows[i].values[1].val.string_val,
				(*cap)[i].scscf_name.len);
		}
	}

	dbf.free_result( hdl_scscf, res);
	
	// return the size of scscf set  
	return i;
	
error:
	if (res)
		dbf.free_result( hdl_scscf, res);
	return 0;
}

/**
 *  Get the S-CSCF capabilities from the database and fill the S-CSCF set.
 * @param cap - array of scscf_capabilities to fill with capabilities
 * @returns 1 on success, 0 on error 
 */
int icscf_db_get_capabilities(scscf_capabilities *cap[],int cap_cnt)
{
//	db_key_t   keys_cmp[] = {"icscf"};
	db_key_t   keys_ret[] = {"id_s_cscf","capability"};
	db_key_t   key_ord = "id_s_cscf";
	db_res_t   * res = 0 ;	
	int i,j;
	int ccnt=0;
	int cnt;


	if (!icscf_db_check_init(hdl_capabilities))
		goto error;

	DBG("DBG:"M_NAME":icscf_db_get_capabilities: fetching list of Capabilities for I-CSCF\n");


	if (dbf.query(hdl_capabilities, 0, 0, 0, keys_ret, 0, 2, key_ord, & res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_capabilities: db_query failed\n");
		goto error;
	}

	if (res->n == 0) {
		DBG("DBG:"M_NAME":icscf_db_get_capabilities: No Capabilites found... not critical...\n");
		return 1;
	}
	else {
		for(i=0;i<cap_cnt;i++){
			cnt = 0;
			for(j=0;j<res->n;j++)
				if (res->rows[j].values[0].val.int_val == (*cap)[i].id_s_cscf)
					cnt++;
			(*cap)[i].capabilities = shm_malloc(sizeof(int)*cnt);
			if (!(*cap)[i].capabilities) {
				LOG(L_ERR,"ERR:"M_NAME":icscf_db_get_capabilities: Error allocating %d bytes\n",
					sizeof(int)*cnt);
				(*cap)[i].cnt=0;
					goto error;
			}			
			cnt=0;
			for(j=0;j<res->n;j++)
				if (res->rows[j].values[0].val.int_val == (*cap)[i].id_s_cscf) {
					(*cap)[i].capabilities[cnt++]=res->rows[j].values[1].val.int_val;
					ccnt++;
				}
			(*cap)[i].cnt=cnt;					
		}
			
	} 
	LOG(L_INFO, "INF:"M_NAME":icscf_db_get_capabilities: Loaded %d capabilities for %d S-CSCFs (%d invalid entries in db)\n",
		ccnt,cap_cnt,res->n-ccnt);
	dbf.free_result( hdl_capabilities, res);
	return 1;
	
error:
	if (res)
		dbf.free_result( hdl_capabilities, res);
	return 0;
}
