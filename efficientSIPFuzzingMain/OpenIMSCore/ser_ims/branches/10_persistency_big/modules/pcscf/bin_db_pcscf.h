/*
 * $Id: bin_pcscf_db.h 236 2007-04-18 12:53:40 +0000 (Wed, 18 Apr 2007) vingarzan $
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
 * P-CSCF persistency operations - db dump/load
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */



#ifndef _BIN_DB_PCSCF_H
#define _BIN_DB_PCSCF_H

#include "bin_pcscf.h"


int bin_dump_to_db(bin_data *x, data_type_t dt);
int bin_dump_registrar_to_table(bin_data* x, int snapshot_version, int step_version);
int bin_dump_dialogs_to_table(bin_data* x, int snapshot_version, int step_version);
int bin_dump_subs_to_table(bin_data* x, int snapshot_version, int step_version);
int bin_bulk_dump_to_table(data_type_t dt, int snapshot_version, int step_version, bin_data *x);
int bin_cache_dump_registrar_to_table(int snapshot_version, int step_version);
int bin_cache_dump_dialogs_to_table(int snapshot_version, int step_version);
int bin_cache_dump_subs_to_table(int snapshot_version, int step_version);
int delete_older_snapshots(char* table, char* node_id, data_type_t dt, int current_snapshot);
 
int bin_load_from_db(bin_data *x, data_type_t dt);
int bin_load_registrar_from_table(bin_data *x);
int bin_load_dialogs_from_table(bin_data *x);
int bin_load_subscriptions_from_table(bin_data *x);
int bin_bulk_load_from_table(data_type_t dt, bin_data* x);
int bin_cache_load_registrar_from_table();
int bin_cache_load_dialogs_from_table();
int bin_cache_load_subscriptions_from_table();
int db_get_last_snapshot_version(char* table, char* node_id, data_type_t dt, int* version);
int set_versions(data_type_t dt, int snapshot_version, int step_version);



#endif
