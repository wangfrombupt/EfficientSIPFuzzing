/*
 * $Id: s_persistency.c 590 2008-10-14 17:26:03Z vingarzan $
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
#include "bin_file.h"
#include "bin_db_scscf.h"

extern persistency_mode_t scscf_persistency_mode;/**< the type of persistency					*/
extern char* scscf_persistency_location;		/**< where to dump the persistency data 		*/ 


extern auth_hash_slot_t *auth_data;				/**< Authentication vector hash table 			*/
extern int auth_data_hash_size;					/**< authentication vector hash table size 		*/

extern int r_hash_size;							/**< Size of S-CSCF registrar hash table		*/
extern r_hash_slot *registrar;					/**< The S-CSCF registrar 						*/

extern int s_dialogs_hash_size;					/**< size of the dialog hash table 				*/
extern s_dialog_hash_slot *s_dialogs;			/**< the hash table								*/


/*****  DB related stuff *****/
extern int* auth_snapshot_version;
extern int* auth_step_version;
extern int* dialogs_snapshot_version;
extern int* dialogs_step_version;
extern int* registrar_snapshot_version;
extern int* registrar_step_version;


/**
 * Creates a snapshots of the authorization data and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_authdata()
{
	bin_data x={0,0,0};
	auth_userdata *aud;
	int i,k;
	time_t unique = time(0);
	FILE *f;
	
	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:			
			return 0;

		case WITH_FILES:
			f = bin_dump_to_file_create(scscf_persistency_location,"sauthdata",unique);
			if (!f) return 0;
			
			for(i=0;i<auth_data_hash_size;i++){
				if (!bin_alloc(&x,1024)) goto error;
				auth_data_lock(i);
				aud = auth_data[i].head;
				if (aud){
					while(aud){
						if (!bin_encode_auth_userdata(&x,aud)) goto error;
						aud = aud->next;
					}
					auth_data_unlock(i);
					k = bind_dump_to_file_append(f,&x);
					if (k!=x.len) {
						LOG(L_ERR,"ERR:"M_NAME":make_snapshot_authdata: error while dumping to file - only wrote %d bytes of %d \n",k,x.len);
						auth_data_unlock(i);
						bin_free(&x);
						return 0;
					} 					
				}
				else auth_data_unlock(i);
				bin_free(&x);
			}
			return bind_dump_to_file_close(f,scscf_persistency_location,"sauthdata",unique);

			break;
			
		case WITH_DATABASE_BULK:
			if (!bin_alloc(&x,1024)) goto error;
			for(i=0;i<auth_data_hash_size;i++){
				auth_data_lock(i);
				aud = auth_data[i].head;
				while(aud){
					if (!bin_encode_auth_userdata(&x,aud)) goto error;
					aud = aud->next;
				}
				auth_data_unlock(i);
			}
			return bin_dump_to_db(&x, S_AUTH);
			break;
			
		case WITH_DATABASE_CACHE:
			return bin_dump_to_db(NULL, S_AUTH);//ignore x, x is empty
			
		default:
			LOG(L_ERR,"ERR:"M_NAME":make_snapshot_authdata: Snapshot done but no such mode %d\n",scscf_persistency_mode);
			return 0;
	}	
error:
	if (x.s) bin_free(&x);	
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
	int k,max;
	FILE *f;

	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:
			k=0;

		case WITH_FILES:
			f = bin_load_from_file_open(scscf_persistency_location,"sauthdata");		
			if (!f) return 0;
			bin_alloc(&x,128*1024);
			k=bin_load_from_file_read(f,&x);
			max = x.max;
			x.max=0;
			LOG(L_INFO,"INFO:"M_NAME":load_snapshot_authdata: max %d len %d\n",x.max,x.len);
			while(x.max<x.len){
				aud = bin_decode_auth_userdata(&x);		
				if (!aud) return 0;
				LOG(L_INFO,"INFO:"M_NAME":load_snapshot_authdata: Loaded auth_userdata for <%.*s>\n",aud->private_identity.len,aud->private_identity.s);
				auth_data_lock(aud->hash);
				aud->prev = auth_data[aud->hash].tail;
				aud->next = 0;
				if (auth_data[aud->hash].tail) auth_data[aud->hash].tail->next = aud;
				auth_data[aud->hash].tail = aud;
				if (!auth_data[aud->hash].head) auth_data[aud->hash].head = aud;
				auth_data_unlock(aud->hash);
				
				memmove(x.s,x.s+x.max,x.len-x.max);
				x.len -= x.max;
				x.max = max;
				k=bin_load_from_file_read(f,&x);
				max = x.max;
				x.max = 0;				
			}
			bin_free(&x);
			bin_load_from_file_close(f);
			k = 1;
			break;
			
		case WITH_DATABASE_BULK:
			k=bin_load_from_db(&x, S_AUTH);
			x.max=0;
			LOG(L_INFO,"INFO:"M_NAME":load_snapshot_authdata: max %d len %d\n",x.max,x.len);
			while(x.max<x.len){
				aud = bin_decode_auth_userdata(&x);		
				if (!aud) return 0;
				LOG(L_INFO,"INFO:"M_NAME":load_snapshot_authdata: Loaded auth_userdata for <%.*s>\n",aud->private_identity.len,aud->private_identity.s);
				auth_data_lock(aud->hash);
				aud->prev = auth_data[aud->hash].tail;
				aud->next = 0;
				if (auth_data[aud->hash].tail) auth_data[aud->hash].tail->next = aud;
				auth_data[aud->hash].tail = aud;
				if (!auth_data[aud->hash].head) auth_data[aud->hash].head = aud;
				auth_data_unlock(aud->hash);
			}
			bin_free(&x);
			break;
			
		case WITH_DATABASE_CACHE:
			k=bin_load_from_db(NULL, S_AUTH); //ignore x, x is empty
			break;
			
		default:
			LOG(L_ERR,"ERR:"M_NAME":load_snapshot_authdata: Can't resume because no such mode %d\n",scscf_persistency_mode);
			k=0;
	}	
	if (!k) goto error;
	
	
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
	bin_data x={0,0,0};
	s_dialog *d;
	int i,k;
	time_t unique = time(0);
	FILE *f;
	
	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:			
			return 0;

		case WITH_FILES:
			f = bin_dump_to_file_create(scscf_persistency_location,"sdialogs",unique);
			if (!f) return 0;
			
			for(i=0;i<s_dialogs_hash_size;i++){
				if (!bin_alloc(&x,1024)) goto error;
				d_lock(i);
				d = s_dialogs[i].head;
				d_unlock(i);
				if (d){
					while(d){
						if (!bin_encode_s_dialog(&x,d)) goto error;
						d = d->next;
					}
					d_unlock(i);
					k = bind_dump_to_file_append(f,&x);
					if (k!=x.len) {
						LOG(L_ERR,"ERR:"M_NAME":make_snapshot_dialogs: error while dumping to file - only wrote %d bytes of %d \n",k,x.len);
						d_unlock(i);
						bin_free(&x);
						return 0;
					} 					
				}
				else d_unlock(i);
				bin_free(&x);
			}
			return bind_dump_to_file_close(f,scscf_persistency_location,"sdialogs",unique);

			break;
			
		case WITH_DATABASE_BULK:
			if (!bin_alloc(&x,1024)) goto error;
			for(i=0;i<s_dialogs_hash_size;i++){
				d_lock(i);
				d = s_dialogs[i].head;
				while(d){
					if (!bin_encode_s_dialog(&x,d)) goto error;
					d = d->next;
				}
				d_unlock(i);
			}
			return bin_dump_to_db(&x, S_DIALOGS);
			break;
			
		case WITH_DATABASE_CACHE:
			return bin_dump_to_db(NULL, S_DIALOGS);//ignore x, x is empty
			
		default:
			LOG(L_ERR,"ERR:"M_NAME":make_snapshot_dialogs: Snapshot done but no such mode %d\n",scscf_persistency_mode);
			return 0;
	}	
error:
	if (x.s) bin_free(&x);		
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
	int k,max;
	FILE *f;

	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:
			k=0;

		case WITH_FILES:
			f = bin_load_from_file_open(scscf_persistency_location,"sdialogs");		
			if (!f) return 0;
			bin_alloc(&x,128*1024);
			k=bin_load_from_file_read(f,&x);
			max = x.max;
			x.max=0;
			LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dialogs: max %d len %d\n",x.max,x.len);
			while(x.max<x.len){
				d = bin_decode_s_dialog(&x);
				if (!d) return 0;
				LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dialogs: Loaded s_dialog for <%.*s>\n",d->aor.len,d->aor.s);
				d_lock(d->hash);
				d->prev = s_dialogs[d->hash].tail;
				d->next = 0;
				if (s_dialogs[d->hash].tail) s_dialogs[d->hash].tail->next = d;
				s_dialogs[d->hash].tail = d;
				if (!s_dialogs[d->hash].head) s_dialogs[d->hash].head = d;
				d_unlock(d->hash);
				
				memmove(x.s,x.s+x.max,x.len-x.max);
				x.len -= x.max;
				x.max = max;
				k=bin_load_from_file_read(f,&x);
				max = x.max;
				x.max = 0;				
			}
			bin_free(&x);
			bin_load_from_file_close(f);
			k = 1;
			break;
			
		case WITH_DATABASE_BULK:
			k=bin_load_from_db(&x, S_DIALOGS);
			x.max=0;
			LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dialogs: max %d len %d\n",x.max,x.len);
			while(x.max<x.len){
				d = bin_decode_s_dialog(&x);
				if (!d) return 0;
				LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dialogs: Loaded s_dialog for <%.*s>\n",d->aor.len,d->aor.s);
				d_lock(d->hash);
				d->prev = s_dialogs[d->hash].tail;
				d->next = 0;
				if (s_dialogs[d->hash].tail) s_dialogs[d->hash].tail->next = d;
				s_dialogs[d->hash].tail = d;
				if (!s_dialogs[d->hash].head) s_dialogs[d->hash].head = d;
				d_unlock(d->hash);
			}
			bin_free(&x);
			break;
			
		case WITH_DATABASE_CACHE:
			k=bin_load_from_db(NULL, S_DIALOGS); //ignore x, x is empty
			break;
			
		default:
			LOG(L_ERR,"ERR:"M_NAME":load_snapshot_dialogs: Can't resume because no such mode %d\n",scscf_persistency_mode);
			k=0;
	}	
	if (!k) goto error;
	
	
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
	bin_data x={0,0,0};
	r_public *p;
	int i,k;
	time_t unique = time(0);
	FILE *f;
	
	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:			
			return 0;

		case WITH_FILES:
			f = bin_dump_to_file_create(scscf_persistency_location,"sregistrar",unique);
			if (!f) return 0;
			
			for(i=0;i<r_hash_size;i++){
				if (!bin_alloc(&x,1024)) goto error;		
				r_lock(i);
				p = registrar[i].head;
				if (p){
					while(p){
						if (!bin_encode_r_public(&x,p)) goto error;
						p = p->next;
					}
					r_unlock(i);
					k = bind_dump_to_file_append(f,&x);
					if (k!=x.len) {
						LOG(L_ERR,"ERR:"M_NAME":make_snapshot_registrar: error while dumping to file - only wrote %d bytes of %d \n",k,x.len);
						r_unlock(i);
						bin_free(&x);
						return 0;
					} 					
				}
				else r_unlock(i);				
				bin_free(&x);
			}
			return bind_dump_to_file_close(f,scscf_persistency_location,"sregistrar",unique);

			break;
			
		case WITH_DATABASE_BULK:
			if (!bin_alloc(&x,1024)) goto error;
			for(i=0;i<r_hash_size;i++){
				r_lock(i);
				p = registrar[i].head;
				while(p){
					if (!bin_encode_r_public(&x,p)) goto error;
					p = p->next;
				}
				r_unlock(i);
			}
			return bin_dump_to_db(&x, S_REGISTRAR);
			break;
			
		case WITH_DATABASE_CACHE:
			return bin_dump_to_db(NULL, S_REGISTRAR);//ignore x, x is empty
			
		default:
			LOG(L_ERR,"ERR:"M_NAME":make_snapshot_registrar: Snapshot done but no such mode %d\n",scscf_persistency_mode);
			return 0;
	}	
error:
	if (x.s) bin_free(&x);		
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
	int k,max;
	FILE *f;

	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:
			k=0;

		case WITH_FILES:
			f = bin_load_from_file_open(scscf_persistency_location,"sregistrar");		
			if (!f) return 0;
			bin_alloc(&x,128*1024);
			k=bin_load_from_file_read(f,&x);
			max = x.max;
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
				
				memmove(x.s,x.s+x.max,x.len-x.max);
				x.len -= x.max;
				x.max = max;
				k=bin_load_from_file_read(f,&x);
				max = x.max;
				x.max = 0;				
			}
			bin_free(&x);
			bin_load_from_file_close(f);
			k = 1;
			break;
			
		case WITH_DATABASE_BULK:
			k=bin_load_from_db(&x, S_REGISTRAR);
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
			break;
			
		case WITH_DATABASE_CACHE:
			k=bin_load_from_db(NULL, S_REGISTRAR); //ignore x, x is empty
			break;
			
		default:
			LOG(L_ERR,"ERR:"M_NAME":load_snapshot_registrar: Can't resume because no such mode %d\n",scscf_persistency_mode);
			k=0;
	}	
	if (!k) goto error;
	
	
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



//
//	
///**
// * Loads S-CSCF data from files or DB
// * @param x - destination of the loaded data. Empty if scscf_persistency_mode==WITH_DATABASE_CACHE.
// * In this case, each row of the table is loaded and placed on the hashtable now, instead of to be
// * passed to load_snapshot_...() function, which does this work when the whole hashtable was dumped
// * to only one row of the table.
// * @param location -
// * @param prepend_fname -
// * @param dt - load registrar, dialogs or auth
// * @returns 1 on success or 0 on failure
// */
//int s_load(bin_data *x, char* location, char* prepend_fname, data_type_t dt){
//	switch (scscf_persistency_mode){
//		case NO_PERSISTENCY:
//			LOG(L_ERR,"ERR:"M_NAME":s_load: Persistency support was disabled\n");
//			return 0;
//		case WITH_FILES:
//			return bin_load_from_file(x,location,prepend_fname);		
//		case WITH_DATABASE_BULK:
//			return bin_load_from_db(x, dt);
//		case WITH_DATABASE_CACHE:
//			return bin_load_from_db(NULL, dt); //ignore x, x is empty
//		default:
//			LOG(L_ERR,"ERR:"M_NAME":s_load: Can't resume because no such mode %d\n",scscf_persistency_mode);
//			return 0;
//	}
//}
//
