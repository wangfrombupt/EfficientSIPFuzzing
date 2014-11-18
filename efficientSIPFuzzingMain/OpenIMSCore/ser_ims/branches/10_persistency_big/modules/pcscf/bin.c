/*
 * $Id: bin.c 588 2008-10-14 16:20:18Z vingarzan $
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
 * Binary codec operations
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */


#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

#include "bin.h"


/** 
 * Whether to print debug message while encoding/decoding 
 */
#define BIN_DEBUG 0

/** 
 * Whether to do sanity checks on the available data when decoding
 * If you are crazy about start-up performance you can disable this.
 * However, this is very useful for detecting broken snapshots
 */
#define BIN_DECODE_CHECKS 1

inline int bin_alloc(bin_data *x, int max_len)
{                                
	x->s = (char*)BIN_ALLOC_METHOD(max_len);     
	if (!x->s){
		LOG(L_ERR,"ERR:"M_NAME":bin_alloc: Error allocating %d bytes.\n",max_len);
		x->len=0;
		x->max=0;
		return 0;
	}
    x->len=0;
    x->max=max_len;
	return 1;
}

inline int bin_realloc(bin_data *x, int delta)
{
#if BIN_DEBUG
	LOG(L_INFO,"INFO:"M_NAME":bin_realloc: realloc %p from %d to + %d\n",x->s,x->max,delta);
#endif	
	x->s=BIN_REALLOC_METHOD(x->s,x->max + delta);    
	if (x->s==NULL){                             
		LOG(L_ERR,"ERR:"M_NAME":bin_realloc: No more memory to expand %d with %d  \n",x->max,delta);
		return 0;
	}
	x->max += delta;
	return 1;
}

inline int bin_expand(bin_data *x, int delta)
{
	if (x->max-x->len>=delta) return 1;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_realloc: realloc %p from %d to + %d\n",x->s,x->max,delta);
#endif	
	x->s=BIN_REALLOC_METHOD(x->s,x->max + delta);    
	if (x->s==NULL){                             
		LOG(L_ERR,"ERR:"M_NAME":bin_realloc: No more memory to expand %d with %d  \n",x->max,delta);
		return 0;
	}
	x->max += delta;
	return 1;
}

inline void bin_free(bin_data *x)
{
	BIN_FREE_METHOD(x->s);
	x->s=0;x->len=0;x->max=0;
}

/**
 *	simple print function 
 */
inline void bin_print(bin_data *x)
{
	int i,j,w=16;
	char c;
	fprintf(stderr,"----------------------------------\nBinary form %d (max %d) bytes:\n",x->len,x->max);
	for(i=0;i<x->len;i+=w){
		fprintf(stderr,"%04X> ",i);
		for(j=0;j<w;j++){
			if (i+j<x->len) fprintf(stderr,"%02X ",(unsigned char)x->s[i+j]);
			else fprintf(stderr,"   ");
		}
		printf("\t");
		for(j=0;j<w;j++)if (i+j<x->len){
			if (x->s[i+j]>32) c=x->s[i+j];
			else c = '.';
			fprintf(stderr,"%c",c);
		}else fprintf(stderr," ");
		fprintf(stderr,"\n");
	}
	fprintf(stderr,"\n---------------------------------\n");
}







/* basic data type reprezentation functions */




/**
 *	Append a char of 1 byte 
 */
inline int bin_encode_char(bin_data *x,char k) 
{ 
	if (!bin_expand(x,1)) return 0;
	x->s[x->len++]= k; 
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_encode_char: [%d]:[%.02x] new len %04x\n",k,x->s[x->len-1],x->len);
#endif
	return 1;   
}
/**
 *	Decode of 1 char
 */
inline int bin_decode_char(bin_data *x,char *c)
{
#if BIN_DECODE_CHECKS
	if (x->max+1 > x->len) return 0;
#endif	
	*c = x->s[x->max];
	x->max += 1;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_char: [%d] new pos %04x\n",*c,x->max);
#endif
	return 1;
}




/**
 *	Append an unsigned char of 1 byte 
 */
inline int bin_encode_uchar(bin_data *x,unsigned char k) 
{ 
	if (!bin_expand(x,1)) return 0;
	x->s[x->len++]= k; 
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_encode_uchar: [%u]:[%.02x] new len %04x\n",k,x->s[x->len-1],x->len);
#endif
	return 1;   
}
/**
 *	Decode of 1 unsigned char
 */
inline int bin_decode_uchar(bin_data *x,unsigned char *c)
{
#if BIN_DECODE_CHECKS
	if (x->max+1 > x->len) return 0;
#endif	
	*c = x->s[x->max];
	x->max += 1;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_uchar: [%u] new pos %04x\n",*c,x->max);
#endif
	return 1;
}







/**
 *	Append the a short  
 */
