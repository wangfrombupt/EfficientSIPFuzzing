/* 
 * $Id: dbase.c 166 2007-03-02 19:28:23Z vingarzan $ 
 *
 * MySQL module core functions
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include "../../mem/mem.h"
#include "../../dprint.h"
#include "../../db/db_pool.h"
#include "../../globals.h"
#include "../../pt.h"
#include "utils.h"
#include "val.h"
#include "my_con.h"
#include "res.h"
#include "db_mod.h"
#include "dbase.h"


#define SQL_BUF_LEN 65536

static char sql_buf[SQL_BUF_LEN];


/*
 * Send an SQL query to the server
 */
static int submit_query(db_con_t* _h, const char* _s)
{	
	time_t t;
	int i, code;

	if ((!_h) || (!_s)) {
		LOG(L_ERR, "submit_query: Invalid parameter value\n");
		return -1;
	}

	if (ping_interval) {
		t = time(0);
		if ((t - CON_TIMESTAMP(_h)) > ping_interval) {
			if (mysql_ping(CON_CONNECTION(_h))) {
				DBG("submit_query: mysql_ping failed\n");
			}
		}
		CON_TIMESTAMP(_h) = t;
	}

	/* screws up the terminal when the query contains a BLOB :-( (by bogdan)
	 * DBG("submit_query(): %s\n", _s);
	 */

	/* When a server connection is lost and a query is attempted, most of
	 * the time the query will return a CR_SERVER_LOST, then at the second
	 * attempt to execute it, the mysql lib will reconnect and succeed.
	 * However is a few cases, the first attempt returns CR_SERVER_GONE_ERROR
	 * the second CR_SERVER_LOST and only the third succeeds.
	 * Thus the 3 in the loop count. Increasing the loop count over this
	 * value shouldn't be needed, but it doesn't hurt either, since the loop
	 * will most of the time stop at the second or sometimes at the third
	 * iteration.
	 */
	for (i=0; i<(auto_reconnect ? 3 : 1); i++) {
		if (mysql_query(CON_CONNECTION(_h), _s)==0) {
			return 0;
		}
		code = mysql_errno(CON_CONNECTION(_h));
		if (code != CR_SERVER_GONE_ERROR && code != CR_SERVER_LOST) {
			break;
		}
	}
	LOG(L_ERR, "submit_query: %s\n", mysql_error(CON_CONNECTION(_h)));
	return -2;
}


/*
 * Print list of columns separated by comma
 */
static int print_columns(char* _b, int _l, db_key_t* _c, int _n)
{
	int i, ret;
	int len = 0;

	if ((!_c) || (!_n) || (!_b) || (!_l)) {
		LOG(L_ERR, "print_columns: Invalid parameter value\n");
		return -1;
	}

	for(i = 0; i < _n; i++) {
		if (i == (_n - 1)) {
			ret = snprintf(_b + len, _l - len, "%s ", _c[i]);
			if (ret < 0 || ret >= (_l - len)) goto error;
			len += ret;
		} else {
			ret = snprintf(_b + len, _l - len, "%s,", _c[i]);
			if (ret < 0 || ret >= (_l - len)) goto error;
			len += ret;
		}
	}
	return len;

 error:
	LOG(L_ERR, "print_columns: Error in snprintf\n");
	return -1;
}


/*
 * Print list of values separated by comma
 */
static int print_values(MYSQL* _c, char* _b, int _l, db_val_t* _v, int _n)
{
	int i, res = 0, l;

	if (!_c || !_b || !_l || !_v || !_n) {
		LOG(L_ERR, "print_values: Invalid parameter value\n");
		return -1;
	}

	for(i = 0; i < _n; i++) {
		l = _l - res;
		if (val2str(_c, _v + i, _b + res, &l) < 0) {
			LOG(L_ERR, "print_values: Error while converting value to string\n");
			return -1;
		}
		res += l;
		if (i != (_n - 1)) {
			*(_b + res) = ',';
			res++;
		}
	}
	return res;
}


/*
 * Print where clause of SQL statement
 */
static int print_where(MYSQL* _c, char* _b, int _l, db_key_t* _k, db_op_t* _o, db_val_t* _v, int _n)
{
	int i;
	int len = 0, ret;
	int l;

	if (!_c || !_b || !_l || !_k || !_v || !_n) {
		LOG(L_ERR, "print_where: Invalid parameter value\n");
		return -1;
	}

	for(i = 0; i < _n; i++) {
		if (_v[i].nul) {
			ret = snprintf(_b + len, _l - len, "%s is ", _k[i]);
			if (ret < 0 || ret >= (_l - len)) goto error;
			len += ret;
		} else if (_o) {
			ret = snprintf(_b + len, _l - len, "%s%s", _k[i], _o[i]);
			if (ret < 0 || ret >= (_l - len)) goto error;
			len += ret;
		} else {
			ret = snprintf(_b + len, _l - len, "%s=", _k[i]);
			if (ret < 0 || ret >= (_l - len)) goto error;
			len += ret;
		}
		l = _l - len;
		val2str(_c, &(_v[i]), _b + len, &l);
		len += l;
		if (i == (_n - 1))
			ret = snprintf(_b + len, _l - len, " ");
		else
			ret = snprintf(_b + len, _l - len, " AND ");
		if (ret < 0 || ret >= (_l - len)) goto error;
		len += ret;
	}
	return len;

 error:
	LOG(L_ERR, "print_where: Error in snprintf\n");
	return -1;
}


