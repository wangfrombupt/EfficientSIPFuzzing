/*
 * $Id: xl_lib.h 165 2007-03-02 15:15:46Z vingarzan $
 *
 * Copyright (C) 2001-2003 FhG Fokus
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

#ifndef _XL_LIB_H_
#define _XL_LIB_H_

#include "../../parser/msg_parser.h"

typedef int (*item_func_t) (struct sip_msg*, str*, str*, int, int);

typedef struct _xl_elog
{
	str text;
	str hparam;
	int hindex;
	int hflags;
	item_func_t itf;
	struct _xl_elog *next;
} xl_elog_t, *xl_elog_p;

int xl_elog_free_all(xl_elog_p log);
typedef int (xl_parse_format_f)(char *s, xl_elog_p *el);
//int xl_parse_format(char *s, xl_elog_p *el);
typedef int (xl_print_log_f)(struct sip_msg*, xl_elog_t*, char*, int*);
//int xl_print_log(struct sip_msg* msg, xl_elog_p log, char *buf, int *len);
typedef str* (xl_get_nulstr_f)(void);

xl_parse_format_f xl_parse_format;
xl_print_log_f xl_print_log;
xl_get_nulstr_f xl_get_nulstr;

int xl_mod_init();
int xl_child_init(int);
#endif

