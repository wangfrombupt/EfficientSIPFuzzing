/*
 * $Id: enum.c 579 2008-08-25 15:24:33Z vingarzan $
 *
 * Enum and E164 related functions
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
 *
 */


#include "enum.h"
#include "../../parser/parse_uri.h"
#include "../../parser/parse_from.h"
#include "../../parser/parse_nameaddr.h"
#include "../../parser/parse_rr.h"
#include "../../lump_struct.h"
#include "../../data_lump.h"
#include "../../ut.h"
#include "../../resolve.h"
#include "../../mem/mem.h"
#include "../../dset.h"
#include "../../qvalue.h"
#include "../../sr_module.h"
#include "enum_mod.h"
#include "regexp.h"


/* return the length of the string until c, if not found returns n */
static inline int findchr(char* p, int c, unsigned int size)
{
  int len=0;

  for(;len<size;p++){
    if (*p==(unsigned char)c) {
      return len;
    }
    len++;   
  }
  return len;
}

/* Checks if NAPTR record has flag u and its services field
 * e2u+[service:]sip or
 * e2u+service[+service[+service[+...]]]
 */
static inline int sip_match( struct naptr_rdata* naptr, str* service)
{
  if (service->len == 0) {
    return (naptr->flags_len == 1) &&
      ((naptr->flags[0] == 'u') || (naptr->flags[0] == 'U')) &&
      (naptr->services_len == 7) &&
      ((strncasecmp(naptr->services, "e2u+sip", 7) == 0) ||
       (strncasecmp(naptr->services, "sip+e2u", 7) == 0));
  } else if (service->s[0] != '+') {
    return (naptr->flags_len == 1) &&
      ((naptr->flags[0] == 'u') || (naptr->flags[0] == 'U')) &&
      (naptr->services_len == service->len + 8) &&
      (strncasecmp(naptr->services, "e2u+", 4) == 0) &&
      (strncasecmp(naptr->services + 4, service->s, service->len) == 0) &&
      (strncasecmp(naptr->services + 4 + service->len, ":sip", 4) == 0);
  } else { /* handle compound NAPTRs and multiple services */
    str bakservice, baknaptr; /* we bakup the str */
    int naptrlen, len;        /* length of the extracted service */

    /* RFC 3761, NAPTR service field must start with E2U+ */
    if (strncasecmp(naptr->services, "e2u+", 4) != 0) {
      return 0;
    }
    baknaptr.s   = naptr->services + 4; /* leading 'e2u+' */
    baknaptr.len = naptr->services_len - 4;
    for (;;) { /* iterate over services in NAPTR */
      bakservice.s   = service->s + 1; /* leading '+' */
      bakservice.len = service->len - 1;
      naptrlen = findchr(baknaptr.s,'+',baknaptr.len);

      for (;;) { /* iterate over services in enum_query */
        len = findchr(bakservice.s,'+',bakservice.len);
        if ((naptrlen == len ) && !strncasecmp(baknaptr.s , bakservice.s, len)) {
          return 1;
        }
        if ( (bakservice.len -= len+1) > 0) {
          bakservice.s += len+1;
          continue;
        }
        break;
      }
      if ( (baknaptr.len -= naptrlen+1) > 0) {
        baknaptr.s += naptrlen+1;
        continue;
      }
      break;
    }
    /* no matching service found */
    return 0;
  }    
}


static int test_e164(str* user)
{
    int i;
    char c;

    if ((user->len > 2) && (user->len < 17) && ((user->s)[0] == '+')) {
	for (i = 1; i <= user->len; i++) {
	    c = (user->s)[i];
	    if (c < '0' && c > '9') return -1;
	}
	return 1;
    }
    return -1;
}


/*
 * Check if from user is an e164 number
 */
int is_e164(struct sip_msg* msg, char* p1, char* p2)
{
    str user;

    if (get_str_fparam(&user, msg, (fparam_t*)p1) < 0) {
	ERR("Error while obtaining username to be checked\n");
	return -1;
    }
    return test_e164(&user);
}


