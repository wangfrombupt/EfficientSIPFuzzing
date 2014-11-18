/*
 * $Id: bin_pcscf.c 588 2008-10-14 16:20:18Z vingarzan $
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
 * Binary codec operations extensions for the P-CSCF
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */

#include "bin_pcscf.h"


extern struct tm_binds tmb;   		/**< Structure with pointers to tm funcs 		*/

extern int r_hash_size;						/**< Size of S-CSCF registrar hash table		*/


static inline int str_shm_dup(str *dest,str *src)
{
	dest->s = shm_malloc(src->len);
	if (!dest->s){
		LOG(L_ERR,"ERR:"M_NAME":str_shm_dup: Error allocating %d bytes\n",src->len);
		dest->len=0;
		return 0;
	}
	dest->len = src->len;
	memcpy(dest->s,src->s,src->len);
	return 1;
}



/**
 * Encode an dialog into a binary form
 * @param x - binary data to append to
 * @param u - the dialog to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_p_dialog(bin_data *x,p_dialog *d)
{
	int i;
	char c;
	
	if (!bin_encode_str(x,&(d->call_id))) goto error;
	
	c = d->direction;
	if (!bin_encode_uchar(x,c)) goto error;
	
	if (!bin_encode_str(x,&(d->host))) goto error;
	if (!bin_encode_ushort(x,d->port)) goto error;
	
	c = d->transport;
	if (!bin_encode_uchar(x,c)) goto error;

	if (!bin_encode_ushort(x,d->routes_cnt)) goto error;
	for(i=0;i<d->routes_cnt;i++)
		if (!bin_encode_str(x,d->routes+i)) goto error;
	
	c = d->method;
	if (!bin_encode_uchar(x,c)) goto error;
	if (!bin_encode_str(x,&(d->method_str))) goto error;
	
	if (!bin_encode_int(x,d->first_cseq)) goto error;	
	if (!bin_encode_int(x,d->last_cseq)) goto error;	

	c = d->state;
	if (!bin_encode_uchar(x,c)) goto error;	

	if (!bin_encode_time_t(x,d->expires)) goto error;		
	if (!bin_encode_time_t(x,d->lr_session_expires)) goto error;
	if (!bin_encode_str(x,&(d->refresher))) goto error;
	if (!bin_encode_uchar(x,d->uac_supp_timer)) goto error;
	
	if (!bin_encode_uchar(x,d->is_releasing)) goto error;
	if (!bin_encode_dlg_t(x,d->dialog_c)) goto error;	
	if (!bin_encode_dlg_t(x,d->dialog_s)) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_p_dialog: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a dialog from a binary data structure
 * @param x - binary data to decode from
 * @returns the p_dialog* where the data has been decoded
 */
p_dialog* bin_decode_p_dialog(bin_data *x)
{
	p_dialog *d=0;
	int len,i;
	str s;
	char c;
	unsigned char uc;
	
	len = sizeof(p_dialog);
	d = (p_dialog*) shm_malloc(len);
	if (!d) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_p_dialog: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(d,0,len);

	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->call_id),&s)) goto error;

	if (!bin_decode_uchar(x,	&uc)) goto error;
	d->direction = uc;

	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->host),&s)) goto error;
	if (!bin_decode_ushort(x,	&d->port)) goto error;
	
	if (!bin_decode_uchar(x,	&uc)) goto error;
	d->transport = uc;

	if (!bin_decode_ushort(x,	&d->routes_cnt)) goto error;

	len = sizeof(str)*d->routes_cnt;
	d->routes = (str*) shm_malloc(len);
	if (!d) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_p_dialog: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(d->routes,0,len);	
	for(i=0;i<d->routes_cnt;i++)
		if (!bin_decode_str(x,&s)||!str_shm_dup(d->routes+i,&s)) goto error;
	
	if (!bin_decode_char(x,	&c)) goto error;
	d->method = c;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->method_str),&s)) goto error;
	
	if (!bin_decode_int(x,	&d->first_cseq)) goto error;
	if (!bin_decode_int(x,	&d->last_cseq)) goto error;

	if (!bin_decode_char(x,	&c)) goto error;
	d->state = c;
	
	if (!bin_decode_time_t(x,	&d->expires)) goto error;
	
	if (!bin_decode_time_t(x, &d->lr_session_expires)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->refresher),&s)) goto error;
	if (!bin_decode_uchar(x,&d->uac_supp_timer)) goto error;	

	if (!bin_decode_uchar(x, &d->is_releasing)) goto error;
	if (!bin_decode_dlg_t(x,&(d->dialog_c))) goto error;
	if (!bin_decode_dlg_t(x,&(d->dialog_s))) goto error;
	
	d->hash = get_p_dialog_hash(d->call_id);		
	
	return d;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_p_dialog: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (d) {
		if (d->call_id.s) shm_free(d->call_id.s);
		if (d->host.s) shm_free(d->host.s);
		if (d->routes_cnt){
			for(i=0;i<d->routes_cnt;i++)
				if (d->routes[i].s) shm_free(d->routes[i].s);
			shm_free(d->routes);
		}
		if (d->method_str.s) shm_free(d->method_str.s);
		if (d->refresher.s) shm_free(d->refresher.s);
		shm_free(d);
	}
	return 0;
}










