/*
 * $Id: p_persistency.c 236 2007-04-18 12:53:40Z vingarzan $
 *
 * Copyright (C) 2004-2007 FhG Fokus
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
 * P-CSCF persistency operations
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */

#include "bin_db_pcscf.h"

extern persistency_mode_t pcscf_persistency_mode;/**< the type of persistency					*/



extern int r_hash_size;							/**< Size of P-CSCF registrar hash table		*/
extern r_hash_slot *registrar;					/**< The P-CSCF registrar 						*/

extern int p_dialogs_hash_size;					/**< size of the dialog hash table 					*/
extern p_dialog_hash_slot *p_dialogs;			/**< the hash table									*/


extern int subscriptions_hash_size;				/**< Size of P-CSCF subscriptions hash table		*/
extern r_subscription_hash_slot *subscriptions;	/**< The P-CSCF subscriptions 						*/


extern db_con_t* pcscf_db; /**< Database connection handle */
extern db_func_t pcscf_dbf;	/**< Structure with pointers to db functions */
extern int* registrar_snapshot_version;
extern int* registrar_step_version;
extern int* dialogs_snapshot_version;
extern int* dialogs_step_version;
extern int* subs_snapshot_version;
extern int* subs_step_version;
extern gen_lock_t* db_lock;
extern char* pcscf_name;


	/* Dump related functions*/
	


/**
 * Dumps P-CSCF data to DB
 * @param x - binary data to dump. NULL if pcscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each hashtable element is serialized and dumped to DB separately.
 * @param dt - dump registrar, dialogs or susbcriptions
 * @returns 1 on success or 0 on failure
 */
int bin_dump_to_db(bin_data *x, data_type_t dt){

	int snapshot_version;
	int step_version;
	
	switch(dt){
		case P_REGISTRAR:
			snapshot_version=*registrar_snapshot_version;
			step_version=*registrar_step_version;
			return bin_dump_registrar_to_table(x, snapshot_version, step_version);
		case P_DIALOGS:
			snapshot_version=*dialogs_snapshot_version;
			step_version=*dialogs_step_version;
			return bin_dump_dialogs_to_table(x, snapshot_version, step_version);
		case P_SUBSCRIPTIONS:
			snapshot_version=*subs_snapshot_version;
			step_version=*subs_step_version;
			return bin_dump_subs_to_table(x, snapshot_version, step_version);
		default:
			LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_db: No such information to dump %d\n", dt);
			return 0;
	}
}

/**
 * Dumps P-CSCF registrar to DB
 * @param x - binary data to dump. NULL if pcscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each hashtable element is serialized and dumped to DB separately.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_dump_registrar_to_table(bin_data *x, int snapshot_version, int step_version){
	
	if(x){//whole hashtable serialized to x
		return bin_bulk_dump_to_table(P_REGISTRAR, snapshot_version, step_version, x);
	}
	else{//serialize and dump each hashtable element separately
		return bin_cache_dump_registrar_to_table(snapshot_version, step_version);
	}
}

/**
 * Dumps P-CSCF dialogs to DB
 * @param x - binary data to dump. NULL if pcscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each hashtable element is serialized and dumped to DB separately.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_dump_dialogs_to_table(bin_data *x, int snapshot_version, int step_version){
	
	if(x){//whole hashtable serialized to x
		return bin_bulk_dump_to_table(P_DIALOGS, snapshot_version, step_version, x);
	}
	else{//serialize and dump each hashtable element separately
		return bin_cache_dump_dialogs_to_table(snapshot_version, step_version);
	}
}

/**
 * Dumps P-CSCF subscriptions to DB
 * @param x - binary data to dump. NULL if pcscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each hashtable element is serialized and dumped to DB separately.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_dump_subs_to_table(bin_data *x, int snapshot_version, int step_version){
	
	if(x){//whole hashtable serialized to x
		return bin_bulk_dump_to_table(P_SUBSCRIPTIONS, snapshot_version, step_version, x);
	}
	else{//serialize and dump each hashtable element separately
		return bin_cache_dump_subs_to_table(snapshot_version, step_version);
	}
}

/**
 * Dumps P-CSCF data to the snapshot table
 * @param dt - dump registrar, dialogs or susbcriptions
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @param x - binary data to dump (the whole hashtable).
 * @returns 1 on success or 0 on failure
 */
