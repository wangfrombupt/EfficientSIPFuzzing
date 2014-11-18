/*
 * $Id: bin.h 588 2008-10-14 16:20:18Z vingarzan $
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

The binary form of the subscription is a vector of bytes and has the same type as
str from SER.

Basic datatype representations:

- Each integer is written as litter endian on a specific width(1->4 bytes)
        int(k,2):
                k & 0x00ff
                k & 0xff00 >>8


- Each str is written as:
        str(s):
                int(s.len,2)
                s.s[0]
                s.s[1]
                .
                .
                .
                s.s[len-1]

- Each regex is written as:
        regex(r):
                int(sizeof(r),2)
                &r[0]
                &r[1]
                .
                .
                .
                &r[len-1]

- Each pointer is a 4 byte offset into the binary form.
        ptr(p)
                int(p,4)

- Array of pointers
        array(p,len)
                int(len,4)
                int(p[1],4)                                     - p[0] can be computed = &array + 4*(len+1)
                int(p[2],4)
                .
                .
                int(p[len],4)                           - the ptr of the next byte after the array

 *
 */



#ifndef _BIN_H
#define _BIN_H

#include "mod.h"
#include "../../mem/mem.h"
#include "../dialog/dlg_mod.h"


typedef struct _bin_data {
	char* s; /*string*/
	int len; /*string len*/
	int max; /*allocated size of the buffer s*/ 
} bin_data;


/*
 *		Binary encoding functions
 */
/* memory allocation and initialization macros */
#define BIN_ALLOC_METHOD    pkg_malloc
#define BIN_REALLOC_METHOD  pkg_realloc
#define BIN_FREE_METHOD     pkg_free

inline int bin_alloc(bin_data *x, int max_len);
inline int bin_realloc(bin_data *x, int delta);
inline int bin_expand(bin_data *x, int delta);
inline void bin_free(bin_data *x);
inline void bin_print(bin_data *x);

inline int bin_encode_char(bin_data *x,char k);
inline int bin_decode_char(bin_data *x,char *c);

inline int bin_encode_uchar(bin_data *x,unsigned char k); 
inline int bin_decode_uchar(bin_data *x,unsigned char *c);

inline int bin_encode_short(bin_data *x,short k);
inline int bin_decode_short(bin_data *x,short *c);

inline int bin_encode_ushort(bin_data *x,unsigned short k); 
inline int bin_decode_ushort(bin_data *x,unsigned short *c);

inline int bin_encode_int(bin_data *x,int k);
inline int bin_decode_int(bin_data *x,int *c);

inline int bin_encode_uint(bin_data *x,unsigned int k); 
inline int bin_decode_uint(bin_data *x,unsigned int *c);

inline int bin_encode_time_t(bin_data *x,time_t k);
inline int bin_decode_time_t(bin_data *x,time_t *c);

inline int bin_encode_str(bin_data *x,str *s);
inline int bin_decode_str(bin_data *x,str *s);


int bin_encode_dlg_t(bin_data *x,dlg_t *d);
int bin_decode_dlg_t(bin_data *x,dlg_t **d);


typedef enum {
	NO_PERSISTENCY=0,
	WITH_FILES=1,
	WITH_DATABASE_BULK=2,
	WITH_DATABASE_CACHE=3
} persistency_mode_t;



#endif
