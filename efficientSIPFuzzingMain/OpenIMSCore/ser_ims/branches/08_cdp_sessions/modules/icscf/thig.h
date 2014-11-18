/**$Id: thig.h 2 2006-11-14 22:37:20Z vingarzan $ Author Florin Dinu
 *
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
 * 
 *
 * I-CSCF Module - THIG Operations 
 * 
 * Copyright (C) 2006 FhG Fokus
 * 		
 */
 
/**\page THIG Topology Hiding
 *  \b Module \b Documentation
 * 
 * [\subpage thig_overview]
 * [\subpage thig_code]
 * [\subpage thig_config]
 * [\subpage thig_example]
 * 
 * \section thig_overview Overview
 * As SIP is a text based protocol all information is sent as plain text.
 * Some headers contain sensitive information such as the addresses of
 * entities from the home network or the number of such entities.
 * Therefore, an operator may choose to hide this information in order to
 * minimize the risk of attacks and to be able to perform changes to the internal 
 * network at will.
 * 
 * The headers in question here are : Via, Path, Record-Route and Service-Route.
 * 
 * The solution is to encrypt the headers as they leave the network and
 * to decrypt the messages when they enter the internal network.
 *
 * The Twofish encryption algorithm has been chosen for this purpose because of
 * its speed and flexibility. However, if you consider it necessary you can 
 * use some other algorithm as the interface of the application allows such 
 * changes to be performed with relative ease.
 * 
 * \section thig_code Code Structure
 * 
 * The file thig_ims_enc.c provides the following functionality:
 * 
 * 1.Initialization of the data structures needed by Twofish\n
 * 2.Ability to encrypt a SER string (also adds padding in order to 
 * perform encryption, and performs BASE64 encoding in order to use 
 * only characters that do not confuse the SIP parsers).\n
 * 3.Ability to decrypt a SER string (this removes the padding 
 * and decodes the previously BASE64 encoded string\n
 * 
 * Note: 
 * You may choose not to use the BASE64 encoding as this adds to the 
 * length of the SIP message.However be aware that doing so will result
 * in incorrect parsing of the message as the ouput from the encryption 
 * algorithm can contain any character, including the ones used by the
 * parser to delimit the parts of the message.If you choose to modify 
 * the parser accordingly, then the BASE64 encoding may indeed be
 * superfluous.
 * 
 * If you choose to use another algorithm here you can call its own 
 * block encryption functions.
 * 
 * The file thig.c contains functions necessary to add the ICSCFs address
 * to the SIP headers and also the functions that you can call to encrypt
 * or decrypt headers.
 *   
 * 
 * \section thig_config Configuration and Usage
 * You may choose to tweak the following parameters :
 * 
 * in thig_ims_enc.c: \n
 * keySize - the Twofish key length.256 bits it's the biggest value and also provides the
 * 			  most security.If you choose to use a shorter key this can improve speed
 *            but also provide less security.\n
 * mode    - the mode in which Twofish operates.MODE_EBC is not recommended as it yields
 *  		  obvious patterns in the encrypted strings and makes them prone to attacks 
 * 			  or decyphering.
 * 
 * in  thig_aes.h: \n
 * BLOCK_SIZE - number of bits per block (Twofish encrypts blocks of data) \n
 * MAX_BLK_CNT - max nr blocks per call in Twofish\n
 * 
 * You should be aware that a big value of the BLOCK_SIZE can lead to unnecessary
 * padding being added to a SIPMessage (for Example if the BLOCK_SIZE converted into
 * bytes is 16 a block of 2bytes will have to be padded with 14bytes to be encrypted).
 * 
 * Usage: \n
 * In provide thig functionality the functions from thig.c should be used.\n
 * I_THIG_encrypt_header and I_THIG_decrypt_header take as parameters the SIP message 
 * and the name of the header.Using this name the function searches for the header
 * in the messages and then encrypts the information.
 * However if you are not concerned with the details of each header then you should use
 * I_THIG_encrypt_all_headers and I_THIG_decrypt_all_headers. Those functions try
 * to encrypt/ decrypt all sensitive headers (Via, Route , Record-Route,Service-Route)
 * 
 * 
 * \section thig_example Example
 *  
 * 
 * 
 * - Interrogating-CSCF configuration file with THIG support \include icscf.thig.cfg
 * 
 */ 

#ifndef THIG_H_
#define THIG_H_

#include "mod.h"
#include "../../sr_module.h"

int I_THIG_add_Path(struct sip_msg* msg, char* str1, char* str2);
int I_THIG_add_RR(struct sip_msg* msg, char* str1, char* str2);

int I_THIG_encrypt_header(struct sip_msg* msg, char* str1, char* str2);
int I_THIG_decrypt_header(struct sip_msg* msg, char* str1, char* str2);

int I_THIG_encrypt_all_headers(struct sip_msg* msg, char* str1, char* str2);
int I_THIG_decrypt_all_headers(struct sip_msg* msg, char* str1, char* str2);

#endif /*THIG_H_*/