/*
 * Print set clause of update SQL statement
 */
static int print_set(MYSQL* _c, char* _b, int _l, db_key_t* _k, db_val_t* _v, int _n)
{
	int i;
	int len = 0, ret;
	int l;

	if (!_c || !_b || !_l || !_k || !_v || !_n) {
		LOG(L_ERR, "print_set: Invalid parameter value\n");
		return -1;
	}

	for(i = 0; i < _n; i++) {
		ret = snprintf(_b + len, _l - len, "%s=", _k[i]);
		if (ret < 0 || ret >= (_l - len)) goto error;
		len += ret;

		l = _l - len;
		val2str(_c, &(_v[i]), _b + len, &l);
		len += l;
		if (i != (_n - 1)) {
			if ((_l - len) >= 1) {
				*(_b + len++) = ',';
			}
		}
	}
	return len;

 error:
	LOG(L_ERR, "print_set: Error in snprintf\n");
	return -1;
}


/*
 * Initialize database module
 * No function should be called before this
 */
db_con_t* db_init(const char* _url)
{
	struct db_id* id;
	struct my_con* con;
	db_con_t* res;

	id = 0;
	res = 0;

	/* if called from PROC_MAIN, allow it only from mod_init( when pt==0)*/
	if (is_main && fixup_complete){
		LOG(L_ERR, "BUG: mysql: db_init: called from the main process,"
					" ignoring...\n");
	}

	if (!_url) {
		LOG(L_ERR, "db_init: Invalid parameter value\n");
		return 0;
	}

	res = pkg_malloc(sizeof(db_con_t) + sizeof(struct my_con*));
	if (!res) {
		LOG(L_ERR, "db_init: No memory left\n");
		return 0;
	}
	memset(res, 0, sizeof(db_con_t) + sizeof(struct my_con*));

	id = new_db_id(_url);
	if (!id) {
		LOG(L_ERR, "db_init: Cannot parse URL '%s'\n", _url);
		goto err;
	}

	     /* Find the connection in the pool */
	con = (struct my_con*)pool_get(id);
	if (!con) {
		DBG("db_init: Connection '%s' not found in pool\n", _url);
		     /* Not in the pool yet */
		con = new_connection(id);
		if (!con) {
			goto err;
		}
		pool_insert((struct pool_con*)con);
	} else {
		DBG("db_init: Connection '%s' found in pool\n", _url);
	}

	res->tail = (unsigned long)con;
	return res;

 err:
	if (id) free_db_id(id);
	if (res) pkg_free(res);
	return 0;
}


/*
 * Shut down database module
 * No function should be called after this
 */
void db_close(db_con_t* _h)
{
	struct pool_con* con;

	if (!_h) {
		LOG(L_ERR, "db_close: Invalid parameter value\n");
		return;
	}

	con = (struct pool_con*)_h->tail;
	if (pool_remove(con) != 0) {
		free_connection((struct my_con*)con);
	}

	pkg_free(_h);
}


/*
 * Retrieve result set
 */
static int store_result(db_con_t* _h, db_res_t** _r)
{
	if ((!_h) || (!_r)) {
		LOG(L_ERR, "store_result: Invalid parameter value\n");
		return -1;
	}

	*_r = new_result();
	if (*_r == 0) {
		LOG(L_ERR, "store_result: No memory left\n");
		return -2;
	}

	MYRES_RESULT(*_r) = mysql_store_result(CON_CONNECTION(_h));
	if (!MYRES_RESULT(*_r)) {
		if (mysql_field_count(CON_CONNECTION(_h)) == 0) {
			(*_r)->col.n = 0;
			(*_r)->n = 0;
			return 0;
		} else {
			LOG(L_ERR, "store_result: %s\n", mysql_error(CON_CONNECTION(_h)));
			free_result(*_r);
			*_r = 0;
			return -3;
		}
	}

	if (convert_result(_h, *_r) < 0) {
		LOG(L_ERR, "store_result: Error while converting result\n");
		mysql_free_result(MYRES_RESULT(*_r));
		pkg_free((*_r)->data);
		pkg_free(*_r);

		/* This cannot be used because if convert_result fails,
		 * free_result will try to free rows and columns too 
		 * and free will be called two times
		 */
		/* free_result(*_r); */
		return -4;
	}
	
	return 0;
}