int bin_bulk_dump_to_table(data_type_t dt, int snapshot_version, int step_version, bin_data *x){
	db_key_t keys[5];
	db_val_t vals[5];
	int len;

	/* id auto incremented */
	keys[0] = "node_id";
	keys[1] = "data_type";
	keys[2] = "snapshot_version";
	keys[3] = "step_version";
	/* record_id_1/2/3/4=NULL */
	keys[4] = "data";
	
	vals[0].type = DB_STR;
	vals[0].nul = 0;
	vals[0].val.str_val.s=pcscf_name;
	len = strlen(pcscf_name);
	vals[0].val.str_val.len=MIN(len, 64);

	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val=dt;

	vals[2].type = DB_INT;
	vals[2].nul = 0;
	vals[2].val.int_val=snapshot_version;
	
	vals[3].type = DB_INT;
	vals[3].nul = 0;
	vals[3].val.int_val=step_version;

	str d = {x->s, x->len};
	vals[4].type = DB_BLOB;
	vals[4].nul = 0;
	vals[4].val.blob_val = d;

	//lock
	lock_get(db_lock);

	if (pcscf_dbf.use_table(pcscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_dump_to_table(): Error in use_table\n");
		lock_release(db_lock);//unlock
		return 0;
	}

	if (pcscf_dbf.insert(pcscf_db, keys, vals, 5) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_dump_to_table(): Error while inserting on snapshot table\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	
	//delete older snapshots
	if (delete_older_snapshots("snapshot", pcscf_name, dt, snapshot_version)!=1){
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_dump_to_table(): Error while deleting older snapshots from snapshot table\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	
	//unlock
	lock_release(db_lock);
	
	return 1;
}

/**
 * Dumps P-CSCF registrar to the snapshot table.
 * Each contact is serialized and dumped to one row of the table.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_cache_dump_registrar_to_table(int snapshot_version, int step_version){
	bin_data x;
	r_contact *contact;
	int i;
	int len;
	char aux1[10];
	char aux2[10];
	
	//lock
	lock_get(db_lock);

	if (pcscf_dbf.use_table(pcscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_registrar_to_table(): Error in use_table\n");
		goto error;
	}
	
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
		contact = registrar[i].head;
		while(contact){
			if (!bin_alloc(&x,128)){
				r_unlock(i);
				goto error;
			}
			if (!bin_encode_r_contact(&x,contact)){
				r_unlock(i);
				goto error;
			}
			
			db_key_t keys[8];
			db_val_t vals[8];
			
			/* id auto incremented */
			keys[0] = "node_id";
			keys[1] = "data_type";
			keys[2] = "snapshot_version";
			keys[3] = "step_version";
			keys[4] = "record_id_1"; /* host */
			keys[5] = "record_id_2"; /* port */
			keys[6] = "record_id_3"; /* transport */
			/* record_id_4=NULL */
			keys[7] = "data";
			
			vals[0].type = DB_STR;
			vals[0].nul = 0;
			vals[0].val.str_val.s=pcscf_name;
			len = strlen(pcscf_name);
			vals[0].val.str_val.len=MIN(len, 64);

			vals[1].type = DB_INT;
			vals[1].nul = 0;
			vals[1].val.int_val=P_REGISTRAR;

			vals[2].type = DB_INT;
			vals[2].nul = 0;
			vals[2].val.int_val=snapshot_version;
	
			vals[3].type = DB_INT;
			vals[3].nul = 0;
			vals[3].val.int_val=step_version;
	
			vals[4].type = DB_STR;
			vals[4].nul = 0;
			vals[4].val.str_val.s=contact->host.s;
			vals[4].val.str_val.len=MIN(contact->host.len, 256);
			
			vals[5].type = DB_STR;
			vals[5].nul = 0;
			sprintf(aux1, "%d", contact->port);
			vals[5].val.str_val.s=aux1;
			vals[5].val.str_val.len=strlen(aux1);
	
			vals[6].type = DB_STR;
			vals[6].nul = 0;
			sprintf(aux2, "%d", contact->transport);
			vals[6].val.str_val.s=aux2;
			vals[6].val.str_val.len=strlen(aux2);
		
			vals[7].type = DB_BLOB;
			vals[7].nul = 0;
			str data={x.s, x.len};
			vals[7].val.blob_val = data;
			
			if (pcscf_dbf.insert(pcscf_db, keys, vals, 8) < 0) {
				LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_registrar_to_table(): Error while inserting on snapshot table\n");
				r_unlock(i);
				goto error;
			}
			bin_free(&x);
			
			contact = contact->next;
		}
		r_unlock(i);
	}	
	
	//delete older snapshots
	if (delete_older_snapshots("snapshot", pcscf_name, P_REGISTRAR, snapshot_version)!=1){
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_registrar_to_table(): Error while deleting older snapshots from snapshot table\n");
		goto error;
	}
	
	//unlock
	lock_release(db_lock);
	
	return 1;

error:
	lock_release(db_lock);//unlock
	return 0;
}

