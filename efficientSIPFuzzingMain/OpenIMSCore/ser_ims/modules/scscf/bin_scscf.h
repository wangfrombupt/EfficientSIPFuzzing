/*
 * $Id: bin_scscf.h 161 2007-03-01 14:06:01Z vingarzan $
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
 * Binary codec operations for the S-CSCF
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */



#ifndef _BIN_SCSCF_H
#define _BIN_SCSCF_H

#include "bin.h"
#include "ifc_datastruct.h"
#include "registration.h"
#include "registrar_storage.h"
#include "dlg_state.h"
#include "../tm/tm_load.h"


#define BIN_INITIAL_ALLOC_SIZE 256

int bin_encode_ims_subscription(bin_data *x, ims_subscription *s);
ims_subscription *bin_decode_ims_subscription(bin_data *x);

int bin_encode_r_public(bin_data *x,r_public *p);
r_public* bin_decode_r_public(bin_data *x);

int bin_encode_auth_userdata(bin_data *x,auth_userdata *u);
auth_userdata* bin_decode_auth_userdata(bin_data *x);

int bin_encode_s_dialog(bin_data *x,s_dialog *d);
s_dialog* bin_decode_s_dialog(bin_data *x);

#endif