inline int bin_encode_short(bin_data *x,short k) 
{ 
	if (!bin_expand(x,2)) return 0;
	x->s[x->len++]=k & 0x00FF;    
	x->s[x->len++]=(k & 0xFF00) >> 8;   
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_encode_short: [%d]:[%.02x %.02x] new len %04x\n",k,x->s[x->len-2],x->s[x->len-1],x->len);
#endif
	return 1;   
}
/**
 *	Decode of a short
 */
inline int bin_decode_short(bin_data *x,short *v)
{
#if BIN_DECODE_CHECKS
	if (x->max+2 > x->len) return 0;
#endif
	*v =	(unsigned char)x->s[x->max  ]    |
	 		(unsigned char)x->s[x->max+1]<<8;
	x->max += 2;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_short: [%d] new pos %04x\n",*v,x->max);
#endif
	return 1;
}


/**
 *	Append the an unsigned short  
 */
inline int bin_encode_ushort(bin_data *x,unsigned short k) 
{ 
	if (!bin_expand(x,2)) return 0;
	x->s[x->len++]=k & 0x00FF;    
	x->s[x->len++]=(k & 0xFF00) >> 8;   
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_encode_ushort: [%u]:[%.02x %.02x] new len %04x\n",k,x->s[x->len-2],x->s[x->len-1],x->len);
#endif
	return 1;   
}
/**
 *	Decode of a short
 */
inline int bin_decode_ushort(bin_data *x,unsigned short *v)
{
#if BIN_DECODE_CHECKS
	if (x->max+2 > x->len) return 0;
#endif
	*v =	(unsigned char)x->s[x->max  ]    |
	 		(unsigned char)x->s[x->max+1]<<8;
	x->max += 2;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_ushort: [%u] new pos %04x\n",*v,x->max);
#endif
	return 1;
}


/**
 *	Append an integer
 */
inline int bin_encode_int(bin_data *x,int k) 
{ 
	int len = sizeof(int),i;
	if (!bin_expand(x,len)) return 0;
	for(i=0;i<len;i++){
		x->s[x->len++]= k & 0xFF;
		k = k>>8;          
	}
#if BIN_DEBUG		    
	switch(len){
		case 4:
			LOG(L_INFO,"INFO:"M_NAME":bin_encode_int: [%d]:[%.02x %.02x %.02x %.02x] new len %04x\n",k,
				x->s[x->len-4],x->s[x->len-3],x->s[x->len-2],x->s[x->len-1],x->len);
			break;
		case 8:
			LOG(L_INFO,"INFO:"M_NAME":bin_encode_int: [%d]:[%.02x %.02x %.02x %.02x%.02x %.02x %.02x %.02x] new len %04x\n",k,
				x->s[x->len-8],x->s[x->len-7],x->s[x->len-6],x->s[x->len-5],
				x->s[x->len-4],x->s[x->len-3],x->s[x->len-2],x->s[x->len-1],
				x->len);
			break;
	}
#endif		
	return 1;   
}
/**
 *	Decode an integer
 */
inline int bin_decode_int(bin_data *x,int *v)
{
	int len = sizeof(int),i;
#if BIN_DECODE_CHECKS
	if (x->max+len > x->len) return 0;
#endif
	*v = 0;
	for(i=0;i<len;i++)
		*v =  *v | ((unsigned char)x->s[x->max++] <<(8*i));
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_int: [%d] new pos %04x\n",*v,x->max);
#endif
	return 1;
}



/**
 *	Append an unsigned integer
 */
inline int bin_encode_uint(bin_data *x,unsigned int k) 
{ 
	int len = sizeof(unsigned int),i;
	if (!bin_expand(x,len)) return 0;
	for(i=0;i<len;i++){
		x->s[x->len++]= k & 0xFF;
		k = k>>8;          
	}
#if BIN_DEBUG		    
	switch(len){
		case 4:
			LOG(L_INFO,"INFO:"M_NAME":bin_encode_uint: [%u]:[%.02x %.02x %.02x %.02x] new len %04x\n",k,
				x->s[x->len-4],x->s[x->len-3],x->s[x->len-2],x->s[x->len-1],x->len);
			break;
		case 8:
			LOG(L_INFO,"INFO:"M_NAME":bin_encode_uint: [%u]:[%.02x %.02x %.02x %.02x%.02x %.02x %.02x %.02x] new len %04x\n",k,
				x->s[x->len-8],x->s[x->len-7],x->s[x->len-6],x->s[x->len-5],
				x->s[x->len-4],x->s[x->len-3],x->s[x->len-2],x->s[x->len-1],
				x->len);
			break;
	}
#endif		
	return 1;   
}
/**
 *	Decode an unsigned integer
 */
inline int bin_decode_uint(bin_data *x,unsigned int *v)
{
	int len = sizeof(unsigned int),i;
#if BIN_DECODE_CHECKS
	if (x->max+len > x->len) return 0;
#endif
	*v = 0;
	for(i=0;i<len;i++)
		*v =  *v | ((unsigned char)x->s[x->max++] <<(8*i));
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_uint: [%u] new pos %04x\n",*v,x->max);
#endif
	return 1;
}