/**
 * Dumps P-CSCF dialogs to the snapshot table.
 * Each dialog is serialized and dumped to one row of the table.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_cache_dump_dialogs_to_table(int snapshot_version, int step_version){
	bin_data x;
	p_dialog *dialog;
	int i;
	int len;
	char aux1[10];
	char aux2[10];
	
	//lock
	lock_get(db_lock);

	if (pcscf_dbf.use_table(pcscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_dialogs_to_table(): Error in use_table\n");
		goto error;
	}
	
	for(i=0;i<p_dialogs_hash_size;i++){
		d_lock(i);
		dialog = p_dialogs[i].head;
		while(dialog){
			if (!bin_alloc(&x,128)){
				d_unlock(i);
				goto error;
			}
			if (!bin_encode_p_dialog(&x,dialog)){
				d_unlock(i);
				goto error;
			}
			
			db_key_t keys[9];
			db_val_t vals[9];
			
			/* id auto incremented */
			keys[0] = "node_id";
			keys[1] = "data_type";
			keys[2] = "snapshot_version";
			keys[3] = "step_version";
			keys[4] = "record_id_1"; /* call-id */
			keys[5] = "record_id_2"; /* host */
			keys[6] = "record_id_3"; /* port */
			keys[7] = "record_id_4"; /* transport */
			keys[8] = "data";
			
			vals[0].type = DB_STR;
			vals[0].nul = 0;
			vals[0].val.str_val.s=pcscf_name;
			len = strlen(pcscf_name);
			vals[0].val.str_val.len=MIN(len, 64);

			vals[1].type = DB_INT;
			vals[1].nul = 0;
			vals[1].val.int_val=P_DIALOGS;

			vals[2].type = DB_INT;
			vals[2].nul = 0;
			vals[2].val.int_val=snapshot_version;
	
			vals[3].type = DB_INT;
			vals[3].nul = 0;
			vals[3].val.int_val=step_version;
	
			vals[4].type = DB_STR;
			vals[4].nul = 0;
			vals[4].val.str_val.s=dialog->call_id.s;
			vals[4].val.str_val.len=MIN(dialog->call_id.len, 256);
	
			vals[5].type = DB_STR;
			vals[5].nul = 0;
			vals[5].val.str_val.s=dialog->host.s;
			vals[5].val.str_val.len=MIN(dialog->host.len, 256);
			
			vals[6].type = DB_STR;
			vals[6].nul = 0;
			sprintf(aux1, "%d", dialog->port);
			vals[6].val.str_val.s=aux1;
			vals[6].val.str_val.len=strlen(aux1);
	
			vals[7].type = DB_STR;
			vals[7].nul = 0;
			sprintf(aux2, "%d", dialog->transport);
			vals[7].val.str_val.s=aux2;
			vals[7].val.str_val.len=strlen(aux2);
	
			vals[8].type = DB_BLOB;
			vals[8].nul = 0;
			str data={x.s, x.len};
			vals[8].val.blob_val = data;
			
			if (pcscf_dbf.insert(pcscf_db, keys, vals, 9) < 0) {
				LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_dialogs_to_table(): Error while inserting on snapshot table\n");
				d_unlock(i);
				goto error;
			}
			bin_free(&x);
			
			dialog = dialog->next;
		}
		d_unlock(i);
	}	
	
	//delete older snapshots
	if (delete_older_snapshots("snapshot", pcscf_name, P_DIALOGS, snapshot_version)!=1){
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_dialogs_to_table(): Error while deleting older snapshots from snapshot table\n");
		goto error;
	}
	
	//unlock
	lock_release(db_lock);
	
	return 1;

error:
	lock_release(db_lock);//unlock
	return 0;
}