/* Parse NAPTR regexp field of the form !pattern!replacement! and return its
 * components in pattern and replacement parameters.  Regexp field starts at
 * address first and is len characters long.
 */
static inline int parse_naptr_regexp(char* first, int len, str* pattern,
										str* replacement)
{
	char *second, *third;

	if (len > 0) {
		if (*first == '!') {
			second = (char *)memchr((void *)(first + 1), '!', len - 1);
			if (second) {
				len = len - (second - first + 1);
				if (len > 0) {
					third = memchr(second + 1, '!', len);
					if (third) {
						pattern->len = second - first - 1;
						pattern->s = first + 1;
						replacement->len = third - second - 1;
						replacement->s = second + 1;
						return 1;
					} else {
						LOG(LOG_ERR, "parse_regexp(): third ! missing from regexp\n");
						return -1;
					}
				} else {
					LOG(LOG_ERR, "parse_regexp(): third ! missing from regexp\n");
					return -2;
				}
			} else {
				LOG(LOG_ERR, "parse_regexp(): second ! missing from regexp\n");
				return -3;
			}
		} else {
			LOG(LOG_ERR, "parse_regexp(): first ! missing from regexp\n");
			return -4;
		}
	} else {
		LOG(LOG_ERR, "parse_regexp(): regexp missing\n");
		return -5;
	}
}


/* 
 * Add parameter to URI.
 */
int add_uri_param(str *uri, str *param, str *new_uri)
{
	struct sip_uri puri;
	char *at;

	if (parse_uri(uri->s, uri->len, &puri) < 0) {
		return 0;
	}

	/* if current uri has no headers, pad param to the end of uri */
	if (puri.headers.len == 0) {
		memcpy(uri->s + uri->len, param->s, param->len);
		uri->len = uri->len + param->len;
		new_uri->len = 0;
		return 1;
	}

	/* otherwise take the long path and create new_uri */
	at = new_uri->s;
	memcpy(at, "sip:", 4);
	at = at + 4;
	if (puri.user.len) {
		memcpy(at, puri.user.s, puri.user.len);
		at = at + puri.user.len;
		if (puri.passwd.len) {
			*at = ':';
			at = at + 1;
			memcpy(at, puri.passwd.s, puri.passwd.len);
			at = at + puri.passwd.len;
		};
		*at = '@';
		at = at + 1;
	}
	memcpy(at, puri.host.s, puri.host.len);
	at = at + puri.host.len;
	if (puri.port.len) {
		*at = ':';
		at = at + 1;
		memcpy(at, puri.port.s, puri.port.len);
		at = at + puri.port.len;
	}
	if (puri.params.len) {
		*at = ';';
		at = at + 1;
		memcpy(at, puri.params.s, puri.params.len);
		at = at + puri.params.len;
	}
	memcpy(at, param->s, param->len);
	at = at + param->len;
	*at = '?';
	at = at + 1;
	memcpy(at, puri.headers.s, puri.headers.len);
	at = at + puri.headers.len;
	new_uri->len = at - new_uri->s;
	return 1;
}

/*
 * Tests if one result record is "greater" that the other.  Non-NAPTR records
 * greater that NAPTR record.  An invalid NAPTR record is greater than a 
 * valid one.  Valid NAPTR records are compared based on their
 * (order,preference).
 */
static inline int naptr_greater(struct rdata* a, struct rdata* b)
{
	struct naptr_rdata *na, *nb;

	if (a->type != T_NAPTR) return 1;
	if (b->type != T_NAPTR) return 0;

	na = (struct naptr_rdata*)a->rdata;
	if (na == 0) return 1;

	nb = (struct naptr_rdata*)b->rdata;
	if (nb == 0) return 0;
	
	return (((na->order) << 16) + na->pref) >
		(((nb->order) << 16) + nb->pref);
}
	
	
/*
 * Bubble sorts result record list according to naptr (order,preference).
 */