/**
 *	Append a time_t structure
 */
inline int bin_encode_time_t(bin_data *x,time_t k) 
{ 
	int len = sizeof(time_t),i;
	if (!bin_expand(x,len)) return 0;
	for(i=0;i<len;i++){
		x->s[x->len++]= k & 0xFF;
		k = k>>8;          
	}
#if BIN_DEBUG		    
	switch(len){
		case 4:
			LOG(L_INFO,"INFO:"M_NAME":bin_encode_time_t: [%u]:[%.02x %.02x %.02x %.02x] new len %04x\n",(unsigned int)k,
				x->s[x->len-4],x->s[x->len-3],x->s[x->len-2],x->s[x->len-1],x->len);
			break;
		case 8:
			LOG(L_INFO,"INFO:"M_NAME":bin_encode_time_t: [%u]:[%.02x %.02x %.02x %.02x%.02x %.02x %.02x %.02x] new len %04x\n",(unsigned int)k,
				x->s[x->len-8],x->s[x->len-7],x->s[x->len-6],x->s[x->len-5],
				x->s[x->len-4],x->s[x->len-3],x->s[x->len-2],x->s[x->len-1],
				x->len);
			break;
	}
#endif		
	return 1;   
}
/**
 *	Decode an unsigned integer
 */
inline int bin_decode_time_t(bin_data *x,time_t *v)
{
	int len = sizeof(time_t),i;
#if BIN_DECODE_CHECKS
	if (x->max+len > x->len) return 0;
#endif
	*v = 0;
	for(i=0;i<len;i++)
		*v =  *v | ((unsigned char)x->s[x->max++] <<(8*i));
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_time_t: [%u] new pos %04x\n",(unsigned int) *v,x->max);
#endif
	return 1;
}


/**
 *	Append a string 
 */
inline int bin_encode_str(bin_data *x,str *s) 
{ 
	if (!bin_expand(x,2+s->len)) return 0;
	if (s->len>65535) 
		LOG(L_ERR,"ERROR:"M_NAME":bin_encode_str: Possible loss of characters in encoding (string > 65535bytes) %d bytes \n",s->len);
	x->s[x->len++]=s->len & 0x000000FF;
	x->s[x->len++]=(s->len & 0x0000FF00)>>8;
	memcpy(x->s+x->len,s->s,s->len);
	x->len+=s->len;
#if BIN_DEBUG		
	LOG(L_INFO,"INFO:"M_NAME":bin_encode_str : [%d]:[%.02x %.02x]:[%.*s] new len %04x\n",s->len,
		x->s[x->len-s->len-2],x->s[x->len-s->len-1],s->len,s->s,x->len);
#endif		
	return 1;   
}
/**
 *	Decode of a str string
 */
inline int bin_decode_str(bin_data *x,str *s)
{
#if BIN_DECODE_CHECKS
	if (x->max+2 > x->len) return 0;
#endif
	s->len = (unsigned char)x->s[x->max  ]    |
	 		(unsigned char)x->s[x->max+1]<<8;
	x->max +=2;
	if (x->max+s->len>x->len) return 0;
	s->s = x->s + x->max;
	x->max += s->len;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_str : [%d]:[%.*s] new pos %04x\n",s->len,s->len,s->s,x->max);
#endif
	return 1;
}




/* complex data types */


extern dlg_func_t dialogb;							/**< Structure with pointers to dialog funcs			*/


/**
 * Encode a dlg_t into a binary form
 * @param x - binary data to append to
 * @param d - the dlg_t to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_dlg_t(bin_data *x,dlg_t *d)
{
	str s={0,0};
	if (d){
		if (dialogb.dlg2str(d,&s)!=0) goto error;
	}
	if (!bin_encode_str(x,&s)) goto error;
	str_free_content(&s);
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_dlg_t: Error while encoding.\n");
	if (s.s) str_free_content(&s);
	return 0;		
}

/**
 *	Decode a dlg_t from a binary data structure
 * @param x - binary data to decode from
 * @param dlg - the dlg_t ** to write to
 * @returns 1 on success or 0 on failure
 */
int bin_decode_dlg_t(bin_data *x,dlg_t **d)
{
	int len;
	str s;
	
	if (!bin_decode_str(x,&s)) goto error;

	if (!s.len) {
		*d = 0;
		return 1;
	}
	
	len = sizeof(dlg_t);
	*d = (dlg_t*) shm_malloc(len);
	if (!*d) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_dlg_t: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(*d,0,len);
	if (dialogb.str2dlg(&s,*d)!=0) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_dlg_t: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (*d) {
		shm_free(*d);
	}
	return 0;
}




/* end of bin library functions */
