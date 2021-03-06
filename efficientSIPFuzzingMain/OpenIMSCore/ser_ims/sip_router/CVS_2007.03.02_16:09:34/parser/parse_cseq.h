/*
 * $Id: parse_cseq.h 165 2007-03-02 15:15:46Z vingarzan $
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


#ifndef PARSE_CSEQ
#define PARSE_CSEQ

#include "../str.h"


struct cseq_body{
	int error;  /* Error code */
	str number; /* CSeq number */
	str method; /* Associated method */
};


/* casting macro for accessing CSEQ body */
#define get_cseq(p_msg) ((struct cseq_body*)(p_msg)->cseq->parsed)


/*
 * Parse CSeq header field
 */
char* parse_cseq(char *buf, char* end, struct cseq_body* cb);


/*
 * Free all associated memory
 */
void free_cseq(struct cseq_body* cb);


#endif