static inline void naptr_sort(struct rdata** head)
{
	struct rdata *p, *q, *r, *s, *temp, *start;

        /* r precedes p and s points to the node up to which comparisons
         are to be made */ 

	s = NULL;
	start = *head;
	while ( s != start -> next ) { 
		r = p = start ; 
		q = p -> next ;
		while ( p != s ) { 
			if ( naptr_greater(p, q) ) { 
				if ( p == start ) { 
					temp = q -> next ; 
					q -> next = p ; 
					p -> next = temp ;
					start = q ; 
					r = q ; 
				} else {
					temp = q -> next ; 
					q -> next = p ; 
					p -> next = temp ;
					r -> next = q ; 
					r = q ; 
				} 
			} else {
				r = p ; 
				p = p -> next ; 
			} 
			q = p -> next ; 
			if ( q == s ) s = p ; 
		}
	}
	*head = start;
}	

	
/*
 * See documentation in README file.
 */

int enum_query(struct sip_msg* msg, char* p1, char* p2)
{
	char *user_s;
	int user_len, i, j, first;
	char name[MAX_DOMAIN_SIZE];
	char uri[MAX_URI_SIZE];
	char new_uri[MAX_URI_SIZE];
	unsigned int priority, curr_prio;
	qvalue_t q;

	struct rdata* head;
	struct rdata* l;
	struct naptr_rdata* naptr;

	str pattern, replacement, result, new_result;

	char string[17];

	str suffix, service;

	if (p1) {
	    if (get_str_fparam(&suffix, msg, (fparam_t*)p1) < 0) {
		ERR("Unable to get suffix value\n");
		return -1;
	    }
	} else {
	    suffix = domain_suffix;
	}

	if (p2) {
	    if (get_str_fparam(&service, msg, (fparam_t*)p2) < 0) {
		ERR("Unable to get service value\n");
		return -1;
	    }
	} else {
	    service = default_service;
	}
	
	if (parse_sip_msg_uri(msg) < 0) {
		LOG(L_ERR, "enum_query(): uri parsing failed\n");
		return -1;
	}

	if (test_e164(&(msg->parsed_uri.user)) == -1) {
		LOG(L_ERR, "enum_query(): uri user is not an E164 number\n");
		return -1;
	}

	user_s = msg->parsed_uri.user.s;
	user_len = msg->parsed_uri.user.len;

	memcpy(&(string[0]), user_s, user_len);
	string[user_len] = (char)0;

	j = 0;
	for (i = user_len - 1; i > 0; i--) {
		name[j] = user_s[i];
		name[j + 1] = '.';
		j = j + 2;
	}

	memcpy(name + j, suffix.s, suffix.len + 1);

	head = get_record(name, T_NAPTR, RES_ONLY_TYPE);

	if (head == 0) {
		DBG("enum_query(): No NAPTR record found for %s.\n", name);
		return -1;
	}

	naptr_sort(&head);

	q = MAX_Q - 10;
	curr_prio = 0;
	first = 1;

	for (l = head; l; l = l->next) {

		if (l->type != T_NAPTR) continue; /*should never happen*/
		naptr = (struct naptr_rdata*)l->rdata;
		if (naptr == 0) {
			LOG(L_CRIT, "enum_query: BUG: null rdata\n");
			continue;
		}

		DBG("enum_query(): order %u, pref %u, flen %u, flags '%.*s', slen %u, "
		    "services '%.*s', rlen %u, regexp '%.*s'\n", naptr->order, naptr->pref,
		    naptr->flags_len, (int)(naptr->flags_len), ZSW(naptr->flags),
		    naptr->services_len,
		    (int)(naptr->services_len), ZSW(naptr->services), naptr->regexp_len,
		    (int)(naptr->regexp_len), ZSW(naptr->regexp));

		if (sip_match(naptr, &service) == 0) continue;

		if (parse_naptr_regexp(&(naptr->regexp[0]), naptr->regexp_len,
				       &pattern, &replacement) < 0) {
			LOG(L_ERR, "enum_query(): parsing of NAPTR regexp failed\n");
			continue;
		}
		result.s = &(uri[0]);
		result.len = MAX_URI_SIZE;
		/* Avoid making copies of pattern and replacement */
		pattern.s[pattern.len] = (char)0;
		replacement.s[replacement.len] = (char)0;
		if (reg_replace(pattern.s, replacement.s, &(string[0]),
				&result) < 0) {
			pattern.s[pattern.len] = '!';
			replacement.s[replacement.len] = '!';
			LOG(L_ERR, "enum_query(): regexp replace failed\n");
			continue;
		}
		DBG("enum_query(): resulted in replacement: '%.*s'\n",
		    result.len, ZSW(result.s));
		pattern.s[pattern.len] = '!';
		replacement.s[replacement.len] = '!';
		
		if (tel_uri_params.len > 0) {
			if (result.len + tel_uri_params.len > MAX_URI_SIZE - 1) {
				LOG(L_ERR, "ERROR: enum_query(): URI is too long\n");
				continue;
			}
			new_result.s = &(new_uri[0]);
			new_result.len = MAX_URI_SIZE;
			if (add_uri_param(&result, &tel_uri_params, &new_result) == 0) {
				LOG(L_ERR, "ERROR: enum_query(): Parsing of URI failed\n");
				continue;
			}
			if (new_result.len > 0) {
				result = new_result;
			}
		}

		if (first) {
			if (rewrite_uri(msg, &result) == -1) {
				goto done;
			}
			set_ruri_q(q);
			first = 0;
			curr_prio = ((naptr->order) << 16) + naptr->pref;
		} else {
			priority = ((naptr->order) << 16) + naptr->pref;
			if (priority > curr_prio) {
				q = q - 10;
				curr_prio = priority;
			}
			if (append_branch(msg, result.s, result.len, 0, 0, q, 0) == -1) {
				goto done;
			}
		}
	}

done:
	free_rdata_list(head);
	return first ? -1 : 1;
}