/**
 * Dumps P-CSCF subscriptions to the snapshot table.
 * Each subscription is serialized and dumped to one row of the table.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_cache_dump_subs_to_table(int snapshot_version, int step_version){
	bin_data x;
	r_subscription *sub;
	int i;
	int len;
	
	//lock
	lock_get(db_lock);

	if (pcscf_dbf.use_table(pcscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_subs_to_table(): Error in use_table\n");
		goto error;
	}
	
	for(i=0;i<subscriptions_hash_size;i++){
		subs_lock(i);
		sub = subscriptions[i].head;
		while(sub){
			if (!bin_alloc(&x,128)){
				subs_unlock(i);
				goto error;
			}
			if (!bin_encode_r_subscription(&x,sub)){
				subs_unlock(i);
				goto error;
			}
			
			db_key_t keys[6];
			db_val_t vals[6];
			
			/* id auto incremented */
			keys[0] = "node_id";
			keys[1] = "data_type";
			keys[2] = "snapshot_version";
			keys[3] = "step_version";
			keys[4] = "record_id_1"; /* req_uri */
			/* record_id_2/3/4=NULL*/
			keys[5] = "data";
			
			vals[0].type = DB_STR;
			vals[0].nul = 0;
			vals[0].val.str_val.s=pcscf_name;
			len = strlen(pcscf_name);
			vals[0].val.str_val.len=MIN(len, 64);

			vals[1].type = DB_INT;
			vals[1].nul = 0;
			vals[1].val.int_val=P_SUBSCRIPTIONS;

			vals[2].type = DB_INT;
			vals[2].nul = 0;
			vals[2].val.int_val=snapshot_version;
	
			vals[3].type = DB_INT;
			vals[3].nul = 0;
			vals[3].val.int_val=step_version;
	
			vals[4].type = DB_STR;
			vals[4].nul = 0;
			vals[4].val.str_val.s=sub->req_uri.s;
			vals[4].val.str_val.len=MIN(sub->req_uri.len, 256);
	
			vals[5].type = DB_BLOB;
			vals[5].nul = 0;
			str data={x.s, x.len};
			vals[5].val.blob_val = data;
			
			if (pcscf_dbf.insert(pcscf_db, keys, vals, 6) < 0) {
				LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_subscriptions_to_table(): Error while inserting on snapshot table\n");
				subs_unlock(i);
				goto error;
			}
			bin_free(&x);
			
			sub = sub->next;
		}
		subs_unlock(i);
	}	
	
	//delete older snapshots
	if (delete_older_snapshots("snapshot", pcscf_name, P_SUBSCRIPTIONS, snapshot_version)!=1){
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_subscriptions_to_table(): Error while deleting older snapshots from snapshot table\n");
		goto error;
	}
	
	//unlock
	lock_release(db_lock);
	
	return 1;

error:
	lock_release(db_lock);//unlock
	return 0;
}


int bin_db_keep_count=1; /**< how many old snapshots to keep */

/**
 * Drops older subscriptions/dialogs/registrar snapshots,
 * keeping the 'bin_db_keep_count' most recent. 
 * @param table - where to drop.
 * @param node_id - cscf id
 * @param dt - which data to drop
 * @param current_snapshot_version - version of the current snapshot
 * @returns 1 on success or 0 on failure
 */
int delete_older_snapshots(char* table, char* node_id, data_type_t dt, int current_snapshot){

	db_key_t query_cols[3];
	db_op_t  query_ops[3];
	db_val_t query_vals[3];
	int len;

	query_cols[0] = "snapshot_version";
	query_ops[0] = OP_LEQ;
	query_vals[0].type = DB_INT;
	query_vals[0].nul = 0;
	query_vals[0].val.int_val = current_snapshot - bin_db_keep_count;
	
	query_cols[1] = "node_id";
	query_ops[1] = OP_EQ;
	query_vals[1].type = DB_STR;
	query_vals[1].nul = 0;
	len = strlen(node_id);
	query_vals[1].val.str_val.s=node_id;
	query_vals[1].val.str_val.len=MIN(len, 256);
	
	query_cols[2] = "data_type";
	query_ops[2] = OP_EQ;
	query_vals[2].type = DB_INT;
	query_vals[2].nul = 0;
	query_vals[2].val.int_val = dt;

	if (pcscf_dbf.use_table(pcscf_db, table) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":delete_older_snapshots(): Error in use_table\n");
		return 0;
	}

	if (pcscf_dbf.delete(pcscf_db, query_cols, query_ops, query_vals, 3) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":delete_older_snapshots(): Error while deleting older snapshots\n");
		return 0;
	}
	
	return 1;
}

	/* Load related functions*/