/**
 * Encode an ipsec into a binary form
 * @param x - binary data to append to
 * @param ipsec - the r_ipsec to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_ipsec(bin_data *x,r_ipsec *ipsec)
{
	if (!ipsec){
		if (!bin_encode_char(x,0)) goto error;
		return 1;
	}
	
	if (!bin_encode_char(x,1)) goto error;

	if (!bin_encode_int(x,ipsec->spi_uc)) goto error;
	if (!bin_encode_int(x,ipsec->spi_us)) goto error;
	if (!bin_encode_int(x,ipsec->spi_pc)) goto error;
	if (!bin_encode_int(x,ipsec->spi_ps)) goto error;
	if (!bin_encode_ushort(x,ipsec->port_uc)) goto error;
	if (!bin_encode_ushort(x,ipsec->port_us)) goto error;
	
	if (!bin_encode_str(x,&(ipsec->ealg))) goto error;
	if (!bin_encode_str(x,&(ipsec->r_ealg))) goto error;
	if (!bin_encode_str(x,&(ipsec->ck))) goto error;
	if (!bin_encode_str(x,&(ipsec->alg))) goto error;
	if (!bin_encode_str(x,&(ipsec->r_alg))) goto error;
	if (!bin_encode_str(x,&(ipsec->ik))) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_ipsec: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a ipsec from a binary data structure
 * @param x - binary data to decode from
 * @param ipsec - ** to write into
 * @returns 1 on success or 0 on failure
 */
int bin_decode_ipsec(bin_data *x,r_ipsec **ipsec)
{
	int len;
	str s;
	char c;

	if (!bin_decode_char(x,	&c)) goto error;
	
	if (c==0) {
		*ipsec = 0;
		return 1;
	}
	
	len = sizeof(r_ipsec);
	*ipsec = (r_ipsec*) shm_malloc(len);
	if (!*ipsec) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_ipsec: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(*ipsec,0,len);

	if (!bin_decode_int(x,	&(*ipsec)->spi_uc)) goto error;
	if (!bin_decode_int(x,	&(*ipsec)->spi_us)) goto error;
	if (!bin_decode_int(x,	&(*ipsec)->spi_pc)) goto error;
	if (!bin_decode_int(x,	&(*ipsec)->spi_ps)) goto error;
	if (!bin_decode_ushort(x,	&(*ipsec)->port_uc)) goto error;
	if (!bin_decode_ushort(x,	&(*ipsec)->port_us)) goto error;

	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->ealg),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->r_ealg),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->ck),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->alg),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->r_alg),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->ik),&s)) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_ipsec: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (*ipsec) {
		if ((*ipsec)->ealg.s) shm_free((*ipsec)->ealg.s);
		if ((*ipsec)->r_ealg.s) shm_free((*ipsec)->r_ealg.s);
		if ((*ipsec)->ck.s) shm_free((*ipsec)->ck.s);
		if ((*ipsec)->alg.s) shm_free((*ipsec)->alg.s);
		if ((*ipsec)->r_alg.s) shm_free((*ipsec)->r_alg.s);
		if ((*ipsec)->ik.s) shm_free((*ipsec)->ik.s);
		shm_free(*ipsec);
		*ipsec = 0;
	}
	return 0;
}



