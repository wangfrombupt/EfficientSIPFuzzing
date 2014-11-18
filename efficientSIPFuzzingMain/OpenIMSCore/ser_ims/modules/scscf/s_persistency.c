/*
 * $Id: s_persistency.c 236 2007-04-18 12:53:40Z vingarzan $
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
 * S-CSCF persistency operations
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 * Database persistency implemented by Mario Ferreira (PT INOVACAO)
 * \author Mario Ferreira est-m-aferreira at ptinovacao.pt
 *
 */

#include "s_persistency.h"

extern persistency_mode_t scscf_persistency_mode;/**< the type of persistency					*/
extern char* scscf_persistency_location;		/**< where to dump the persistency data 		*/ 


extern auth_hash_slot_t *auth_data;				/**< Authentication vector hash table 			*/
extern int auth_data_hash_size;					/**< authentication vector hash table size 		*/
extern int r_hash_size;						/**< Size of S-CSCF registrar hash table		*/
extern r_hash_slot *registrar;				/**< The S-CSCF registrar 						*/
extern int s_dialogs_hash_size;						/**< size of the dialog hash table 					*/
extern s_dialog_hash_slot *s_dialogs;				/**< the hash table									*/


/*****  DB related stuff *****/
extern db_con_t* scscf_db; /**< Database connection handle */
extern db_func_t scscf_dbf;	/**< Structure with pointers to db functions */
extern int* auth_snapshot_version;
extern int* auth_step_version;
extern int* dialogs_snapshot_version;
extern int* dialogs_step_version;
extern int* registrar_snapshot_version;
extern int* registrar_step_version;
extern gen_lock_t* db_lock; 
extern char* scscf_name;

int s_dump(bin_data* x, char* location, char* prepend_fname, data_type_t dt);
int bin_dump_to_db(bin_data *x, data_type_t dt);
int bin_dump_registrar_to_table(bin_data *x, int snapshot_version, int step_version);
int bin_dump_dialogs_to_table(bin_data *x, int snapshot_version, int step_version);
int bin_dump_auth_to_table(bin_data *x, int snapshot_version, int step_version);
int bin_bulk_dump_to_table(data_type_t dt, int snapshot_version, int step_version, bin_data *x);
int bin_cache_dump_registrar_to_table(int snapshot_version, int step_version);
int bin_cache_dump_dialogs_to_table(int snapshot_version, int step_version);
int bin_cache_dump_auth_to_table(int snapshot_version, int step_version);
int delete_older_snapshots(char* table, char* node_id, data_type_t dt, int current_snapshot);

int s_load(bin_data *x, char* location, char* prepend_fname, data_type_t dt);
int bin_load_from_db(bin_data *x, data_type_t dt);
int bin_load_registrar_from_table(bin_data *x);
int bin_load_dialogs_from_table(bin_data *x);
int bin_load_auth_from_table(bin_data *x);
int bin_bulk_load_from_table(data_type_t dt, bin_data* x);
int bin_cache_load_registrar_from_table();
int bin_cache_load_dialogs_from_table();
int bin_cache_load_auth_from_table();
int db_get_last_snapshot_version(char* table, char* node_id, data_type_t dt, int* version);
int set_versions(data_type_t dt, int snapshot_version, int step_version);
/*********************/

/**
 * Creates a snapshots of the authorization data and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_authdata()
{
	bin_data x;
	auth_userdata *aud;
	int i;
	
	/*In WITH_DATABASE_CACHE mode, serialize each hashtable element separately */
	if(scscf_persistency_mode!=WITH_DATABASE_CACHE){	
		if (!bin_alloc(&x,256)) goto error;
		for(i=0;i<auth_data_hash_size;i++){
			auth_data_lock(i);
			aud = auth_data[i].head;
			while(aud){
				if (!bin_encode_auth_userdata(&x,aud)) goto error;
				aud = aud->next;
			}
			auth_data_unlock(i);
		}
		//bin_print(&x);
	}
	i=s_dump(&x,scscf_persistency_location,"authdata", S_AUTH);
	//i = bin_dump(&x,scscf_persistency_mode,scscf_persistency_location,"authdata");		
	if(scscf_persistency_mode!=WITH_DATABASE_CACHE){
		bin_free(&x);
	}
	return i;
error:
	return 0;
}
  

/**
 * Loads the authorization data from the last snapshot.
 * @returns 1 on success or 0 on failure
 */