str s_asserted_identity={"P-Asserted-Identity",19};
/**
 * Looks for the P-Asserted-Identity header and extracts its content
 * @param msg - the sip message
 * @returns the asserted identity
 */
str get_asserted_identity(struct sip_msg *msg)
{
	name_addr_t id;
	struct hdr_field *h;
	rr_t *r;
	memset(&id,0,sizeof(name_addr_t));
	if (!msg) return id.uri;
	if (parse_headers(msg, HDR_EOH_F, 0)<0) {
		return id.uri;
	}
	h = msg->headers;
	while(h)
	{
		if (h->name.len == s_asserted_identity.len  &&
			strncasecmp(h->name.s,s_asserted_identity.s,s_asserted_identity.len)==0)
		{
			if (parse_rr(h)<0){
				//This might be an old client
				LOG(L_CRIT,"WARN:enum:get_asserted_identity: P-Asserted-Identity header must contain a Nameaddr!!! Fix the client!\n");
				id.name.s = h->body.s;
				id.name.len = 0;
				id.len = h->body.len;
				id.uri = h->body;
				while(id.uri.len && (id.uri.s[0]==' ' || id.uri.s[0]=='\t' || id.uri.s[0]=='<')){
					id.uri.s = id.uri.s+1;
					id.uri.len --;
				}
				while(id.uri.len && (id.uri.s[id.uri.len-1]==' ' || id.uri.s[id.uri.len-1]=='\t' || id.uri.s[id.uri.len-1]=='>')){
					id.uri.len--;
				}
				return id.uri;	
			}
			r = (rr_t*) h->parsed;
			id = r->nameaddr;			
			free_rr(&r);
			h->parsed=r;
			//LOG(L_RIT,"%.*s",id.uri.len,id.uri.s);
			return id.uri;
		}
		h = h->next;
	}
	return id.uri;
}

