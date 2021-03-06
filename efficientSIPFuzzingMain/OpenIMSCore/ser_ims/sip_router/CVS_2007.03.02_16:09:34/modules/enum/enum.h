/*
 * $Id: enum.h 165 2007-03-02 15:15:46Z vingarzan $
 *
 * Header file for Enum and E164 related functions
 *
 * Copyright (C) 2002-2003 Juha Heinanen
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


#ifndef ENUM_H
#define ENUM_H


#include "../../parser/msg_parser.h"


#define MAX_DOMAIN_SIZE 256
		

/*
 * Check if from user is an e164 number
 */
int is_e164(struct sip_msg* msg, char* p1, char* p2);


/*
 * Make enum query and if query succeeds, replace current uri with the
 * result of the query
 */
int enum_query(struct sip_msg* msg, char* p1, char* p2);


#endif /* ENUM_H */