int load_snapshot_authdata()
{
	bin_data x;
	auth_userdata *aud;
	if (!s_load(&x,scscf_persistency_location,"authdata", S_AUTH)) goto error;
	//if (!bin_load(&x,scscf_persistency_mode,scscf_persistency_location,"authdata")) goto error;
	
	if(scscf_persistency_mode!=WITH_DATABASE_CACHE){
		//bin_print(&x);
		x.max=0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_auth: max %d len %d\n",x.max,x.len);
		while(x.max<x.len){
			aud = bin_decode_auth_userdata(&x);		
			if (!aud) return 0;
			LOG(L_INFO,"INFO:"M_NAME":load_snapshot_auth: Loaded auth_userdata for <%.*s>\n",aud->private_identity.len,aud->private_identity.s);
			auth_data_lock(aud->hash);
			aud->prev = auth_data[aud->hash].tail;
			aud->next = 0;
			if (auth_data[aud->hash].tail) auth_data[aud->hash].tail->next = aud;
			auth_data[aud->hash].tail = aud;
			if (!auth_data[aud->hash].head) auth_data[aud->hash].head = aud;
			auth_data_unlock(aud->hash);
		}
		bin_free(&x);
	}
	return 1;
error:
	return 0;
}

/**
 * Timer callback for persistency dumps
 * @param ticks - what's the time
 * @param param - a given parameter to be called with
 */
void persistency_timer_authdata(unsigned int ticks, void* param)
{
	make_snapshot_authdata();
	
	if (auth_snapshot_version) (*auth_snapshot_version)++; 	
}



/**
 * Creates a snapshots of the dialogs data and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_dialogs()
{
	bin_data x;
	s_dialog *d;
	int i;	
	
	/*In WITH_DATABASE_CACHE mode, serialize each hashtable element separately */
	if(scscf_persistency_mode!=WITH_DATABASE_CACHE){
		if (!bin_alloc(&x,256)) goto error;		
		for(i=0;i<s_dialogs_hash_size;i++){
			d_lock(i);
			d = s_dialogs[i].head;
			while(d){
				if (!bin_encode_s_dialog(&x,d)) goto error;
				d = d->next;
			}
			d_unlock(i);
		}
		//bin_print(&x);
	}
	i=s_dump(&x,scscf_persistency_location,"sdialogs",S_DIALOGS);
	//i = bin_dump(&x,scscf_persistency_mode,scscf_persistency_location,"sdialogs");		
	if(scscf_persistency_mode!=WITH_DATABASE_CACHE){
		bin_free(&x);
	}
	return i;
error:
	return 0;
}  

/**
 * Loads the dialogs data from the last snapshot.
 * @returns 1 on success or 0 on failure
 */
int load_snapshot_dialogs()
{
	bin_data x;
	s_dialog *d;
	if (!s_load(&x,scscf_persistency_location,"sdialogs",S_DIALOGS)) goto error;
	//if (!bin_load(&x,scscf_persistency_mode,scscf_persistency_location,"sdialogs")) goto error;
	
	if(scscf_persistency_mode!=WITH_DATABASE_CACHE){
		//bin_print(&x);
		x.max=0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dlg: max %d len %d\n",x.max,x.len);
		while(x.max<x.len){
			d = bin_decode_s_dialog(&x);
			if (!d) return 0;
			LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dlg: Loaded s_dialog for <%.*s>\n",d->aor.len,d->aor.s);
			d_lock(d->hash);
			d->prev = s_dialogs[d->hash].tail;
			d->next = 0;
			if (s_dialogs[d->hash].tail) s_dialogs[d->hash].tail->next = d;
			s_dialogs[d->hash].tail = d;
			if (!s_dialogs[d->hash].head) s_dialogs[d->hash].head = d;
			d_unlock(d->hash);
		}
		bin_free(&x);
	}
	return 1;
error:
	return 0;
}


/**
 * Timer callback for persistency dumps
 * @param ticks - what's the time
 * @param param - a given parameter to be called with
 */
void persistency_timer_dialogs(unsigned int ticks, void* param)
{
	make_snapshot_dialogs();
	
	if (dialogs_snapshot_version) (*dialogs_snapshot_version)++;	 	
}


/**
 * Creates a snapshots of the registrar and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_registrar()
{
	bin_data x;
	r_public *p;
	int i;
	
	/*In WITH_DATABASE_CACHE mode, serialize each hashtable element separately */
	if(scscf_persistency_mode!=WITH_DATABASE_CACHE){
		if (!bin_alloc(&x,256)) goto error;		
		for(i=0;i<r_hash_size;i++){
			r_lock(i);
			p = registrar[i].head;
			while(p){
				if (!bin_encode_r_public(&x,p)) goto error;
				p = p->next;
			}
			r_unlock(i);
		}
		//bin_print(&x);
	}
	i = s_dump(&x,scscf_persistency_location,"sregistrar",S_REGISTRAR);
	//i = bin_dump(&x,scscf_persistency_mode,scscf_persistency_location,"sregistrar");		
	if(scscf_persistency_mode!=WITH_DATABASE_CACHE){
		bin_free(&x);
	}
	return i;