/**
 * Loads P-CSCF data from DB
 * @param x - Destination of the data.
 * @param dt - dump registrar, dialogs or susbcriptions
 * @returns 1 on success or 0 on failure
 */
int bin_load_from_db(bin_data *x, data_type_t dt){
	switch(dt){
		case P_REGISTRAR:
			return bin_load_registrar_from_table(x);
		case P_DIALOGS:
			return bin_load_dialogs_from_table(x);
		case P_SUBSCRIPTIONS:
			return bin_load_subscriptions_from_table(x);
		default:
			LOG(L_ERR,"ERR:"M_NAME":bin_load_from_db: No such information to load %d\n", dt);
			return 0;
	}
}

/**
 * Loads P-CSCF registrar from DB
 * @param x - Destination of the data
 * @returns 1 on success or 0 on failure
 */
int bin_load_registrar_from_table(bin_data *x){
	if(x){//whole hashtable dumped to db
		return bin_bulk_load_from_table(P_REGISTRAR, x);
	}
	else{//each hashtable element dumped to DB separately
		return bin_cache_load_registrar_from_table();
	}
}

/**
 * Loads P-CSCF dialogs from DB
 * @param x - Destination of the data
 * @returns 1 on success or 0 on failure
 */
int bin_load_dialogs_from_table(bin_data *x){
	if(x){//whole hashtable dumped to db
		return bin_bulk_load_from_table(P_DIALOGS, x);
	}
	else{//each hashtable element dumped to DB separately
		return bin_cache_load_dialogs_from_table();
	}
}

/**
 * Loads P-CSCF subscriptions from DB
 * @param x - Destination of the data
 * @returns 1 on success or 0 on failure
 */
int bin_load_subscriptions_from_table(bin_data *x){
	if(x){//whole hashtable dumped to db
		return bin_bulk_load_from_table(P_SUBSCRIPTIONS, x);
	}
	else{//each hashtable element dumped to DB separately
		return bin_cache_load_subscriptions_from_table();
	}
}

/**
 * Loads P-CSCF data from the snapshot table
 * @param dt - load registrar, dialogs or susbcriptions
 * @param x - where to load
 * @returns 1 on success or 0 on failure
 */