/*
 * Release a result set from memory
 */
int db_free_result(db_con_t* _h, db_res_t* _r)
{
     if ((!_h) || (!_r)) {
	     LOG(L_ERR, "db_free_result: Invalid parameter value\n");
	     return -1;
     }

     if (free_result(_r) < 0) {
	     LOG(L_ERR, "db_free_result: Unable to free result structure\n");
	     return -1;
     }
     return 0;
}


/*
 * Query table for specified rows
 * _h: structure representing database connection
 * _k: key names
 * _op: operators
 * _v: values of the keys that must match
 * _c: column names to return
 * _n: number of key=values pairs to compare
 * _nc: number of columns to return
 * _o: order by the specified column
 */
int db_query(db_con_t* _h, db_key_t* _k, db_op_t* _op,
	     db_val_t* _v, db_key_t* _c, int _n, int _nc,
	     db_key_t _o, db_res_t** _r)
{
	int off, ret;

	if (!_h) {
		LOG(L_ERR, "db_query: Invalid parameter value\n");
		return -1;
	}

	if (!_c) {
		ret = snprintf(sql_buf, SQL_BUF_LEN, "select * from %s ", CON_TABLE(_h));
		if (ret < 0 || ret >= SQL_BUF_LEN) goto error;
		off = ret;
	} else {
		ret = snprintf(sql_buf, SQL_BUF_LEN, "select ");
		if (ret < 0 || ret >= SQL_BUF_LEN) goto error;
		off = ret;

		ret = print_columns(sql_buf + off, SQL_BUF_LEN - off, _c, _nc);
		if (ret < 0) return -1;
		off += ret;

		ret = snprintf(sql_buf + off, SQL_BUF_LEN - off, "from %s ", CON_TABLE(_h));
		if (ret < 0 || ret >= (SQL_BUF_LEN - off)) goto error;
		off += ret;
	}
	if (_n) {
		ret = snprintf(sql_buf + off, SQL_BUF_LEN - off, "where ");
		if (ret < 0 || ret >= (SQL_BUF_LEN - off)) goto error;
		off += ret;

		ret = print_where(CON_CONNECTION(_h), sql_buf + off, SQL_BUF_LEN - off, _k, _op, _v, _n);
		if (ret < 0) return -1;;
		off += ret;
	}
	if (_o) {
		ret = snprintf(sql_buf + off, SQL_BUF_LEN - off, "order by %s", _o);
		if (ret < 0 || ret >= (SQL_BUF_LEN - off)) goto error;
		off += ret;
	}
	
	*(sql_buf + off) = '\0';
	if (submit_query(_h, sql_buf) < 0) {
		LOG(L_ERR, "db_query: Error while submitting query\n");
		return -2;
	}

	return store_result(_h, _r);

 error:
	LOG(L_ERR, "db_query: Error in snprintf\n");
	return -1;
}


/*
 * Execute a raw SQL query
 */
int db_raw_query(db_con_t* _h, char* _s, db_res_t** _r)
{
	if ((!_h) || (!_s)) {
		LOG(L_ERR, "db_raw_query: Invalid parameter value\n");
		return -1;
	}

	if (submit_query(_h, _s) < 0) {
		LOG(L_ERR, "db_raw_query: Error while submitting query\n");
		return -2;
	}

	if(_r)
	    return store_result(_h, _r);
	return 0;
}


/*
 * Insert a row into specified table
 * _h: structure representing database connection
 * _k: key names
 * _v: values of the keys
 * _n: number of key=value pairs
 */
int db_insert(db_con_t* _h, db_key_t* _k, db_val_t* _v, int _n)
{
	int off, ret;

	if ((!_h) || (!_k) || (!_v) || (!_n)) {
		LOG(L_ERR, "db_insert: Invalid parameter value\n");
		return -1;
	}

	ret = snprintf(sql_buf, SQL_BUF_LEN, "insert into %s (", CON_TABLE(_h));
	if (ret < 0 || ret >= SQL_BUF_LEN) goto error;
	off = ret;

	ret = print_columns(sql_buf + off, SQL_BUF_LEN - off, _k, _n);
	if (ret < 0) return -1;
	off += ret;

	ret = snprintf(sql_buf + off, SQL_BUF_LEN - off, ") values (");
	if (ret < 0 || ret >= (SQL_BUF_LEN - off)) goto error;
	off += ret;

	ret = print_values(CON_CONNECTION(_h), sql_buf + off, SQL_BUF_LEN - off, _v, _n);
	if (ret < 0) return -1;
	off += ret;

	*(sql_buf + off++) = ')';
	*(sql_buf + off) = '\0';

	if (submit_query(_h, sql_buf) < 0) {
	        LOG(L_ERR, "db_insert: Error while submitting query\n");
		return -2;
	}
	return 0;

 error:
	LOG(L_ERR, "db_insert: Error in snprintf\n");
	return -1;
}