error:
	return 0;
}  

/**
 * Loads the registrar data from the last snapshot.
 * @returns 1 on success or 0 on failure
 */
int load_snapshot_registrar()
{
	bin_data x;
	r_public *p;
	if (!s_load(&x,scscf_persistency_location,"sregistrar",S_REGISTRAR)) goto error;
	//if (!bin_load(&x,scscf_persistency_mode,scscf_persistency_location,"sregistrar")) goto error;
	
	if(scscf_persistency_mode!=WITH_DATABASE_CACHE){
		//bin_print(&x);
		x.max=0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_registrar: max %d len %d\n",x.max,x.len);
		while(x.max<x.len){
			p = bin_decode_r_public(&x);
			if (!p) return 0;
			LOG(L_INFO,"INFO:"M_NAME":load_snapshot_registrar: Loaded r_public for <%.*s>\n",p->aor.len,p->aor.s);
			r_lock(p->hash);
			p->prev = registrar[p->hash].tail;
			p->next = 0;
			if (registrar[p->hash].tail) registrar[p->hash].tail->next = p;
			registrar[p->hash].tail = p;
			if (!registrar[p->hash].head) registrar[p->hash].head = p;
			r_unlock(p->hash);
		}
		bin_free(&x);
	}
	return 1;
error:
	return 0;
	
}


/**
 * Timer callback for persistency dumps
 * @param ticks - what's the time
 * @param param - a given parameter to be called with
 */
void persistency_timer_registrar(unsigned int ticks, void* param)
{
	make_snapshot_registrar();
	
	if (registrar_snapshot_version) (*registrar_snapshot_version)++;
}


/******  DB related functions  *****/

	/* Dump related functions*/
	
/**
 * Dumps S-CSCF data to files or DB
 * @param x - binary data to dump. Empty if scscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each hashtable element is serialized and dumped to DB separately.
 * @param location -
 * @param prepend_fname -
 * @param dt - dump registrar, dialogs or auth
 * @returns 1 on success or 0 on failure
 */
int s_dump(bin_data* x, char* location, char* prepend_fname, data_type_t dt){
	
	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:
			LOG(L_ERR,"ERR:"M_NAME":s_dump: Snapshot done but persistency was disabled...\n");
			return 0;
		case WITH_FILES:
			return bin_dump_to_file(x,location,prepend_fname);
		case WITH_DATABASE_BULK:
			return bin_dump_to_db(x, dt);
		case WITH_DATABASE_CACHE:
			return bin_dump_to_db(NULL, dt);//ignore x, x is empty
		default:
			LOG(L_ERR,"ERR:"M_NAME":s_dump: Snapshot done but no such mode %d\n",scscf_persistency_mode);
			return 0;
	}
}

/**
 * Dumps S-CSCF data to DB
 * @param x - binary data to dump. NULL if scscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each hashtable element is serialized and dumped to DB separately.
 * @param dt - dump registrar, dialogs or auth
 * @returns 1 on success or 0 on failure
 */
int bin_dump_to_db(bin_data *x, data_type_t dt){
	
	int snapshot_version;
	int step_version;
	
	switch(dt){
		case S_REGISTRAR:
			snapshot_version=*registrar_snapshot_version;
			step_version=*registrar_step_version;
			return bin_dump_registrar_to_table(x, snapshot_version, step_version);
		case S_DIALOGS:
			snapshot_version=*dialogs_snapshot_version;
			step_version=*dialogs_step_version;
			return bin_dump_dialogs_to_table(x, snapshot_version, step_version);
		case S_AUTH:
			snapshot_version=*auth_snapshot_version;
			step_version=*auth_step_version;
			return bin_dump_auth_to_table(x, snapshot_version, step_version);
		default:
			LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_db: No such information to dump %d\n", dt);
			return 0;
	}
}

