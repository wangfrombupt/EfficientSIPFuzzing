
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     FORWARD = 258,
     FORWARD_TCP = 259,
     FORWARD_TLS = 260,
     FORWARD_UDP = 261,
     SEND = 262,
     SEND_TCP = 263,
     DROP = 264,
     RETURN = 265,
     BREAK = 266,
     LOG_TOK = 267,
     ERROR = 268,
     ROUTE = 269,
     ROUTE_FAILURE = 270,
     ROUTE_ONREPLY = 271,
     ROUTE_BRANCH = 272,
     ROUTE_SEND = 273,
     EXEC = 274,
     SET_HOST = 275,
     SET_HOSTPORT = 276,
     PREFIX = 277,
     STRIP = 278,
     STRIP_TAIL = 279,
     APPEND_BRANCH = 280,
     SET_USER = 281,
     SET_USERPASS = 282,
     SET_PORT = 283,
     SET_URI = 284,
     REVERT_URI = 285,
     FORCE_RPORT = 286,
     FORCE_TCP_ALIAS = 287,
     IF = 288,
     ELSE = 289,
     SET_ADV_ADDRESS = 290,
     SET_ADV_PORT = 291,
     FORCE_SEND_SOCKET = 292,
     URIHOST = 293,
     URIPORT = 294,
     MAX_LEN = 295,
     SETFLAG = 296,
     RESETFLAG = 297,
     ISFLAGSET = 298,
     SETAVPFLAG = 299,
     RESETAVPFLAG = 300,
     ISAVPFLAGSET = 301,
     METHOD = 302,
     URI = 303,
     FROM_URI = 304,
     TO_URI = 305,
     SRCIP = 306,
     SRCPORT = 307,
     DSTIP = 308,
     DSTPORT = 309,
     TOIP = 310,
     TOPORT = 311,
     SNDIP = 312,
     SNDPORT = 313,
     SNDPROTO = 314,
     SNDAF = 315,
     PROTO = 316,
     AF = 317,
     MYSELF = 318,
     MSGLEN = 319,
     RETCODE = 320,
     UDP = 321,
     TCP = 322,
     TLS = 323,
     DEBUG_V = 324,
     FORK = 325,
     LOGSTDERROR = 326,
     LOGFACILITY = 327,
     LISTEN = 328,
     ALIAS = 329,
     DNS = 330,
     REV_DNS = 331,
     DNS_TRY_IPV6 = 332,
     DNS_RETR_TIME = 333,
     DNS_RETR_NO = 334,
     DNS_SERVERS_NO = 335,
     DNS_USE_SEARCH = 336,
     DNS_USE_CACHE = 337,
     DNS_USE_FAILOVER = 338,
     DNS_CACHE_FLAGS = 339,
     DNS_CACHE_NEG_TTL = 340,
     DNS_CACHE_MIN_TTL = 341,
     DNS_CACHE_MAX_TTL = 342,
     DNS_CACHE_MEM = 343,
     DNS_CACHE_GC_INT = 344,
     USE_DST_BLST = 345,
     DST_BLST_MEM = 346,
     DST_BLST_TTL = 347,
     DST_BLST_GC_INT = 348,
     PORT = 349,
     STAT = 350,
     CHILDREN = 351,
     CHECK_VIA = 352,
     SYN_BRANCH = 353,
     MEMLOG = 354,
     MEMDBG = 355,
     SIP_WARNING = 356,
     SERVER_SIGNATURE = 357,
     REPLY_TO_VIA = 358,
     LOADMODULE = 359,
     MODPARAM = 360,
     MAXBUFFER = 361,
     USER = 362,
     GROUP = 363,
     CHROOT = 364,
     WDIR = 365,
     MHOMED = 366,
     DISABLE_TCP = 367,
     TCP_ACCEPT_ALIASES = 368,
     TCP_CHILDREN = 369,
     TCP_CONNECT_TIMEOUT = 370,
     TCP_SEND_TIMEOUT = 371,
     TCP_CON_LIFETIME = 372,
     TCP_POLL_METHOD = 373,
     TCP_MAX_CONNECTIONS = 374,
     DISABLE_TLS = 375,
     ENABLE_TLS = 376,
     TLSLOG = 377,
     TLS_PORT_NO = 378,
     TLS_METHOD = 379,
     TLS_HANDSHAKE_TIMEOUT = 380,
     TLS_SEND_TIMEOUT = 381,
     SSLv23 = 382,
     SSLv2 = 383,
     SSLv3 = 384,
     TLSv1 = 385,
     TLS_VERIFY = 386,
     TLS_REQUIRE_CERTIFICATE = 387,
     TLS_CERTIFICATE = 388,
     TLS_PRIVATE_KEY = 389,
     TLS_CA_LIST = 390,
     ADVERTISED_ADDRESS = 391,
     ADVERTISED_PORT = 392,
     DISABLE_CORE = 393,
     OPEN_FD_LIMIT = 394,
     MCAST_LOOPBACK = 395,
     MCAST_TTL = 396,
     TOS = 397,
     KILL_TIMEOUT = 398,
     FLAGS_DECL = 399,
     AVPFLAGS_DECL = 400,
     ATTR_MARK = 401,
     SELECT_MARK = 402,
     ATTR_FROM = 403,
     ATTR_TO = 404,
     ATTR_FROMURI = 405,
     ATTR_TOURI = 406,
     ATTR_FROMUSER = 407,
     ATTR_TOUSER = 408,
     ATTR_FROMDOMAIN = 409,
     ATTR_TODOMAIN = 410,
     ATTR_GLOBAL = 411,
     ADDEQ = 412,
     STUN_REFRESH_INTERVAL = 413,
     STUN_ALLOW_STUN = 414,
     STUN_ALLOW_FP = 415,
     EQUAL = 416,
     EQUAL_T = 417,
     GT = 418,
     LT = 419,
     GTE = 420,
     LTE = 421,
     DIFF = 422,
     MATCH = 423,
     LOG_OR = 424,
     LOG_AND = 425,
     BIN_OR = 426,
     BIN_AND = 427,
     MINUS = 428,
     PLUS = 429,
     NOT = 430,
     NUMBER = 431,
     ID = 432,
     STRING = 433,
     IPV6ADDR = 434,
     COMMA = 435,
     SEMICOLON = 436,
     RPAREN = 437,
     LPAREN = 438,
     LBRACE = 439,
     RBRACE = 440,
     LBRACK = 441,
     RBRACK = 442,
     SLASH = 443,
     DOT = 444,
     CR = 445,
     COLON = 446,
     STAR = 447
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 171 "cfg.y"

	long intval;
	unsigned long uval;
	char* strval;
	struct expr* expr;
	struct action* action;
	struct net* ipnet;
	struct ip_addr* ipaddr;
	struct socket_id* sockid;
	struct avp_spec* attr;
	select_t* select;



/* Line 1676 of yacc.c  */
#line 259 "cfg.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