/*
 * Delete a row from the specified table
 * _h: structure representing database connection
 * _k: key names
 * _o: operators
 * _v: values of the keys that must match
 * _n: number of key=value pairs
 */
int db_delete(db_con_t* _h, db_key_t* _k, db_op_t* _o, db_val_t* _v, int _n)
{
	int off, ret;

	if (!_h) {
		LOG(L_ERR, "db_delete: Invalid parameter value\n");
		return -1;
	}

	ret = snprintf(sql_buf, SQL_BUF_LEN, "delete from %s", CON_TABLE(_h));
	if (ret < 0 || ret >= SQL_BUF_LEN) goto error;
	off = ret;

	if (_n) {
		ret = snprintf(sql_buf + off, SQL_BUF_LEN - off, " where ");
		if (ret < 0 || ret >= (SQL_BUF_LEN - off)) goto error;
		off += ret;

		ret = print_where(CON_CONNECTION(_h), sql_buf + off, SQL_BUF_LEN - off, _k, _o, _v, _n);
		if (ret < 0) return -1;
		off += ret;
	}

	*(sql_buf + off) = '\0';
	if (submit_query(_h, sql_buf) < 0) {
		LOG(L_ERR, "db_delete: Error while submitting query\n");
		return -2;
	}
	return 0;

 error:
	LOG(L_ERR, "db_delete: Error in snprintf\n");
	return -1;
}


/*
 * Update some rows in the specified table
 * _h: structure representing database connection
 * _k: key names
 * _o: operators
 * _v: values of the keys that must match
 * _uk: updated columns
 * _uv: updated values of the columns
 * _n: number of key=value pairs
 * _un: number of columns to update
 */
int db_update(db_con_t* _h, db_key_t* _k, db_op_t* _o, db_val_t* _v,
	      db_key_t* _uk, db_val_t* _uv, int _n, int _un)
{
	int off, ret;

	if ((!_h) || (!_uk) || (!_uv) || (!_un)) {
		LOG(L_ERR, "db_update: Invalid parameter value\n");
		return -1;
	}

	ret = snprintf(sql_buf, SQL_BUF_LEN, "update %s set ", CON_TABLE(_h));
	if (ret < 0 || ret >= SQL_BUF_LEN) goto error;
	off = ret;

	ret = print_set(CON_CONNECTION(_h), sql_buf + off, SQL_BUF_LEN - off, _uk, _uv, _un);
	if (ret < 0) return -1;
	off += ret;

	if (_n) {
		ret = snprintf(sql_buf + off, SQL_BUF_LEN - off, " where ");
		if (ret < 0 || ret >= (SQL_BUF_LEN - off)) goto error;
		off += ret;

		ret = print_where(CON_CONNECTION(_h), sql_buf + off, SQL_BUF_LEN - off, _k, _o, _v, _n);
		if (ret < 0) return -1;
		off += ret;

		*(sql_buf + off) = '\0';
	}

	if (submit_query(_h, sql_buf) < 0) {
		LOG(L_ERR, "db_update: Error while submitting query\n");
		return -2;
	}
	return 0;

 error:
	LOG(L_ERR, "db_update: Error in snprintf\n");
	return -1;
}


/*
 * Just like insert, but replace the row if it exists
 */
int db_replace(db_con_t* handle, db_key_t* keys, db_val_t* vals, int n)
{
	int off, ret;

	if (!handle || !keys || !vals) {
		LOG(L_ERR, "db_replace: Invalid parameter value\n");
		return -1;
	}

	ret = snprintf(sql_buf, SQL_BUF_LEN, "replace %s (", CON_TABLE(handle));
	if (ret < 0 || ret >= SQL_BUF_LEN) goto error;
	off = ret;

	ret = print_columns(sql_buf + off, SQL_BUF_LEN - off, keys, n);
	if (ret < 0) return -1;
	off += ret;

	ret = snprintf(sql_buf + off, SQL_BUF_LEN - off, ") values (");
	if (ret < 0 || ret >= (SQL_BUF_LEN - off)) goto error;
	off += ret;

	ret = print_values(CON_CONNECTION(handle), sql_buf + off, SQL_BUF_LEN - off, vals, n);
	if (ret < 0) return -1;
	off += ret;

	*(sql_buf + off++) = ')';
	*(sql_buf + off) = '\0';

	if (submit_query(handle, sql_buf) < 0) {
	        LOG(L_ERR, "db_replace: Error while submitting query\n");
		return -2;
	}
	return 0;

 error:
	LOG(L_ERR, "db_replace: Error in snprintf\n");
	return -1;
}