/**
 * Dumps S-CSCF registrar to DB
 * @param x - binary data to dump. NULL if scscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each hashtable element is serialized and dumped to DB separately.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_dump_registrar_to_table(bin_data *x, int snapshot_version, int step_version){
	
	if(x){//whole hashtable serialized to x
		return bin_bulk_dump_to_table(S_REGISTRAR, snapshot_version, step_version, x);
	}
	else{//serialize and dump each hashtable element separately
		return bin_cache_dump_registrar_to_table(snapshot_version, step_version);
	}
}

/**
 * Dumps S-CSCF dialogs to DB
 * @param x - binary data to dump. NULL if scscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each hashtable element is serialized and dumped to DB separately.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_dump_dialogs_to_table(bin_data *x, int snapshot_version, int step_version){
	
	if(x){//whole hashtable serialized to x
		return bin_bulk_dump_to_table(S_DIALOGS, snapshot_version, step_version, x);
	}
	else{//serialize and dump each hashtable element separately
		return bin_cache_dump_dialogs_to_table(snapshot_version, step_version);
	}
}

/**
 * Dumps S-CSCF auth to DB
 * @param x - binary data to dump. NULL if scscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each hashtable element is serialized and dumped to DB separately.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_dump_auth_to_table(bin_data *x, int snapshot_version, int step_version){
	
	if(x){//whole hashtable serialized to x
		return bin_bulk_dump_to_table(S_AUTH, snapshot_version, step_version, x);
	}
	else{//serialize and dump each hashtable element separately
		return bin_cache_dump_auth_to_table(snapshot_version, step_version);
	}
}

/**
 * Dumps S-CSCF data to the snapshot table
 * @param dt - dump registrar, dialogs or auth
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
	vals[0].val.str_val.s=scscf_name;
	len = strlen(scscf_name);
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

	if (scscf_dbf.use_table(scscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_dump_to_table(): Error in use_table\n");
		lock_release(db_lock);//unlock
		return 0;
	}

	if (scscf_dbf.insert(scscf_db, keys, vals, 5) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_dump_to_table(): Error while inserting on snapshot table\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	
	//delete older snapshots
	if (delete_older_snapshots("snapshot", scscf_name, dt, snapshot_version)!=1){
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_dump_to_table(): Error while deleting older snapshots from snapshot table\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	
	//unlock
	lock_release(db_lock);
	
	return 1;
}

/**
 * Dumps S-CSCF registrar to the snapshot table.
 * Each r_public is serialized and dumped to one row of the table.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_cache_dump_registrar_to_table(int snapshot_version, int step_version){
	bin_data x;
	r_public *p;
	int i;
	int len;
	
	//lock
	lock_get(db_lock);

	if (scscf_dbf.use_table(scscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_registrar_to_table(): Error in use_table\n");
		goto error;
	}
	
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
		p = registrar[i].head;
		while(p){
			if (!bin_alloc(&x,128)){
				r_unlock(i);
				goto error;
			}
			if (!bin_encode_r_public(&x,p)){
				r_unlock(i);
				goto error;
			}
			
			db_key_t keys[6];
			db_val_t vals[6];
			
			/* id auto incremented */
			keys[0] = "node_id";
			keys[1] = "data_type";
			keys[2] = "snapshot_version";
			keys[3] = "step_version";
			keys[4] = "record_id_1"; /* aor */
			/* record_id_2/3/4=NULL */
			keys[5] = "data";
			
			vals[0].type = DB_STR;
			vals[0].nul = 0;
			vals[0].val.str_val.s=scscf_name;
			len = strlen(scscf_name);
			vals[0].val.str_val.len=MIN(len, 64);

			vals[1].type = DB_INT;
			vals[1].nul = 0;
			vals[1].val.int_val=S_REGISTRAR;

			vals[2].type = DB_INT;
			vals[2].nul = 0;
			vals[2].val.int_val=snapshot_version;
	
			vals[3].type = DB_INT;
			vals[3].nul = 0;
			vals[3].val.int_val=step_version;
	
			vals[4].type = DB_STR;
			vals[4].nul = 0;
			vals[4].val.str_val.s=p->aor.s;
			vals[4].val.str_val.len=MIN(p->aor.len, 256);
		
			vals[5].type = DB_BLOB;
			vals[5].nul = 0;
			str data={x.s, x.len};
			vals[5].val.blob_val = data;
			
			if (scscf_dbf.insert(scscf_db, keys, vals, 6) < 0) {
				LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_registrar_to_table(): Error while inserting on snapshot table\n");
				r_unlock(i);
				goto error;
			}
			bin_free(&x);
			
			p=p->next;
		}
		r_unlock(i);
	}	
	
	//delete older snapshots
	if (delete_older_snapshots("snapshot", scscf_name, S_REGISTRAR, snapshot_version)!=1){
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
 * Dumps S-CSCF dialogs to the snapshot table.
 * Each dialog is serialized and dumped to one row of the table.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_cache_dump_dialogs_to_table(int snapshot_version, int step_version){
	bin_data x;
	s_dialog *dialog;
	int i;
	int len;
	
	//lock
	lock_get(db_lock);

	if (scscf_dbf.use_table(scscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_dialogs_to_table(): Error in use_table\n");
		goto error;
	}
	
	for(i=0;i<s_dialogs_hash_size;i++){
		d_lock(i);
		dialog = s_dialogs[i].head;
		while(dialog){
			if (!bin_alloc(&x,128)){
				d_unlock(i);
				goto error;
			}
			if (!bin_encode_s_dialog(&x,dialog)){
				d_unlock(i);
				goto error;
			}
			
			db_key_t keys[7];
			db_val_t vals[7];
			
			/* id auto incremented */
			keys[0] = "node_id";
			keys[1] = "data_type";
			keys[2] = "snapshot_version";
			keys[3] = "step_version";
			keys[4] = "record_id_1"; /* call-id */
			keys[5] = "record_id_2"; /* aor */
			/* record_id_3/4=NULL */
			keys[6] = "data";
			
			vals[0].type = DB_STR;
			vals[0].nul = 0;
			vals[0].val.str_val.s=scscf_name;
			len = strlen(scscf_name);
			vals[0].val.str_val.len=MIN(len, 64);

			vals[1].type = DB_INT;
			vals[1].nul = 0;
			vals[1].val.int_val=S_DIALOGS;

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
			vals[5].val.str_val.s=dialog->aor.s;
			vals[5].val.str_val.len=MIN(dialog->aor.len, 256);
	
			vals[6].type = DB_BLOB;
			vals[6].nul = 0;
			str data={x.s, x.len};
			vals[6].val.blob_val = data;
			
			if (scscf_dbf.insert(scscf_db, keys, vals, 7) < 0) {
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
	if (delete_older_snapshots("snapshot", scscf_name, S_DIALOGS, snapshot_version)!=1){
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
 * Dumps S-CSCF auth data to the snapshot table.
 * Each auth hashtable element is serialized and dumped to one row of the table.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_cache_dump_auth_to_table(int snapshot_version, int step_version){
	bin_data x;
	auth_userdata *aud;
	int i;
	int len;
	
	//lock
	lock_get(db_lock);

	if (scscf_dbf.use_table(scscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_auth_to_table(): Error in use_table\n");
		goto error;
	}
	
	for(i=0;i<auth_data_hash_size;i++){
		auth_data_lock(i);
		aud = auth_data[i].head;
		while(aud){
			if (!bin_alloc(&x,128)){
				auth_data_unlock(i);
				goto error;
			}
			if (!bin_encode_auth_userdata(&x,aud)){
				auth_data_unlock(i);
				goto error;
			}
			
			db_key_t keys[7];
			db_val_t vals[7];
			
			/* id auto incremented */
			keys[0] = "node_id";
			keys[1] = "data_type";
			keys[2] = "snapshot_version";
			keys[3] = "step_version";
			keys[4] = "record_id_1"; /* private */
			keys[5] = "record_id_2"; /* public */
			/* record_id_3/4=NULL*/
			keys[6] = "data";
			
			vals[0].type = DB_STR;
			vals[0].nul = 0;
			vals[0].val.str_val.s=scscf_name;
			len = strlen(scscf_name);
			vals[0].val.str_val.len=MIN(len, 64);

			vals[1].type = DB_INT;
			vals[1].nul = 0;
			vals[1].val.int_val=S_AUTH;

			vals[2].type = DB_INT;
			vals[2].nul = 0;
			vals[2].val.int_val=snapshot_version;
	
			vals[3].type = DB_INT;
			vals[3].nul = 0;
			vals[3].val.int_val=step_version;
	
			vals[4].type = DB_STR;
			vals[4].nul = 0;
			vals[4].val.str_val.s=aud->private_identity.s;
			vals[4].val.str_val.len=MIN(aud->private_identity.len, 256);
			
			vals[5].type = DB_STR;
			vals[5].nul = 0;
			vals[5].val.str_val.s=aud->public_identity.s;
			vals[5].val.str_val.len=MIN(aud->public_identity.len, 256);
	
			vals[6].type = DB_BLOB;
			vals[6].nul = 0;
			str data={x.s, x.len};
			vals[6].val.blob_val = data;
			
			if (scscf_dbf.insert(scscf_db, keys, vals, 7) < 0) {
				LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_auth_to_table(): Error while inserting on snapshot table\n");
				auth_data_unlock(i);
				goto error;
			}
			bin_free(&x);
			
			aud = aud->next;
		}
		auth_data_unlock(i);
	}	
	
	//delete older snapshots
	if (delete_older_snapshots("snapshot", scscf_name, S_AUTH, snapshot_version)!=1){
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_dump_auth_to_table(): Error while deleting older snapshots from snapshot table\n");
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
 * Drops older auth/dialogs/registrar snapshots,
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

	if (scscf_dbf.use_table(scscf_db, table) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":delete_older_snapshots(): Error in use_table\n");
		return 0;
	}

	if (scscf_dbf.delete(scscf_db, query_cols, query_ops, query_vals, 3) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":delete_older_snapshots(): Error while deleting older snapshots\n");
		return 0;
	}
	
	return 1;
}


	/* Load related functions*/
	
/**
 * Loads S-CSCF data from files or DB
 * @param x - destination of the loaded data. Empty if scscf_persistency_mode==WITH_DATABASE_CACHE.
 * In this case, each row of the table is loaded and placed on the hashtable now, instead of to be
 * passed to load_snapshot_...() function, which does this work when the whole hashtable was dumped
 * to only one row of the table.
 * @param location -
 * @param prepend_fname -
 * @param dt - load registrar, dialogs or auth
 * @returns 1 on success or 0 on failure
 */
int s_load(bin_data *x, char* location, char* prepend_fname, data_type_t dt){
	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:
			LOG(L_ERR,"ERR:"M_NAME":s_load: Persistency support was disabled\n");
			return 0;
		case WITH_FILES:
			return bin_load_from_file(x,location,prepend_fname);		
		case WITH_DATABASE_BULK:
			return bin_load_from_db(x, dt);
		case WITH_DATABASE_CACHE:
			return bin_load_from_db(NULL, dt); //ignore x, x is empty
		default:
			LOG(L_ERR,"ERR:"M_NAME":s_load: Can't resume because no such mode %d\n",scscf_persistency_mode);
			return 0;
	}
}

/**
 * Loads S-CSCF data from DB
 * @param x - Destination of the data.
 * @param dt - load registrar, dialogs or auth
 * @returns 1 on success or 0 on failure
 */
int bin_load_from_db(bin_data *x, data_type_t dt){
	switch(dt){
		case S_REGISTRAR:
			return bin_load_registrar_from_table(x);
		case S_DIALOGS:
			return bin_load_dialogs_from_table(x);
		case S_AUTH:
			return bin_load_auth_from_table(x);
		default:
			LOG(L_ERR,"ERR:"M_NAME":bin_load_from_db: No such information to load %d\n", dt);
			return 0;
	}
}

/**
 * Loads S-CSCF registrar from DB
 * @param x - Destination of the data
 * @returns 1 on success or 0 on failure
 */
int bin_load_registrar_from_table(bin_data *x){
	if(x){//whole hashtable dumped to db
		return bin_bulk_load_from_table(S_REGISTRAR, x);
	}
	else{//each hashtable element dumped to DB separately
		return bin_cache_load_registrar_from_table();
	}
}

/**
 * Loads S-CSCF dialogs from DB
 * @param x - Destination of the data
 * @returns 1 on success or 0 on failure
 */
int bin_load_dialogs_from_table(bin_data *x){
	if(x){//whole hashtable dumped to db
		return bin_bulk_load_from_table(S_DIALOGS, x);
	}
	else{//each hashtable element dumped to DB separately
		return bin_cache_load_dialogs_from_table();
	}
}

/**
 * Loads S-CSCF auth data from DB
 * @param x - Destination of the data
 * @returns 1 on success or 0 on failure
 */
int bin_load_auth_from_table(bin_data *x){
	if(x){//whole hashtable dumped to db
		return bin_bulk_load_from_table(S_AUTH, x);
	}
	else{//each hashtable element dumped to DB separately
		return bin_cache_load_auth_from_table();
	}
}

/**
 * Loads S-CSCF data from the snapshot table
 * @param dt - load registrar, dialogs or auth
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
	
	if((r=db_get_last_snapshot_version("snapshot", scscf_name, dt, &snapshot_version))==0){
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
	vals[0].val.str_val.s = scscf_name;
	len = strlen(scscf_name);
	vals[0].val.str_val.len = MIN(len, 256);
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val = dt;

	vals[2].type = DB_INT;
	vals[2].nul = 0;
	vals[2].val.int_val = snapshot_version;
	
	result_cols[0]="data";
	
	if (scscf_dbf.use_table(scscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_load_from_table(): Error in use_table\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	
	if (scscf_dbf.query(scscf_db, keys, ops, vals, result_cols, 3, 1, 0, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_bulk_load_from_table(): Error while querying snapshot table\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	
	if (!res) {
		lock_release(db_lock);//unlock
		return 0;
	}
	
	if(res->n==0){
		scscf_dbf.free_result(scscf_db, res);
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
	
	scscf_dbf.free_result(scscf_db, res);
	
	lock_release(db_lock);//unlock
	
	return 1;
}

/**
 * Loads S-CSCF registrar from the snapshot table.
 * Each r_public is placed on the registrar hastable here.
 * @returns 1 on success or 0 on failure
 */
int bin_cache_load_registrar_from_table(){
	int snapshot_version;
	int r, len, i=0;
	bin_data x;
	r_public *p;
	
	//lock
	lock_get(db_lock);
	
	if((r=db_get_last_snapshot_version("snapshot", scscf_name, S_REGISTRAR, &snapshot_version))==0){
		LOG(L_ERR,"ERR:"M_NAME":bin_cache_load_registrar_from_table: Error while getting snapshot version\n");
		goto error;
	}
	else if(r==-1){//empty table, nothing to load
			LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_registrar_from_table: snapshot table empty\n");
			lock_release(db_lock);//unlock
			return 1;
		}
		
	//set snapshot/step versions
	set_versions(S_REGISTRAR, snapshot_version+1, 0);
		
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
	vals[0].val.str_val.s = scscf_name;
	len = strlen(scscf_name);
	vals[0].val.str_val.len = MIN(len, 256);
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val = S_REGISTRAR;

	vals[2].type = DB_INT;
	vals[2].nul = 0;
	vals[2].val.int_val = snapshot_version;
	
	result_cols[0]="data";
	
	if (scscf_dbf.use_table(scscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_registrar_from_table(): Error in use_table\n");
		goto error;
	}
	
	if (scscf_dbf.query(scscf_db, keys, ops, vals, result_cols, 3, 1, 0, &res) < 0) {
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
		p = bin_decode_r_public(&x);
		if (!p){
			scscf_dbf.free_result(scscf_db, res);
			goto error;
		};
		LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_registrar_from_table(): Loaded r_public for <%.*s>\n",p->aor.len,p->aor.s);
		r_lock(p->hash);
		p->prev = registrar[p->hash].tail;
		p->next = 0;
		if (registrar[p->hash].tail) registrar[p->hash].tail->next = p;
		registrar[p->hash].tail = p;
		if (!registrar[p->hash].head) registrar[p->hash].head = p;
		r_unlock(p->hash);
		bin_free(&x);
	}
	
	scscf_dbf.free_result(scscf_db, res);
	lock_release(db_lock);//unlock
	return 1;
	
error:
	lock_release(db_lock);//unlock
	return 0;
}

/**
 * Loads S-CSCF dialogs from the snapshot table.
 * Each dialog is placed on the dialogs hastable here.
 * @returns 1 on success or 0 on failure
 */
int bin_cache_load_dialogs_from_table(){
	int snapshot_version;
	int r, len, i=0;
	bin_data x;
	s_dialog *dialog;
	
	//lock
	lock_get(db_lock);
	
	if((r=db_get_last_snapshot_version("snapshot", scscf_name, S_DIALOGS, &snapshot_version))==0){
		LOG(L_ERR,"ERR:"M_NAME":bin_cache_load_dialogs_from_table: Error while getting snapshot version\n");
		goto error;
	}
	else if(r==-1){//empty table, nothing to load
			LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_dialogs_from_table: snapshot table empty\n");
			lock_release(db_lock);//unlock
			return 1;
		}
		
	//set snapshot/step versions
	set_versions(S_DIALOGS, snapshot_version+1, 0);
		
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
	vals[0].val.str_val.s = scscf_name;
	len = strlen(scscf_name);
	vals[0].val.str_val.len = MIN(len, 256);
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val = S_DIALOGS;

	vals[2].type = DB_INT;
	vals[2].nul = 0;
	vals[2].val.int_val = snapshot_version;
	
	result_cols[0]="data";
	
	if (scscf_dbf.use_table(scscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_registrar_from_table(): Error in use_table\n");
		goto error;
	}
	
	if (scscf_dbf.query(scscf_db, keys, ops, vals, result_cols, 3, 1, 0, &res) < 0) {
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
		dialog = bin_decode_s_dialog(&x);
		if (!dialog){
			scscf_dbf.free_result(scscf_db, res);
			goto error;
		};
		LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_registrar_from_table(): Loaded s_dialog for <%.*s>\n",dialog->aor.len,dialog->aor.s);
		d_lock(dialog->hash);
		dialog->prev = s_dialogs[dialog->hash].tail;
		dialog->next = 0;
		if (s_dialogs[dialog->hash].tail) s_dialogs[dialog->hash].tail->next = dialog;
		s_dialogs[dialog->hash].tail = dialog;
		if (!s_dialogs[dialog->hash].head) s_dialogs[dialog->hash].head = dialog;
		d_unlock(dialog->hash);
		bin_free(&x);
	}
	
	scscf_dbf.free_result(scscf_db, res);
	lock_release(db_lock);//unlock
	return 1;
	
error:
	lock_release(db_lock);//unlock
	return 0;
}

/**
 * Loads S-CSCF auth data from the snapshot table.
 * Each auth_userdata is placed on the auth_data hastable here.
 * @returns 1 on success or 0 on failure
 */
int bin_cache_load_auth_from_table(){
	int snapshot_version;
	int r, len, i=0;
	bin_data x;
	auth_userdata *aud;
	
	//lock
	lock_get(db_lock);
	
	if((r=db_get_last_snapshot_version("snapshot", scscf_name, S_AUTH, &snapshot_version))==0){
		LOG(L_ERR,"ERR:"M_NAME":bin_cache_load_auth_from_table: Error while getting snapshot version\n");
		goto error;
	}
	else if(r==-1){//empty table, nothing to load
			LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_auth_from_table: snapshot table empty\n");
			lock_release(db_lock);//unlock
			return 1;
		}
		
	//set snapshot/step versions
	set_versions(S_AUTH, snapshot_version+1, 0);
		
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
	vals[0].val.str_val.s = scscf_name;
	len = strlen(scscf_name);
	vals[0].val.str_val.len = MIN(len, 256);
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val = S_AUTH;

	vals[2].type = DB_INT;
	vals[2].nul = 0;
	vals[2].val.int_val = snapshot_version;
	
	result_cols[0]="data";
	
	if (scscf_dbf.use_table(scscf_db, "snapshot") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_auth_from_table(): Error in use_table\n");
		goto error;
	}
	
	if (scscf_dbf.query(scscf_db, keys, ops, vals, result_cols, 3, 1, 0, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_cache_load_auth_from_table(): Error while querying snapshot table\n");
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
		aud = bin_decode_auth_userdata(&x);
		if (!aud){
			scscf_dbf.free_result(scscf_db, res);
			goto error;
		};
		LOG(L_INFO,"INFO:"M_NAME":bin_cache_load_auth_from_table(): Loaded auth_userdata for <%.*s>\n",aud->private_identity.len,aud->private_identity.s);
		auth_data_lock(aud->hash);
		aud->prev = auth_data[aud->hash].tail;
		aud->next = 0;
		if (auth_data[aud->hash].tail) auth_data[aud->hash].tail->next = aud;
		auth_data[aud->hash].tail = aud;
		if (!auth_data[aud->hash].head) auth_data[aud->hash].head = aud;
		auth_data_unlock(aud->hash);
		bin_free(&x);
	}
	
	scscf_dbf.free_result(scscf_db, res);
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
 * @param dt - registrar, dialogs or auth
 * @param version - where to load
 * @returns 1 on success, -1 if empty table, 0 on error
 */
int db_get_last_snapshot_version(char* table, char* node_id, data_type_t dt, int* version){
	db_res_t *res = NULL;
	char sql[200];
	
	sprintf(sql, "SELECT max(snapshot_version) from %s where node_id='%s' and data_type=%d", table, node_id, dt);
	
	if (scscf_dbf.raw_query(scscf_db, sql, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":db_get_last_snapshot_version(): Error while getting last snapshot version from %s\n", table);
		return 0;
	}
	
	if (!res) return 0;
	
	/*if(res->n==0) { //never happens with MAX function
		LOG(L_ERR, "ERR:"M_NAME":db_get_last_snapshot_version(): %s empty\n", table);
		scscf_dbf.free_result(scscf_db, res);
		return -1;
	}*/
	
	db_row_t* rows = RES_ROWS(res);
	db_val_t *row_vals = ROW_VALUES(rows);
	
	//MAX/MIN of an empty table will return a row of value NULL
	if(VAL_NULL(&row_vals[0])) {
		LOG(L_INFO, "INFO:"M_NAME":db_get_last_snapshot_version(): %s empty\n", table);
		scscf_dbf.free_result(scscf_db, res);
		return -1;
	}
	
	*version=row_vals[0].val.int_val;
	
	LOG(L_INFO, "INFO:"M_NAME":db_get_last_snapshot_version(): %s -> %d\n", table, *version);
	
	scscf_dbf.free_result(scscf_db, res);
	
	return 1;
}

/**
 * Sets the values of the global variables registrar/dialogs/auth_snapshot_version
 * and registrar/dialogs/auth_step_version.
 * @param dt - auth, dialogs or registrar
 * @param snapshot_version - it must continue from the last snapshot version + 1
 * @param step_version - it must continue from the last step version + 1 
 * @returns 1 on success, 0 on error
 */
int set_versions(data_type_t dt, int snapshot_version, int step_version){
	switch(dt){
		case S_REGISTRAR:
			*registrar_snapshot_version=snapshot_version;
			*registrar_step_version=step_version;
			break;
		case S_DIALOGS:
			*dialogs_snapshot_version=snapshot_version;
			*dialogs_step_version=step_version;
			break;
		case S_AUTH:
			*auth_snapshot_version=snapshot_version;
			*auth_step_version=step_version;
			break;
		default:
			LOG(L_ERR,"ERR:"M_NAME"set_versions: No such information %d\n", dt);
			return 0;
	}
	
	return 1;
}
