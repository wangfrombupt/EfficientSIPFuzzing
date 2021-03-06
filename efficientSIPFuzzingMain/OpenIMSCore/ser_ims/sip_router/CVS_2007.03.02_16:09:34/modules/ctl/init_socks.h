/*
 * $Id: init_socks.h 165 2007-03-02 15:15:46Z vingarzan $
 *
 * Copyright (C) 2006 iptelorg GmbH
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
/* History:
 * --------
 *  2006-02-14  created by andrei
 */

#ifndef _init_socks_h
#define _init_socks_h
#include <sys/un.h>
#include "../../ip_addr.h"

enum socket_protos	{	UNKNOWN_SOCK=0, UDP_SOCK, TCP_SOCK, 
						UNIXS_SOCK, UNIXD_SOCK
#ifdef USE_FIFO
							, FIFO_SOCK
#endif
};

int init_unix_sock(struct sockaddr_un* su, char* name, int type,
					int perm, int uid, int gid);
int init_tcpudp_sock(union sockaddr_union* su, char* address, int port,
					enum socket_protos type);
int init_sock_opt(int s, enum socket_protos type);

inline static char* socket_proto_name(enum socket_protos p)
{
	switch(p){
		case UDP_SOCK:
			return "udp";
		case TCP_SOCK:
			return "tcp";
		case UNIXS_SOCK:
			return "unix_stream";
		case UNIXD_SOCK:
			return "unix_dgram";
#ifdef USE_FIFO
		case FIFO_SOCK:
			return "fifo";
#endif
		default:
			;
	}
	return "<unknown>";
}
#endif
