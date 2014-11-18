/*
 * $Id: nat_helper.h 566 2008-06-05 12:55:00Z vingarzan $
 *
 *
 * Copyright (C) 2005 Porta Software Ltd.
 *
 * This file is part of ser, a free SIP server.
 *
 * ser is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the ser software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * ser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * $Id: nat_helper.h 566 2008-06-05 12:55:00Z vingarzan $
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
 * Proxy-CSCF - NAT helper for signalling
 * 
 * \note Taken from SER nathelper module
 *  
 * \author adapted for the pcscf module by Marius Corici mco -at- fokus dot fraunhofer dot de
 * 
 */


#ifndef P_CSCF_NAT_HELPER_H_
#define P_CSCF_NAT_HELPER_H_

#include "registrar_storage.h"

/** Network structure */
typedef struct {
	const char *cnetaddr;	/**< string with he network address */
	uint32_t netaddr;		/**< network address */
	uint32_t mask;			/**< network mask */
} network_t;

char * ser_memmem(const void *b1, const void *b2, size_t len1, size_t len2);
int is1918addr(str *saddr);

/** NAT test for Contact */
#define NAT_UAC_TEST_C_1918	0x01
/** NAT test for received from */
#define NAT_UAC_TEST_RCVD	0x02
/** NAT test for Via */
#define NAT_UAC_TEST_V_1918 0x04
/** NAT test for SDP? */
#define NAT_UAC_TEST_S_1918 0x08
/** NAT test for rport */
#define NAT_UAC_TEST_RPORT	0x10 
 
int nat_send_ping(r_contact *c);
int nat_uac_test(struct sip_msg * msg);
int nat_prepare_1918addr();
r_nat_dest* nat_msg_origin(struct sip_msg * msg);
int requires_nat(struct sip_msg * msg);


#endif