/**
 *	Get the public identity from P-Asserted-Identity, or From if asserted not found.
 * @param msg - the SIP message
 * @param uri - uri to fill into
 * @returns 1 if found, 0 if not
 */
int get_originating_user( struct sip_msg * msg, str *uri )
{
	struct to_body * from;
	*uri = get_asserted_identity(msg);
	if (!uri->len) {		
		/* Fallback to From header */
		if ( parse_from_header( msg ) == -1 ) {
			LOG(L_ERR,"ERROR:enum:get_originating_user: unable to extract URI from FROM header\n" );
			return 0;
		}
		if (!msg->from) return 0;
		from = (struct to_body*) msg->from->parsed;
		*uri = from->uri;
		//isc_strip_uri(uri);
	}
	DBG("DEBUG:enum:get_originating_user: From %.*s\n", uri->len,uri->s );
	return 1;
}

int set_dst_uri(struct sip_msg* msg, str* uri)
{
		/* change destination so it forwards to the app server */
	if (msg->dst_uri.s) pkg_free(msg->dst_uri.s);
	msg->dst_uri.s = pkg_malloc(uri->len);
	if (!msg->dst_uri.s) {
		LOG(L_ERR,"ERR:enum:set_dst_uri(): error allocating %d bytes\n",uri->len);
		return -1;
	}
	msg->dst_uri.len = uri->len;
	memcpy(msg->dst_uri.s,uri->s,uri->len);
	return 0;
}

