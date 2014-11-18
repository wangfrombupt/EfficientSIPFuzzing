/**$Id: thig_ims_enc.c 257 2007-04-26 12:44:54Z vingarzan $ Author Florin Dinu
 *
 * Copyright (C) 2006 FhG Fokus
 *
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
 * I-CSCF Module - THIG Operations 
 * \author Florin Dinu fdi -at- fokus dot fraunhofer dot de
 * 		
 */

#include "thig_ims_enc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../mem/mem.h"
#include "base64.h"

/** Using base64 encoding reduces the length of the encrypted strings.*/
#define USE_BASE64 1 

keyInstance ki;
cipherInstance ci;


DWORD Here(DWORD x){
	unsigned int mask=~0U;

	return (* (((DWORD *)&x)-1)) & mask;
}

int keySize		=256;		/**< key size*/
int mode 		=MODE_CFB1; /**< Twofish mode*/
int byteCnt 	=BLOCK_SIZE/8;	

/**
 * prints a message and the contents of the str structure
 * 
 * @param text - some text
 * @param x    - some string
 * 
 */
void printstr(char* text,str x)
{
	int i;
	if(text!=NULL)
		printf("%s 0x",text);
	for(i=0;i<x.len;i++)
		printf("%.02x", (unsigned char)x.s[i]);
	printf("\n");
}

/**
 * Initializes the structures that hold the keys and the cipher
 * 
 * @param ki - the structure where the key information is to be stored
 * @param ci - the structure where the cipher information is to be stored
 * @returns 1
 */
int thig_key_and_cipher_init(keyInstance *ki , cipherInstance *ci){
	int i;
	BYTE iv[BLOCK_SIZE/8];
	if (makeKey(ki,DIR_ENCRYPT,keySize,NULL) != TRUE)
		return 1;				/* 'dummy' setup for a 128-bit key */
	if (cipherInit(ci,mode,NULL) != TRUE)
		return 1;				/* 'dummy' setup for cipher */
	
	/* choose a key */
	for (i=0;i<keySize/32;i++)	
		(*ki).key32[i]=0x10003 * rand();
	reKey(ki);					/* run the key schedule */
	
	/* set the initialization vector */
	for (i=0;i<sizeof(iv);i++)
			iv[i]=(BYTE) rand();
	memcpy((*ci).iv32,iv,sizeof((*ci).iv32));	/* copy the IV to ci */
	return 1;
}





/**
 * Encrypts a string (str struct).
 *
 * @param src - the string that needs to be encrypted.Must NOT contain \0
 * @returns - the encrypted string, allocated in this function.
 */
str thig_encrypt(str src)
{
	int padding;
	str my_text={0,0},enc_text={0,0};
	cipherInstance ci2 = ci;
	str encoded={0,0};
	
	padding = (src.len%byteCnt)==0?0:byteCnt-(src.len%byteCnt);
	LOG(L_ERR,"DBG:"M_NAME":encrypt:String has length %d so needs padding %d\n",src.len,padding);

	my_text.len = src.len+padding;
	my_text.s = pkg_malloc(my_text.len);
	if (!my_text.s){
		LOG(L_ERR,"ERR:"M_NAME":encrypt: error allocating %d bytes\n",my_text.len);
		goto error;
	}	
	memcpy(my_text.s,src.s,src.len);
	memset(my_text.s+src.len,0,my_text.len-src.len);
	
	enc_text.s = pkg_malloc(my_text.len);
	if (!enc_text.s){
		LOG(L_ERR,"ERR:"M_NAME":encrypt: error allocating %d bytes\n",my_text.len);
		goto error;		
	}
	enc_text.len = my_text.len;

	printstr("String bef :",my_text);
	if (blockEncrypt(&ci2,&ki,(unsigned char*)my_text.s,my_text.len*8,(unsigned char*)enc_text.s) != my_text.len*8){
		LOG(L_ERR,"DBG:"M_NAME":encrypt: Error in encryption phase\n");
		goto error;
	}
	printstr("String aft :",enc_text);

#ifdef USE_BASE64
	encoded = base64_encode(enc_text);
#else
	encoded = base16_encode(enc_text);
#endif
	
	if (my_text.s) pkg_free(my_text.s);
	if (enc_text.s) pkg_free(enc_text.s);
	return encoded;	
error:
	if (my_text.s) pkg_free(my_text.s);
	if (enc_text.s) pkg_free(enc_text.s);
	enc_text.s = 0;enc_text.len=0;
	return enc_text;		
}

/**
 * Decrypts a string(str struct).
 * 
 * @param encoded - the string that needs to be decrypted
 * @returns - the decoded string.Memory allocation takes place in this
 * 				function.
 */
str thig_decrypt(str encoded)
{
	str src;
	str my_text={0,0},dec_text={0,0};
	cipherInstance ci2 = ci;

#ifdef USE_BASE64
	src = base64_decode(encoded);
#else
	src = base16_decode(encoded);
#endif
	if (!src.len) return dec_text;

	my_text = src;
	dec_text.s = pkg_malloc(my_text.len);
	if (!dec_text.s){
		LOG(L_ERR,"ERR:"M_NAME":decrypt: error allocating %d bytes\n",my_text.len);
		goto error;		
	}
	dec_text.len = my_text.len;
	
	printstr("String bef :",my_text);

	if (blockDecrypt(&ci2,&ki,(unsigned char*)my_text.s,my_text.len*8,(unsigned char*)dec_text.s) != my_text.len*8){
		LOG(L_ERR,"DBG:"M_NAME":decrypt: Error in encryption phase\n");
		goto error;
	}
	while(dec_text.s[dec_text.len-1]==0 && dec_text.len>0)
		dec_text.len--;
	printstr("String aft :",dec_text);
	if (src.s) pkg_free(src.s);
	return dec_text;
error:
	if (src.s) pkg_free(src.s);
	if (dec_text.s) pkg_free(dec_text.s);
	dec_text.s = 0;dec_text.len=0;
	return dec_text;	
}


#ifdef THIG_IMS_ENC_TEST

int main (){
 	
	

	BYTE  plainText[MAX_BLK_CNT*(BLOCK_SIZE/8)];
	BYTE *tmp;
	BYTE *ret_tmp;
	int i = 0;
	
	printf("*********************************************************************\n");
	printf("*********************************************************************\n");
	srand((unsigned) time(NULL));
	key_and_cipher_init(&ki,&ci);
	
	printf("byteCnt: %d\n",byteCnt);
	printf("-----------------------------------------------------------------------------\n");
	for (i=0;i<25;i++)		
		plainText[i]=(BYTE)((rand()%94)+32);

	tmp=enc(plainText,25);
	ret_tmp=dec(tmp,25);
	free(tmp);
	free(ret_tmp);
	/*printf("\n\n");printf("-----------------------------------------------------------------------------\n");
	
	int new_byteCnt=28;
	for (i=0;i<new_byteCnt;i++)		
		plainText[i]=(BYTE)('a');

	tmp=enc(plainText,new_byteCnt);
	ret_tmp=dec(tmp);
	free(tmp);
	free(ret_tmp);
*/
	return 1;
}	

#endif