/**
 * Encode an tls into a binary form
 * @param x - binary data to append to
 * @param tls - the r_tls to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_tls(bin_data *x,r_tls *tls)
{
	if (!tls){
		if (!bin_encode_char(x,0)) goto error;
		return 1;
	}
	
	if (!bin_encode_char(x,1)) goto error;

	if (!bin_encode_ushort(x,tls->port_tls)) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_tls: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a tls from a binary data structure
 * @param x - binary data to decode from
 * @param tls - ** to write into
 * @returns 1 on success or 0 on failure
 */
int bin_decode_tls(bin_data *x,r_tls **tls)
{
	int len;
	char c;

	if (!bin_decode_char(x,	&c)) goto error;
	
	if (c==0) {
		*tls = 0;
		return 1;
	}
	
	len = sizeof(r_tls);
	*tls = (r_tls*) shm_malloc(len);
	if (!*tls) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_tls: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(*tls,0,len);

	if (!bin_decode_ushort(x,	&(*tls)->port_tls)) goto error;	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_tls: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (*tls) {
		shm_free(*tls);
		*tls = 0;
	}
	return 0;
}




/**
 * Encode an security into a binary form
 * @param x - binary data to append to
 * @param s - the r_security to encode
 * @returns 1 s succcess or 0 on error
 */
int bin_encode_r_security(bin_data *x,r_security *s)
{
	if (!s){
		if (!bin_encode_char(x,0)) goto error;
		return 1;
	}
	
	if (!bin_encode_char(x,1)) goto error;

	if (!bin_encode_str(x,&(s->sec_header))) goto error;	
	if (!bin_encode_int(x,s->type)) goto error;
	switch (s->type){		
		case SEC_NONE:
			break;
		case SEC_TLS:
			if (!bin_encode_tls(x,s->data.tls)) goto error;
			break;
		case SEC_IPSEC:
			if (!bin_encode_ipsec(x,s->data.ipsec)) goto error;
			break;		
	}	
	if (!bin_encode_int(x,s->q*1000)) goto error; /* yeah yeah.... */
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_r_security: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a security from a binary data structure
 * @param x - binary data to decode from
 * @param sec - ** to write into
 * @returns 1 on success or 0 on failure
 */
int bin_decode_r_security(bin_data *x,r_security **sec)
{
	int len;
	str s;
	char c;
	int y;

	if (!bin_decode_char(x,	&c)) goto error;
	
	if (c==0) {
		*sec = 0;
		return 1;
	}
	
	len = sizeof(r_security);
	*sec = (r_security*) shm_malloc(len);
	if (!*sec) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_security: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(*sec,0,len);

	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*sec)->sec_header),&s)) goto error;
	if (!bin_decode_int(x,&y)) goto error;
	(*sec)->type = y;
	switch ((*sec)->type){
		case SEC_NONE:
			break;		
		case SEC_TLS:
			if (!bin_decode_tls(x,&((*sec)->data.tls))) goto error;		
			break;
		case SEC_IPSEC:
			if (!bin_decode_ipsec(x,&((*sec)->data.ipsec))) goto error;
			break;
	}	
	if (!bin_decode_int(x,	&y)) goto error;
	(*sec)->q = ((float)y)/1000.0;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_security: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (*sec) {
		if ((*sec)->sec_header.s) shm_free((*sec)->sec_header.s);
		switch ((*sec)->type){
			case SEC_NONE:
				break;		
			case SEC_TLS:
				if ((*sec)->data.tls) free_r_tls((*sec)->data.tls);
				break;
			case SEC_IPSEC:
				if ((*sec)->data.ipsec) free_r_ipsec((*sec)->data.ipsec); 
				break;
		}	
		shm_free(*sec);
		*sec = 0;
	}
	return 0;
}