inline int add_route(struct sip_msg *msg,str *uri)
{
	struct hdr_field *first;
	struct lump* anchor;
	str route;
	if (!uri || !uri->len) return -1;
	
	parse_headers(msg,HDR_EOH_F,0);			
	first = msg->headers;	
	route.s = pkg_malloc(21+uri->len);
	if (!route.s){
		LOG(L_ERR,"ERR:enum:add_route(): error allocating %d bytes\n",14+uri->len);
		return -1;
	}
	sprintf(route.s,"Route: <%.*s;lr>\r\n",uri->len,uri->s);
	
	route.len =strlen(route.s);
	LOG(L_DBG,"DEBUG:enum:add_route: <%.*s>\n",route.len,route.s);
	
	anchor = anchor_lump(msg, first->name.s - msg->buf, 0 , HDR_ROUTE_T);
	if (anchor == NULL) {
		LOG(L_ERR, "ERROR:enum:add_route: anchor_lump failed\n");
		if (route.s) pkg_free(route.s);
		return -1;
	}
	if (!insert_new_lump_before(anchor, route.s,route.len,HDR_ROUTE_T)){
		LOG( L_ERR, "ERROR:enum:add_route: error creating lump for Route header\n" );
		if (route.s) pkg_free(route.s);
		return -1;
	}	
	return 0;
}
int enum_query_orig(struct sip_msg* msg, char* p1, char* p2)
{
	char *user_s;
	int user_len, i, j, first;
	char name[MAX_DOMAIN_SIZE];
	char uri[MAX_URI_SIZE];
	char new_uri[MAX_URI_SIZE];
	unsigned int priority, curr_prio;
	qvalue_t q;
	str user={0,0};

	struct rdata* head;
	struct rdata* l;
	struct naptr_rdata* naptr;

	str pattern, replacement, result, new_result;

	char string[17];

	str suffix, service;

	if (p1) {
	    if (get_str_fparam(&suffix, msg, (fparam_t*)p1) < 0) {
		ERR("Unable to get suffix value\n");
		return -1;
	    }
	} else {
	    suffix = domain_suffix;
	}

	if (p2) {
	    if (get_str_fparam(&service, msg, (fparam_t*)p2) < 0) {
		ERR("Unable to get service value\n");
		return -1;
	    }
	} else {
	    service = default_service;
	}
	
	if (!get_originating_user(msg,&user)){
		LOG(L_ERR, "enum_query_orig(): originating uri extraction failed\n");
		return -1;
	}

	if (test_e164(&user) == -1) {
		LOG(L_ERR, "enum_query_orig(): uri user is not an E164 number\n");
		return -1;
	}

	user_s = user.s;
	user_len = user.len;

	memcpy(&(string[0]), user_s, user_len);
	string[user_len] = (char)0;

	j = 0;
	for (i = user_len - 1; i > 0; i--) {
		name[j] = user_s[i];
		name[j + 1] = '.';
		j = j + 2;
	}

	memcpy(name + j, suffix.s, suffix.len + 1);

	head = get_record(name, T_NAPTR, RES_ONLY_TYPE);

	if (head == 0) {
		DBG("enum_query_orig(): No NAPTR record found for %s.\n", name);
		return -1;
	}

	naptr_sort(&head);

	q = MAX_Q - 10;
	curr_prio = 0;
	first = 1;

	for (l = head; l; l = l->next) {

		if (l->type != T_NAPTR) continue; /*should never happen*/
		naptr = (struct naptr_rdata*)l->rdata;
		if (naptr == 0) {
			LOG(L_CRIT, "enum_query_orig: BUG: null rdata\n");
			continue;
		}

		DBG("enum_query_orig(): order %u, pref %u, flen %u, flags '%.*s', slen %u, "
		    "services '%.*s', rlen %u, regexp '%.*s'\n", naptr->order, naptr->pref,
		    naptr->flags_len, (int)(naptr->flags_len), ZSW(naptr->flags),
		    naptr->services_len,
		    (int)(naptr->services_len), ZSW(naptr->services), naptr->regexp_len,
		    (int)(naptr->regexp_len), ZSW(naptr->regexp));

		if (sip_match(naptr, &service) == 0) continue;

		if (parse_naptr_regexp(&(naptr->regexp[0]), naptr->regexp_len,
				       &pattern, &replacement) < 0) {
			LOG(L_ERR, "enum_query_orig(): parsing of NAPTR regexp failed\n");
			continue;
		}
		result.s = &(uri[0]);
		result.len = MAX_URI_SIZE;
		/* Avoid making copies of pattern and replacement */
		pattern.s[pattern.len] = (char)0;
		replacement.s[replacement.len] = (char)0;
		if (reg_replace(pattern.s, replacement.s, &(string[0]),
				&result) < 0) {
			pattern.s[pattern.len] = '!';
			replacement.s[replacement.len] = '!';
			LOG(L_ERR, "enum_query_orig(): regexp replace failed\n");
			continue;
		}
		DBG("enum_query(): resulted in replacement: '%.*s'\n",
		    result.len, ZSW(result.s));
		pattern.s[pattern.len] = '!';
		replacement.s[replacement.len] = '!';
		
		if (tel_uri_params_orig.len > 0) {
			if (result.len + tel_uri_params_orig.len > MAX_URI_SIZE - 1) {
				LOG(L_ERR, "ERROR: enum_query_orig(): URI is too long\n");
				continue;
			}
			new_result.s = &(new_uri[0]);
			new_result.len = MAX_URI_SIZE;
			if (add_uri_param(&result, &tel_uri_params_orig, &new_result) == 0) {
				LOG(L_ERR, "ERROR: enum_query_orig(): Parsing of URI failed\n");
				continue;
			}
			if (new_result.len > 0) {
				result = new_result;
			}
		}

		if (first) {
			if (set_dst_uri(msg, &result) == -1) {
				goto done;
			}
			/*if (rewrite_uri(msg, &result) == -1) {
				goto done;
			}
			set_ruri_q(q);
			*/
			first = 0;
			curr_prio = ((naptr->order) << 16) + naptr->pref;
			
		} else {
			// Not supported yet
			priority = ((naptr->order) << 16) + naptr->pref;
			if (priority > curr_prio) {
				q = q - 10;
				curr_prio = priority;
			}
			/*
			if (append_branch(msg, result.s, result.len, 0, 0, q, 0) == -1) {
				goto done;
			}*/
		}
	}

done:
	free_rdata_list(head);
	return first ? -1 : 1;
}


