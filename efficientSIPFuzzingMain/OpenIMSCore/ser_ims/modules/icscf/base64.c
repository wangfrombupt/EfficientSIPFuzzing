/**
 * $Id: base64.c 2 2006-11-14 22:37:20Z vingarzan $
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
 * Interrogating-CSCF - Base 64 conversions
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#include <sys/types.h>
#include <stdio.h>
#include "string.h"
#include "stdlib.h"
#include "base64.h"

#include "mod.h"
#include "../../mem/mem.h"



/** conversion from int to base16 constant */
static char hexa[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

/**
 * Encode string from base256 to base16
 * @param src - source
 * @returns base16 representation, pkg_malloced or empty string on error
 */
str base16_encode(str src)
{
	int i,j,x1,x2;
	str dst={0,0};
	
	dst.len=src.len*2;
	dst.s=pkg_malloc(dst.len);
	if (!dst.s){
		LOG(L_ERR,"ERR:"M_NAME":bin2hexa: error allocating %d bytes\n",dst.len);
		dst.len = 0;
		return dst;
	}		
	for(i=0,j=0;i<src.len;i++){
		x1=((unsigned char)src.s[i])/16;
		x2=((unsigned char)src.s[i])%16;
		dst.s[j++]=hexa[x1];
		dst.s[j++]=hexa[x2];
   }
   dst.len=j;	
   return dst;
}

/** base16 char to int value */
#define HEX2VALUE(x) ((x)>='0'&&(x)<='9'?(x)-'0':((x)>='a'&&(x)<='f'?(x)-'a'+10:((x)>='A'&&(x)<='F'?(x)-'A'+10:0)))
/**
 * Decode string from base16 to base256
 * @param src - source
 * @returns base256 representation, pkg_malloced or empty string on error
 */
str base16_decode(str src)
{
	int i,j;
	unsigned char x1,x2;
	str dst={0,0};

	dst.len=src.len/2;
	dst.s=pkg_malloc(dst.len);
	if (!dst.s){
		LOG(L_ERR,"ERR:"M_NAME":hexa2bin: error allocating %d bytes\n",dst.len);
		dst.len = 0;
		return dst;
	}	
	for(i=0,j=0;j<dst.len;i+=2,j++){
		x1 = (unsigned char) src.s[i];
		x2 = (unsigned char) src.s[i+1];
		dst.s[j]=(unsigned char)(HEX2VALUE(x1)*16+HEX2VALUE(x2));
	}	
	return dst;
}


#define PAD_CHAR '.'
/**
 * Converts from char to value in base 64.
 * Here is the list of characters allowed in base64 encoding:
 * "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 * @param x - character to convert
 * @returns the int value 0-64 or -1 if the character is a base64 terminal ('=')
 */
static int base64_val(char x)\
{
	switch(x){
		case PAD_CHAR: return -1;
		case 'A': return 0;
		case 'B': return 1;
		case 'C': return 2;
		case 'D': return 3;
		case 'E': return 4;
		case 'F': return 5;
		case 'G': return 6;
		case 'H': return 7;
		case 'I': return 8;
		case 'J': return 9;
		case 'K': return 10;
		case 'L': return 11;
		case 'M': return 12;
		case 'N': return 13;
		case 'O': return 14;
		case 'P': return 15;
		case 'Q': return 16;
		case 'R': return 17;
		case 'S': return 18;
		case 'T': return 19;
		case 'U': return 20;
		case 'V': return 21;
		case 'W': return 22;
		case 'X': return 23;
		case 'Y': return 24;
		case 'Z': return 25;
		case 'a': return 26;
		case 'b': return 27;
		case 'c': return 28;
		case 'd': return 29;
		case 'e': return 30;
		case 'f': return 31;
		case 'g': return 32;
		case 'h': return 33;
		case 'i': return 34;
		case 'j': return 35;
		case 'k': return 36;
		case 'l': return 37;
		case 'm': return 38;
		case 'n': return 39;
		case 'o': return 40;
		case 'p': return 41;
		case 'q': return 42;
		case 'r': return 43;
		case 's': return 44;
		case 't': return 45;
		case 'u': return 46;
		case 'v': return 47;
		case 'w': return 48;
		case 'x': return 49;
		case 'y': return 50;
		case 'z': return 51;
		case '0': return 52;
		case '1': return 53;
		case '2': return 54;
		case '3': return 55;
		case '4': return 56;
		case '5': return 57;
		case '6': return 58;
		case '7': return 59;
		case '8': return 60;
		case '9': return 61;
		case '+': return 62;
		case '-': return 63;
	}
	return 0;
}

/**
 * Decode from base64 to base256.
 * @param buf - input character buffer
 * @param len - length of input buffer
 * @param newlen - int updated with the length in base 256
 * @returns a pkg_malloced char buffer with the base256 value
 */
str base64_decode( str src)
{
	int i,j,x1,x2,x3,x4;
	str dst = {0,0};
	dst.len = ( src.len * 3/4 ) + 8 ;
	dst.s = (char *)pkg_malloc( dst.len );
	if (!dst.s){
		LOG(L_ERR,"ERR:"M_NAME":base64_decode: error allocating %d bytes\n",dst.len);
		dst.len = 0;
		return dst;
	}
	
	for(i=0,j=0;i+3<src.len;i+=4){
		x1=base64_val(src.s[i]);
		x2=base64_val(src.s[i+1]);
		x3=base64_val(src.s[i+2]);
		x4=base64_val(src.s[i+3]);
		dst.s[j++]=(x1<<2) | ((x2 & 0x30)>>4);
		if (x3!=-1) dst.s[j++]=((x2 & 0x0F)<<4) | ((x3 & 0x3C)>>2);
		if (x4!=-1) dst.s[j++]=((x3 & 0x03)<<6) | (x4 & 0x3F);
	}
	if (i<src.len) {
		x1 = base64_val(src.s[i]);
		if (i+1<src.len)
			x2=base64_val(src.s[i+1]);
		else 
			x2=-1;
		if (i+2<src.len)		
			x3=base64_val(src.s[i+2]);
		else
			x3=-1;
		if(i+3<src.len)	
			x4=base64_val(src.s[i+3]);
		else x4=-1;
		if (x2!=-1) {
			dst.s[j++]=(x1<<2) | ((x2 & 0x30)>>4);
			if (x3==-1) {
				dst.s[j++]=((x2 & 0x0F)<<4) | ((x3 & 0x3C)>>2);
				if (x4==-1) {
					dst.s[j++]=((x3 & 0x03)<<6) | (x4 & 0x3F);
				}
			}
		}
			
	}
	dst.len=j;
	return dst;
}

/** conversion from int to base64 constant */
char base64[64]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";

/**
 * Convert a string from base256 to base64.
 * the output is padded with base64 terminating characters '='
 * @param buf - input character buffer
 * @param len - length of input buffer
 * @param newlen - int updated with the length in base64
 * @returns a pkg_malloced char buffer with the base64 value
 */
str base64_encode( str src )
{
	int i,k;
	int triplets,rest;
	str dst={0,0};
	char *ptr;

	triplets = src.len/3;
	rest = src.len%3;
	dst.len = ( triplets * 4 ) + 8 ;
	dst.s = (char *)pkg_malloc( dst.len );
	if (!dst.s){
		LOG(L_ERR,"ERR:"M_NAME":base64_encode: error allocating %d bytes\n",dst.len);
		dst.len = 0;
		return dst;
	}
	
	
	ptr = dst.s;
	for(i=0;i<triplets*3;i+=3){
		k = (((unsigned char) src.s[i])&0xFC)>>2;
		*ptr=base64[k];ptr++;

		k = (((unsigned char) src.s[i])&0x03)<<4;
		k |=(((unsigned char) src.s[i+1])&0xF0)>>4;
		*ptr=base64[k];ptr++;

		k = (((unsigned char) src.s[i+1])&0x0F)<<2;
		k |=(((unsigned char) src.s[i+2])&0xC0)>>6;
		*ptr=base64[k];ptr++;

		k = (((unsigned char) src.s[i+2])&0x3F);
		*ptr=base64[k];ptr++;
	}
	i=triplets*3;
	switch(rest){
		case 0:
			break;
		case 1:
			k = (((unsigned char) src.s[i])&0xFC)>>2;
			*ptr=base64[k];ptr++;

			k = (((unsigned char) src.s[i])&0x03)<<4;
			*ptr=base64[k];ptr++;

			*ptr=PAD_CHAR;ptr++;

			*ptr=PAD_CHAR;ptr++;
			break;
		case 2:
			k = (((unsigned char) src.s[i])&0xFC)>>2;
			*ptr=base64[k];ptr++;

			k = (((unsigned char) src.s[i])&0x03)<<4;
			k |=(((unsigned char) src.s[i+1])&0xF0)>>4;
			*ptr=base64[k];ptr++;

			k = (((unsigned char) src.s[i+1])&0x0F)<<2;
			*ptr=base64[k];ptr++;

			*ptr=PAD_CHAR;ptr++;
			break;
	}
//	fprintf(stderr,"base64=%.*s >> %d\n",ptr-out,out,ptr-out);
    //*ptr = 0;					
	dst.len = ptr-dst.s;
	return dst;
}