/**
 * Encode a pinhole into a binary form
 * @param x - binary data to append to
 * @param u - the dialog to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_pinhole(bin_data *x,r_nat_dest *pinhole)
{
	if (!pinhole){
		if (!bin_encode_char(x,0)) goto error;
		return 1;
	}
	
	if (!bin_encode_char(x,1)) goto error;

	if (!bin_encode_uint(x,pinhole->nat_addr.af)) goto error;
	if (!bin_encode_uint(x,pinhole->nat_addr.len)) goto error;
	if (!bin_encode_uint(x,pinhole->nat_addr.u.addr32[0])) goto error;
	if (!bin_encode_uint(x,pinhole->nat_addr.u.addr32[1])) goto error;
	if (!bin_encode_uint(x,pinhole->nat_addr.u.addr32[2])) goto error;
	if (!bin_encode_uint(x,pinhole->nat_addr.u.addr32[3])) goto error;
	
	if (!bin_encode_ushort(x,pinhole->nat_port)) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_pinhole: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a pinhole from a binary data structure
 * @param x - binary data to decode from
 * @param pinhole - ** to write into
 * @returns 1 on success or 0 on failure
 */
int bin_decode_pinhole(bin_data *x,r_nat_dest **pinhole)
{
	int len;
	char c;
	
	if (!bin_decode_char(x,	&c)) goto error;
	
	if (c==0) {
		*pinhole = 0;
		return 1;
	}
	
	len = sizeof(r_nat_dest);
	*pinhole = (r_nat_dest*) shm_malloc(len);
	if (!*pinhole) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_pinhole: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(*pinhole,0,len);

	if (!bin_decode_uint(x,	&(*pinhole)->nat_addr.af)) goto error;
	if (!bin_decode_uint(x,	&(*pinhole)->nat_addr.len)) goto error;
	if (!bin_decode_uint(x,	&(*pinhole)->nat_addr.u.addr32[0])) goto error;
	if (!bin_decode_uint(x,	&(*pinhole)->nat_addr.u.addr32[1])) goto error;
	if (!bin_decode_uint(x,	&(*pinhole)->nat_addr.u.addr32[2])) goto error;
	if (!bin_decode_uint(x,	&(*pinhole)->nat_addr.u.addr32[3])) goto error;
	if (!bin_decode_ushort(x, &(*pinhole)->nat_port)) goto error;

	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_pinhole: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (*pinhole) {
		shm_free(*pinhole);
		*pinhole = 0;
	}
	return 0;
}



/**
 * Encode a r_public into a binary form
 * @param x - binary data to append to
 * @param p - the r_public to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_r_public(bin_data *x,r_public *p)
{
	char c;
	if (!bin_encode_str(x,&(p->aor))) goto error;
	c = p->is_default;
	if (!bin_encode_char(x,c)) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_r_public: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a r_public from a binary data structure
 * @param x - binary data to decode from
 * @returns the new *r_public or 0 on error
 */
r_public* bin_decode_r_public(bin_data *x)
{
	str s;
	int len;
	char c;
	r_public *p;

	len = sizeof(r_public);
	p = (r_public*) shm_malloc(len);
	if (!p) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_public: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(p,0,len);
	
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(p->aor),&s)) goto error;
	if (!bin_decode_char(x,	&c)) goto error;
	p->is_default = c;
	
	return p;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_public: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (p) {
		if (p->aor.s) shm_free(p->aor.s);
		shm_free(p);
	}
	return 0;
}











/**
 * Encode a r_contact into a binary form
 * @param x - binary data to append to
 * @param p - the r_contact to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_r_contact(bin_data *x,r_contact *c)
{
	int i;
	char k;
	unsigned short us;
	r_public *p=0;
	
	if (!bin_encode_str(x,&(c->host))) goto error;
	if (!bin_encode_ushort(x,c->port)) goto error;
	if (!bin_encode_char(x,c->transport)) goto error;
	
	if (!bin_encode_r_security(x,c->security_temp)) goto error;
	if (!bin_encode_r_security(x,c->security)) goto error;
	
	if (!bin_encode_str(x,&(c->uri))) goto error;
	
	k = c->reg_state;
	if (!bin_encode_char(x,k)) goto error;

	if (!bin_encode_time_t(x,c->expires)) goto error;
	
	if (!bin_encode_ushort(x,c->service_route_cnt)) goto error;
	for(i=0;i<c->service_route_cnt;i++)
		if (!bin_encode_str(x,c->service_route+i)) goto error;
		
	if (!bin_encode_pinhole(x,c->pinhole)) goto error;
	
	us=0;
	for(p=c->head;p;p=p->next)
		us++;
	if (!bin_encode_ushort(x,us)) goto error;
	for(p=c->head;p;p=p->next)
		if (!bin_encode_r_public(x,p)) goto error;	
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_r_contact: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a r_contact from a binary data structure
 * @param x - binary data to decode from
 * @returns the r_contact* where the data has been decoded
 */