int bin_bulk_load_from_table(data_type_t dt, bin_data* x){
	int snapshot_version;
	int r;
	int len;
	bin_alloc(x,1024);
	
	//lock
	lock_get(db_lock);
	
	if((r=db_get_last_snapshot_version("snapshot", pcscf_name, dt, &snapshot_version))==0){
		LOG(L_ERR,"ERR:"M_NAME":bin_bulk_load_from_table: Error while getting snapshot version\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	else if(r==-1){//empty table, nothing to load
			LOG(L_INFO,"INFO:"M_NAME":bin_bulk_load_from_table: snapshot table empty\n");
			lock_release(db_lock);//unlock
			return 1;
		}
		
	//set snapshot/step versions
	set_versions(dt, snapshot_version+1, 0);
		
	db_key_t keys[3];
	db_val_t vals[3];
	db_op_t ops[3];
	db_key_t result_cols[1];
	
	db_res_t *res = NULL;
	
	keys[0] = "node_id";
	keys[1] = "data_type";
	keys[2] = "snapshot_version";
	
	ops[0] = OP_EQ;
	ops[1] = OP_EQ;
	ops[2] = OP_EQ;

	vals[0].type = DB_STR;
	vals[0].nul = 0;
	vals[0].val.str_val.s = pcscf_name;
	len = strlen(pcscf_name);
	vals[0].val.str_val.len = MIN(len, 256);
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val = dt;

	vals[2].type = DB_INT;
	vals[2].nul = 0;
	vals[2].val.int_val = snapshot_version;
	
	result_cols[0]="data";
	
	if (pcscf_dbf.use_table(pcscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_load_from_table(): Error in use_table\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	
	if (pcscf_dbf.query(pcscf_db, keys, ops, vals, result_cols, 3, 1, 0, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_load_from_table(): Error while querying snapshot table\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	
	if (!res) {
		lock_release(db_lock);//unlock
		return 0;
	}
	
	if(res->n==0){
		pcscf_dbf.free_result(pcscf_db, res);
		lock_release(db_lock);//unlock
		return 1;
	}
	
	db_row_t* rows = RES_ROWS(res);
	db_val_t *row_vals = ROW_VALUES(rows);
	
	len=row_vals[0].val.blob_val.len;
	LOG(L_INFO,"INFO:"M_NAME":snaphot row length -> %d\n", len);
	bin_expand(x, len);
	
	memcpy(x->s, row_vals[0].val.blob_val.s, len);
	x->len+=len;
	
	pcscf_dbf.free_result(pcscf_db, res);
	
	lock_release(db_lock);//unlock
	
	return 1;
}

/**
 * Loads P-CSCF registrar from the snapshot table.
 * Each contact is placed on the registrar hastable here.
 * @returns 1 on success or 0 on failure
 */
int bin_cache_load_registrar_from_table(){
	int snapshot_version;
	int r, len, i=0;
	bin_data x;
	r_contact *contact;
	
	//lock
	lock_get(db_lock);
	
	if((r=db_get_last_snapshot_version("snapshot", pcscf_name, P_REGISTRAR, &snapshot_version))==0){
		LOG(L_ERR,"ERR:"M_NAME":bin_cache_load_registrar_from_table: Error while getting snapshot version\n");
		goto error;
	}
	else if(r==-1){//empty table, nothing to load
			LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_registrar_from_table: snapshot table empty\n");
			lock_release(db_lock);//unlock
			return 1;
		}
		
	//set snapshot/step versions
	set_versions(P_REGISTRAR, snapshot_version+1, 0);
		
	db_key_t keys[3];
	db_val_t vals[3];
	db_op_t ops[3];
	db_key_t result_cols[1];
	
	db_res_t *res = NULL;
	
	keys[0] = "node_id";
	keys[1] = "data_type";
	keys[2] = "snapshot_version";
	
	ops[0] = OP_EQ;
	ops[1] = OP_EQ;
	ops[2] = OP_EQ;

	vals[0].type = DB_STR;
	vals[0].nul = 0;
	vals[0].val.str_val.s = pcscf_name;
	len = strlen(pcscf_name);
	vals[0].val.str_val.len = MIN(len, 256);
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val = P_REGISTRAR;

	vals[2].type = DB_INT;
	vals[2].nul = 0;
	vals[2].val.int_val = snapshot_version;
	
	result_cols[0]="data";
	
	if (pcscf_dbf.use_table(pcscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_registrar_from_table(): Error in use_table\n");
		goto error;
	}
	
	if (pcscf_dbf.query(pcscf_db, keys, ops, vals, result_cols, 3, 1, 0, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_registrar_from_table(): Error while querying snapshot table\n");
		goto error;
	}
	
	if (!res) goto error;
	
	for(i=0;i<res->n;i++){
		db_row_t *row = &res->rows[i];
		db_val_t *row_vals = ROW_VALUES(row);
		
		len = row_vals[0].val.blob_val.len;
		bin_alloc(&x,len);
		memcpy(x.s, row_vals[0].val.blob_val.s, len);
		x.len=len;
		x.max=0;
		contact = bin_decode_r_contact(&x);
		if (!contact){
			pcscf_dbf.free_result(pcscf_db, res);
			goto error;
		};
		LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_registrar_from_table():Loaded r_contact for <%.*s>\n",contact->uri.len,contact->uri.s);
		r_lock(contact->hash);
		contact->prev = registrar[contact->hash].tail;
		contact->next = 0;
		if (registrar[contact->hash].tail) registrar[contact->hash].tail->next = contact;
		registrar[contact->hash].tail = contact;
		if (!registrar[contact->hash].head) registrar[contact->hash].head = contact;
		r_unlock(contact->hash);
		bin_free(&x);
	}
	
	pcscf_dbf.free_result(pcscf_db, res);
	lock_release(db_lock);//unlock
	return 1;
	
error:
	lock_release(db_lock);//unlock
	return 0;
}

/**
 * Loads P-CSCF dialogs from the snapshot table.
 * Each dialog is placed on the dialogs hastable here.
 * @returns 1 on success or 0 on failure
 */
int bin_cache_load_dialogs_from_table(){
	int snapshot_version;
	int r, len, i=0;
	bin_data x;
	p_dialog *dialog;
	
	//lock
	lock_get(db_lock);
	
	if((r=db_get_last_snapshot_version("snapshot", pcscf_name, P_DIALOGS, &snapshot_version))==0){
		LOG(L_ERR,"ERR:"M_NAME":bin_cache_load_dialogs_from_table: Error while getting snapshot version\n");
		goto error;
	}
	else if(r==-1){//empty table, nothing to load
			LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_dialogs_from_table: snapshot table empty\n");
			lock_release(db_lock);//unlock
			return 1;
		}
		
	//set snapshot/step versions
	set_versions(P_DIALOGS, snapshot_version+1, 0);
		
	db_key_t keys[3];
	db_val_t vals[3];
	db_op_t ops[3];
	db_key_t result_cols[1];
	
	db_res_t *res = NULL;
	
	keys[0] = "node_id";
	keys[1] = "data_type";
	keys[2] = "snapshot_version";
	
	ops[0] = OP_EQ;
	ops[1] = OP_EQ;
	ops[2] = OP_EQ;

	vals[0].type = DB_STR;
	vals[0].nul = 0;
	vals[0].val.str_val.s = pcscf_name;
	len = strlen(pcscf_name);
	vals[0].val.str_val.len = MIN(len, 256);
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val = P_DIALOGS;

	vals[2].type = DB_INT;
	vals[2].nul = 0;
	vals[2].val.int_val = snapshot_version;
	
	result_cols[0]="data";
	
	if (pcscf_dbf.use_table(pcscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_registrar_from_table(): Error in use_table\n");
		goto error;
	}
	
	if (pcscf_dbf.query(pcscf_db, keys, ops, vals, result_cols, 3, 1, 0, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_dialogs_from_table(): Error while querying snapshot table\n");
		goto error;
	}
	
	if (!res) goto error;
	
	for(i=0;i<res->n;i++){
		db_row_t *row = &res->rows[i];
		db_val_t *row_vals = ROW_VALUES(row);
		
		len = row_vals[0].val.blob_val.len;
		bin_alloc(&x,len);
		memcpy(x.s, row_vals[0].val.blob_val.s, len);
		x.len=len;
		x.max=0;
		dialog = bin_decode_p_dialog(&x);
		if (!dialog){
			pcscf_dbf.free_result(pcscf_db, res);
			goto error;
		};
		LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_registrar_from_table(): Loaded p_dialog for <%.*s>\n",dialog->host.len,dialog->host.s);
		d_lock(dialog->hash);
		dialog->prev = p_dialogs[dialog->hash].tail;
		dialog->next = 0;
		if (p_dialogs[dialog->hash].tail) p_dialogs[dialog->hash].tail->next = dialog;
		p_dialogs[dialog->hash].tail = dialog;
		if (!p_dialogs[dialog->hash].head) p_dialogs[dialog->hash].head = dialog;
		d_unlock(dialog->hash);
		bin_free(&x);
	}
	
	pcscf_dbf.free_result(pcscf_db, res);
	lock_release(db_lock);//unlock
	return 1;
	
error:
	lock_release(db_lock);//unlock
	return 0;
}

/**
 * Loads P-CSCF subscriptions from the snapshot table.
 * Each subscription is placed on the subscriptions hastable here.
 * @returns 1 on success or 0 on failure
 */
int bin_cache_load_subscriptions_from_table(){
	int snapshot_version;
	int r, len, i=0;
	bin_data x;
	r_subscription *sub;
	
	//lock
	lock_get(db_lock);
	
	if((r=db_get_last_snapshot_version("snapshot", pcscf_name, P_SUBSCRIPTIONS, &snapshot_version))==0){
		LOG(L_ERR,"ERR:"M_NAME":bin_cache_load_subscriptions_from_table: Error while getting snapshot version\n");
		goto error;
	}
	else if(r==-1){//empty table, nothing to load
			LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_subscriptions_from_table: snapshot table empty\n");
			lock_release(db_lock);//unlock
			return 1;
		}
		
	//set snapshot/step versions
	set_versions(P_SUBSCRIPTIONS, snapshot_version+1, 0);
		
	db_key_t keys[3];
	db_val_t vals[3];
	db_op_t ops[3];
	db_key_t result_cols[1];
	
	db_res_t *res = NULL;
	
	keys[0] = "node_id";
	keys[1] = "data_type";
	keys[2] = "snapshot_version";
	
	ops[0] = OP_EQ;
	ops[1] = OP_EQ;
	ops[2] = OP_EQ;

	vals[0].type = DB_STR;
	vals[0].nul = 0;
	vals[0].val.str_val.s = pcscf_name;
	len = strlen(pcscf_name);
	vals[0].val.str_val.len = MIN(len, 256);
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val = P_SUBSCRIPTIONS;

	vals[2].type = DB_INT;
	vals[2].nul = 0;
	vals[2].val.int_val = snapshot_version;
	
	result_cols[0]="data";
	
	if (pcscf_dbf.use_table(pcscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_subscriptions_from_table(): Error in use_table\n");
		goto error;
	}
	
	if (pcscf_dbf.query(pcscf_db, keys, ops, vals, result_cols, 3, 1, 0, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_subscriptions_from_table(): Error while querying snapshot table\n");
		goto error;
	}
	
	if (!res) goto error;
	
	for(i=0;i<res->n;i++){
		db_row_t *row = &res->rows[i];
		db_val_t *row_vals = ROW_VALUES(row);
		
		len = row_vals[0].val.blob_val.len;
		bin_alloc(&x,len);
		memcpy(x.s, row_vals[0].val.blob_val.s, len);
		x.len=len;
		x.max=0;
		sub = bin_decode_r_subscription(&x);
		if (!sub){
			pcscf_dbf.free_result(pcscf_db, res);
			goto error;
		};
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_subscriptions: Loaded r_subscription for <%.*s>\n",sub->req_uri.len,sub->req_uri.s);
		subs_lock(sub->hash);
		sub->prev = subscriptions[sub->hash].tail;
		sub->next = 0;
		if (subscriptions[sub->hash].tail) subscriptions[sub->hash].tail->next = sub;
		subscriptions[sub->hash].tail = sub;
		if (!subscriptions[sub->hash].head) subscriptions[sub->hash].head = sub;
		subs_unlock(sub->hash);
		bin_free(&x);
	}
	
	pcscf_dbf.free_result(pcscf_db, res);
	lock_release(db_lock);//unlock
	return 1;
	
error:
	lock_release(db_lock);//unlock
	return 0;
}

/**
 * Gets the version of the last snapshot dumped to db.
 * Used to load information from db on startup.
 * @param table - the table to query
 * @param node_id - cscf id
 * @param dt - registrar, dialogs or subscriptions
 * @param version - where to load
 * @returns 1 on success, -1 if empty table, 0 on error
 */
int db_get_last_snapshot_version(char* table, char* node_id, data_type_t dt, int* version){
	db_res_t *res = NULL;
	char sql[200];
	
	sprintf(sql, "SELECT max(snapshot_version) from %s where node_id='%s' and data_type=%d", table, node_id, dt);
	
	if (pcscf_dbf.raw_query(pcscf_db, sql, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":db_get_last_snapshot_version(): Error while getting last snapshot version from %s\n", table);
		return 0;
	}
	
	if (!res) return 0;
	
	/*if(res->n==0) { //never happens with MAX function
		LOG(L_ERR, "ERR:"M_NAME":db_get_last_snapshot_version(): %s empty\n", table);
		pcscf_dbf.free_result(pcscf_db, res);
		return -1;
	}*/
	
	db_row_t* rows = RES_ROWS(res);
	db_val_t *row_vals = ROW_VALUES(rows);
	
	//MAX/MIN of an empty table will return a row of value NULL
	if(VAL_NULL(&row_vals[0])) {
		LOG(L_INFO, "INFO:"M_NAME":db_get_last_snapshot_version(): %s empty\n", table);
		pcscf_dbf.free_result(pcscf_db, res);
		return -1;
	}
	
	*version=row_vals[0].val.int_val;
	
	LOG(L_INFO, "INFO:"M_NAME":db_get_last_snapshot_version(): %s -> %d\n", table, *version);
	
	pcscf_dbf.free_result(pcscf_db, res);
	
	return 1;
}

/**
 * Sets the values of the global variables registrar/dialogs/subs_snapshot_version
 * and registrar/dialogs/subs_step_version.
 * @param dt - subs, dialogs or registrar
 * @param snapshot_version - it must continue from the last snapshot version + 1
 * @param step_version - it must continue from the last step version + 1 
 * @returns 1 on success, 0 on error
 */
int set_versions(data_type_t dt, int snapshot_version, int step_version){
	switch(dt){
		case P_REGISTRAR:
			*registrar_snapshot_version=snapshot_version;
			*registrar_step_version=step_version;
			break;
		case P_DIALOGS:
			*dialogs_snapshot_version=snapshot_version;
			*dialogs_step_version=step_version;
			break;
		case P_SUBSCRIPTIONS:
			*subs_snapshot_version=snapshot_version;
			*subs_step_version=step_version;
			break;
		default:
			LOG(L_ERR,"ERR:"M_NAME"set_versions: No such information %d\n", dt);
			return 0;
	}
	
	return 1;
}