r_contact* bin_decode_r_contact(bin_data *x)
{
	r_contact *c=0;
	r_public *p=0,*pn=0;
	int len,i;
	char k;
	unsigned short us;
	str st;
	
	len = sizeof(r_contact);
	c = (r_contact*) shm_malloc(len);
	if (!c) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(c,0,len);
	
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(c->host),&st)) goto error;
	if (!bin_decode_ushort(x,&c->port)) goto error;
	if (!bin_decode_char(x,&c->transport)) goto error;

	c->hash = get_contact_hash(c->host,c->port,c->transport,r_hash_size);
	
	if (!bin_decode_r_security(x,&(c->security_temp))) goto error;
	if (!bin_decode_r_security(x,&(c->security))) goto error;
	
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(c->uri),&st)) goto error;
	
	if (!bin_decode_char(x,&k)) goto error;
	c->reg_state = k;

	if (!bin_decode_time_t(x,&c->expires)) goto error;
	
	if (!bin_decode_ushort(x,	&c->service_route_cnt)) goto error;

	len = sizeof(str)*c->service_route_cnt;
	c->service_route = (str*) shm_malloc(len);
	if (!c->service_route) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(c->service_route,0,len);	
	for(i=0;i<c->service_route_cnt;i++)
		if (!bin_decode_str(x,&st)||!str_shm_dup(c->service_route+i,&st)) goto error;
	
	if (!bin_decode_pinhole(x,&(c->pinhole ))) goto error;
		
	if (!bin_decode_ushort(x,&us)) goto error;
	for(i=0;i<us;i++){
		p = bin_decode_r_public(x);
		if (!p) goto error;
		p->prev = c->tail;
		p->next = 0;
		if (c->tail) c->tail->next = p;
		c->tail = p;
		if (!c->head) c->head = p;
	}
	return c;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (c) {
		if (c->host.s) shm_free(c->host.s);		
		if (c->security_temp) free_r_security(c->security_temp);
		if (c->security) free_r_security(c->security);
		if (c->uri.s) shm_free(c->uri.s);
		if (c->pinhole) shm_free(c->pinhole);
		while(c->head){
			p = c->head;
			pn = p->next;
			free_r_public(p);
			c->head = pn;
		}
		shm_free(c);
	}
	return 0;
}




/**
 * Encode a r_subscription into a binary form
 * @param x - binary data to append to
 * @param p - the r_subscription to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_r_subscription(bin_data *x,r_subscription *s)
{
	char c;
	
	if (!bin_encode_str(x,&(s->req_uri))) goto error;
	if (!bin_encode_int(x,s->duration)) goto error;
	if (!bin_encode_time_t(x,s->expires)) goto error;
	
	c = s->attempts_left;
	if (!bin_encode_char(x,c)) goto error;

	if (!bin_encode_dlg_t(x,s->dialog)) goto error;	
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_r_subscription: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a r_subscription from a binary data structure
 * @param x - binary data to decode from
 * @returns the r_subscription* where the data has been decoded
 */
r_subscription* bin_decode_r_subscription(bin_data *x)
{
	r_subscription *s=0;
	int len;
	str st;
	char c;
	
	len = sizeof(r_subscription);
	s = (r_subscription*) shm_malloc(len);
	if (!s) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_subscription: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(s,0,len);
	
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(s->req_uri),&st)) goto error;
	if (!bin_decode_int(x,&s->duration)) goto error;
	if (!bin_decode_time_t(x,&s->expires)) goto error;	
	if (!bin_decode_char(x,&c)) goto error;
	s->attempts_left = c;
	
	if (!bin_decode_dlg_t(x,&(s->dialog))) goto error;
	
	s->hash = get_subscription_hash(s->req_uri);
	
	return s;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_subscription: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (s) {
		if (s->req_uri.s) shm_free(s->req_uri.s);
		if (s->dialog) tmb.free_dlg(s->dialog);		
		shm_free(s);
	}
	return 0;
}




