
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 83 "cfg.y"


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "route_struct.h"
#include "globals.h"
#include "route.h"
#include "dprint.h"
#include "sr_module.h"
#include "modparam.h"
#include "ip_addr.h"
#include "resolve.h"
#include "socket_info.h"
#include "name_alias.h"
#include "ut.h"
#include "dset.h"
#include "select.h"
#include "flags.h"

#include "config.h"
#ifdef CORE_TLS
#include "tls/tls_config.h"
#endif

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

/* hack to avoid alloca usage in the generated C file (needed for compiler
 with no built in alloca, like icc*/
#undef _ALLOCA_H

#define onsend_check(s) \
	do{\
		if (rt!=ONSEND_ROUTE) yyerror( s " allowed only in onsend_routes");\
	}while(0)


#ifdef USE_DNS_CACHE
	#define IF_DNS_CACHE(x) x
#else
	#define IF_DNS_CACHE(x) warn("dns cache support not compiled in")
#endif

#ifdef USE_DNS_FAILOVER
	#define IF_DNS_FAILOVER(x) x
#else
	#define IF_DNS_FAILOVER(x) warn("dns failover support not compiled in")
#endif

#ifdef USE_DST_BLACKLIST
	#define IF_DST_BLACKLIST(x) x
#else
	#define IF_DST_BLACKLIST(x) warn("dst blacklist support not compiled in")
#endif

#ifdef USE_STUN
	#define IF_STUN(x) x
#else 
	#define IF_STUN(x) warn("stun support not compiled in")
#endif


extern int yylex();
static void yyerror(char* s);
static char* tmp;
static int i_tmp;
static struct socket_id* lst_tmp;
static int rt;  /* Type of route block for find_export */
static str* str_tmp;
static str s_tmp;
static struct ip_addr* ip_tmp;
static struct avp_spec* s_attr;
static select_t sel;
static select_t* sel_ptr;
static struct action *mod_func_action;

static void warn(char* s);
static struct socket_id* mk_listen_id(char*, int, int);



/* Line 189 of yacc.c  */
#line 162 "cfg.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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

/* Line 214 of yacc.c  */
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



/* Line 214 of yacc.c  */
#line 405 "cfg.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 417 "cfg.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  196
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2182

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  193
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  71
/* YYNRULES -- Number of rules.  */
#define YYNRULES  562
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1048

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   447

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    13,    15,    17,    19,
      21,    22,    25,    26,    29,    30,    33,    34,    37,    38,
      41,    43,    45,    47,    49,    51,    53,    55,    57,    59,
      61,    63,    65,    69,    73,    79,    83,    85,    88,    91,
      94,    96,   100,   102,   106,   108,   110,   113,   116,   118,
     122,   124,   128,   132,   136,   140,   144,   148,   152,   156,
     160,   164,   168,   172,   176,   179,   183,   186,   190,   193,
     197,   200,   204,   207,   211,   214,   218,   221,   225,   228,
     232,   235,   239,   242,   246,   249,   253,   256,   260,   263,
     267,   270,   274,   277,   281,   284,   288,   291,   295,   299,
     303,   307,   311,   315,   319,   323,   327,   331,   335,   339,
     343,   347,   351,   355,   359,   363,   367,   371,   375,   379,
     383,   387,   391,   395,   399,   403,   407,   411,   415,   419,
     423,   427,   431,   435,   439,   443,   447,   451,   455,   459,
     463,   467,   471,   475,   479,   483,   487,   491,   495,   499,
     503,   507,   511,   515,   519,   523,   527,   531,   535,   539,
     543,   547,   551,   555,   559,   563,   567,   571,   575,   579,
     583,   587,   591,   595,   599,   603,   607,   611,   615,   619,
     623,   627,   631,   635,   639,   643,   647,   651,   655,   659,
     663,   667,   671,   675,   679,   683,   687,   691,   695,   699,
     703,   707,   711,   714,   717,   720,   729,   738,   741,   743,
     745,   753,   755,   757,   761,   763,   765,   767,   772,   780,
     783,   788,   796,   799,   804,   812,   815,   820,   828,   831,
     836,   844,   847,   851,   855,   858,   862,   864,   866,   868,
     870,   872,   874,   876,   878,   880,   882,   884,   886,   888,
     890,   892,   896,   900,   904,   908,   912,   915,   919,   923,
     927,   931,   935,   939,   942,   946,   950,   954,   957,   961,
     965,   969,   972,   976,   980,   984,   987,   991,   995,   999,
    1002,  1006,  1010,  1014,  1017,  1021,  1025,  1029,  1032,  1036,
    1040,  1044,  1047,  1051,  1055,  1059,  1062,  1066,  1070,  1074,
    1078,  1081,  1085,  1089,  1093,  1096,  1100,  1104,  1108,  1112,
    1116,  1119,  1123,  1127,  1131,  1135,  1139,  1142,  1146,  1150,
    1154,  1158,  1162,  1165,  1169,  1173,  1177,  1181,  1185,  1188,
    1192,  1196,  1200,  1204,  1208,  1212,  1215,  1217,  1219,  1221,
    1225,  1229,  1233,  1237,  1241,  1243,  1247,  1251,  1255,  1259,
    1263,  1265,  1269,  1271,  1273,  1275,  1279,  1283,  1285,  1287,
    1289,  1291,  1295,  1297,  1301,  1304,  1306,  1309,  1312,  1314,
    1317,  1319,  1322,  1326,  1332,  1334,  1339,  1344,  1348,  1350,
    1351,  1355,  1357,  1359,  1361,  1363,  1365,  1367,  1369,  1371,
    1373,  1375,  1377,  1381,  1383,  1386,  1392,  1397,  1399,  1401,
    1403,  1405,  1407,  1409,  1411,  1413,  1415,  1417,  1421,  1425,
    1429,  1433,  1437,  1443,  1445,  1447,  1449,  1454,  1459,  1464,
    1471,  1478,  1485,  1492,  1499,  1504,  1507,  1512,  1517,  1522,
    1527,  1534,  1541,  1548,  1555,  1562,  1567,  1570,  1575,  1580,
    1585,  1590,  1597,  1604,  1611,  1618,  1625,  1630,  1633,  1638,
    1643,  1648,  1653,  1660,  1667,  1674,  1681,  1688,  1693,  1696,
    1701,  1706,  1711,  1716,  1723,  1730,  1737,  1740,  1745,  1750,
    1755,  1760,  1767,  1774,  1781,  1784,  1789,  1793,  1798,  1801,
    1804,  1806,  1808,  1811,  1814,  1816,  1821,  1828,  1831,  1836,
    1841,  1846,  1849,  1854,  1859,  1862,  1867,  1872,  1875,  1882,
    1885,  1892,  1895,  1900,  1905,  1908,  1913,  1918,  1923,  1926,
    1931,  1936,  1939,  1944,  1949,  1952,  1957,  1962,  1965,  1970,
    1977,  1982,  1986,  1988,  1993,  1996,  2001,  2006,  2009,  2014,
    2019,  2022,  2027,  2032,  2035,  2040,  2045,  2048,  2053,  2057,
    2059,  2063,  2065,  2070,  2074,  2076,  2081,  2086,  2091,  2094,
    2099,  2104,  2107,  2112,  2117,  2120,  2121,  2127,  2128,  2132,
    2134,  2137,  2139
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     194,     0,    -1,   195,    -1,   195,   196,    -1,   196,    -1,
     195,     1,    -1,   214,    -1,   207,    -1,   211,    -1,   215,
      -1,    -1,   197,   221,    -1,    -1,   198,   222,    -1,    -1,
     199,   223,    -1,    -1,   200,   224,    -1,    -1,   201,   225,
      -1,   181,    -1,   190,    -1,   216,    -1,   178,    -1,   235,
      -1,    66,    -1,    67,    -1,    68,    -1,   192,    -1,   176,
      -1,   192,    -1,   202,    -1,   202,   191,   204,    -1,   203,
     191,   202,    -1,   203,   191,   202,   191,   204,    -1,   202,
     191,     1,    -1,   205,    -1,   205,   206,    -1,   144,   208,
      -1,   144,     1,    -1,   209,    -1,   209,   180,   208,    -1,
     210,    -1,   210,   191,   176,    -1,   178,    -1,   177,    -1,
     145,   212,    -1,   145,     1,    -1,   213,    -1,   213,   180,
     212,    -1,   210,    -1,    69,   161,   176,    -1,    69,   161,
       1,    -1,    70,   161,   176,    -1,    70,   161,     1,    -1,
      71,   161,   176,    -1,    71,   161,     1,    -1,    72,   161,
     177,    -1,    72,   161,     1,    -1,    75,   161,   176,    -1,
      75,   161,     1,    -1,    76,   161,   176,    -1,    76,   161,
       1,    -1,    77,   161,   176,    -1,    77,     1,    -1,    78,
     161,   176,    -1,    78,     1,    -1,    79,   161,   176,    -1,
      79,     1,    -1,    80,   161,   176,    -1,    80,     1,    -1,
      81,   161,   176,    -1,    81,     1,    -1,    82,   161,   176,
      -1,    82,     1,    -1,    83,   161,   176,    -1,    83,     1,
      -1,    84,   161,   176,    -1,    84,     1,    -1,    85,   161,
     176,    -1,    85,     1,    -1,    87,   161,   176,    -1,    87,
       1,    -1,    86,   161,   176,    -1,    86,     1,    -1,    88,
     161,   176,    -1,    88,     1,    -1,    89,   161,   176,    -1,
      89,     1,    -1,    90,   161,   176,    -1,    90,     1,    -1,
      91,   161,   176,    -1,    91,     1,    -1,    92,   161,   176,
      -1,    92,     1,    -1,    93,   161,   176,    -1,    93,     1,
      -1,    94,   161,   176,    -1,    95,   161,   178,    -1,   106,
     161,   176,    -1,   106,   161,     1,    -1,    94,   161,     1,
      -1,    96,   161,   176,    -1,    96,   161,     1,    -1,    97,
     161,   176,    -1,    97,   161,     1,    -1,    98,   161,   176,
      -1,    98,   161,     1,    -1,    99,   161,   176,    -1,    99,
     161,     1,    -1,   100,   161,   176,    -1,   100,   161,     1,
      -1,   101,   161,   176,    -1,   101,   161,     1,    -1,   107,
     161,   178,    -1,   107,   161,   177,    -1,   107,   161,     1,
      -1,   108,   161,   178,    -1,   108,   161,   177,    -1,   108,
     161,     1,    -1,   109,   161,   178,    -1,   109,   161,   177,
      -1,   109,   161,     1,    -1,   110,   161,   178,    -1,   110,
     161,   177,    -1,   110,   161,     1,    -1,   111,   161,   176,
      -1,   111,   161,     1,    -1,   112,   161,   176,    -1,   112,
     161,     1,    -1,   113,   161,   176,    -1,   113,   161,     1,
      -1,   114,   161,   176,    -1,   114,   161,     1,    -1,   115,
     161,   176,    -1,   115,   161,     1,    -1,   116,   161,   176,
      -1,   116,   161,     1,    -1,   117,   161,   176,    -1,   117,
     161,     1,    -1,   118,   161,   177,    -1,   118,   161,   178,
      -1,   118,   161,     1,    -1,   119,   161,   176,    -1,   119,
     161,     1,    -1,   120,   161,   176,    -1,   120,   161,     1,
      -1,   121,   161,   176,    -1,   121,   161,     1,    -1,   122,
     161,   176,    -1,   122,   161,     1,    -1,   123,   161,   176,
      -1,   123,   161,     1,    -1,   124,   161,   127,    -1,   124,
     161,   128,    -1,   124,   161,   129,    -1,   124,   161,   130,
      -1,   124,   161,     1,    -1,   131,   161,   176,    -1,   131,
     161,     1,    -1,   132,   161,   176,    -1,   132,   161,     1,
      -1,   133,   161,   178,    -1,   133,   161,     1,    -1,   134,
     161,   178,    -1,   134,   161,     1,    -1,   135,   161,   178,
      -1,   135,   161,     1,    -1,   125,   161,   176,    -1,   125,
     161,     1,    -1,   126,   161,   176,    -1,   126,   161,     1,
      -1,   102,   161,   176,    -1,   102,   161,     1,    -1,   103,
     161,   176,    -1,   103,   161,     1,    -1,    73,   161,   206,
      -1,    73,   161,     1,    -1,    74,   161,   206,    -1,    74,
     161,     1,    -1,   136,   161,   202,    -1,   136,   161,     1,
      -1,   137,   161,   176,    -1,   137,   161,     1,    -1,   138,
     161,   176,    -1,   138,   161,     1,    -1,   139,   161,   176,
      -1,   139,   161,     1,    -1,   140,   161,   176,    -1,   140,
     161,     1,    -1,   141,   161,   176,    -1,   141,   161,     1,
      -1,   142,   161,   176,    -1,   142,   161,     1,    -1,   143,
     161,   176,    -1,   143,   161,     1,    -1,   158,   161,   176,
      -1,   158,   161,     1,    -1,   159,   161,   176,    -1,   159,
     161,     1,    -1,   160,   161,   176,    -1,   160,   161,     1,
      -1,     1,   161,    -1,   104,   178,    -1,   104,     1,    -1,
     105,   183,   178,   180,   178,   180,   178,   182,    -1,   105,
     183,   178,   180,   178,   180,   176,   182,    -1,   105,     1,
      -1,   217,    -1,   219,    -1,   176,   189,   176,   189,   176,
     189,   176,    -1,   179,    -1,   218,    -1,   186,   218,   187,
      -1,   176,    -1,   177,    -1,   178,    -1,    14,   184,   239,
     185,    -1,    14,   186,   220,   187,   184,   239,   185,    -1,
      14,     1,    -1,    15,   184,   239,   185,    -1,    15,   186,
     220,   187,   184,   239,   185,    -1,    15,     1,    -1,    16,
     184,   239,   185,    -1,    16,   186,   220,   187,   184,   239,
     185,    -1,    16,     1,    -1,    17,   184,   239,   185,    -1,
      17,   186,   220,   187,   184,   239,   185,    -1,    17,     1,
      -1,    18,   184,   239,   185,    -1,    18,   186,   220,   187,
     184,   239,   185,    -1,    18,     1,    -1,   226,   170,   226,
      -1,   226,   169,   226,    -1,   175,   226,    -1,   183,   226,
     182,    -1,   232,    -1,   162,    -1,   167,    -1,   227,    -1,
     163,    -1,   164,    -1,   165,    -1,   166,    -1,   171,    -1,
     172,    -1,   227,    -1,   168,    -1,    48,    -1,    49,    -1,
      50,    -1,    47,   230,   178,    -1,    47,   230,   254,    -1,
      47,   230,   244,    -1,    47,   230,   177,    -1,    47,   230,
       1,    -1,    47,     1,    -1,   231,   230,   178,    -1,   231,
     230,   235,    -1,   231,   230,   254,    -1,   231,   230,   244,
      -1,   231,   227,    63,    -1,   231,   230,     1,    -1,   231,
       1,    -1,    52,   228,   176,    -1,    52,   228,   254,    -1,
      52,   228,     1,    -1,    52,     1,    -1,    54,   228,   176,
      -1,    54,   228,   254,    -1,    54,   228,     1,    -1,    54,
       1,    -1,    58,   228,   176,    -1,    58,   228,   254,    -1,
      58,   228,     1,    -1,    58,     1,    -1,    56,   228,   176,
      -1,    56,   228,   254,    -1,    56,   228,     1,    -1,    56,
       1,    -1,    61,   228,   203,    -1,    61,   228,   254,    -1,
      61,   228,     1,    -1,    61,     1,    -1,    59,   228,   203,
      -1,    59,   228,   254,    -1,    59,   228,     1,    -1,    59,
       1,    -1,    62,   228,   176,    -1,    62,   228,   254,    -1,
      62,   228,     1,    -1,    62,     1,    -1,    60,   228,   176,
      -1,    60,   228,   254,    -1,    60,   228,     1,    -1,    60,
       1,    -1,    64,   228,   176,    -1,    64,   228,   254,    -1,
      64,   228,    40,    -1,    64,   228,     1,    -1,    64,     1,
      -1,    65,   228,   176,    -1,    65,   228,   254,    -1,    65,
     228,     1,    -1,    65,     1,    -1,    51,   227,   233,    -1,
      51,   230,   178,    -1,    51,   230,   235,    -1,    51,   227,
      63,    -1,    51,   230,     1,    -1,    51,     1,    -1,    53,
     227,   233,    -1,    53,   230,   178,    -1,    53,   230,   235,
      -1,    53,   227,    63,    -1,    53,   230,     1,    -1,    53,
       1,    -1,    57,   227,   233,    -1,    57,   230,   178,    -1,
      57,   230,   235,    -1,    57,   227,    63,    -1,    57,   230,
       1,    -1,    57,     1,    -1,    55,   227,   233,    -1,    55,
     230,   178,    -1,    55,   230,   235,    -1,    55,   227,    63,
      -1,    55,   230,     1,    -1,    55,     1,    -1,    63,   227,
     231,    -1,    63,   227,    51,    -1,    63,   227,    53,    -1,
      63,   227,    57,    -1,    63,   227,    55,    -1,    63,   227,
       1,    -1,    63,     1,    -1,   237,    -1,   176,    -1,   255,
      -1,   254,   230,   178,    -1,   254,   230,   244,    -1,   254,
     228,   176,    -1,   254,   229,   176,    -1,   254,   230,   254,
      -1,   244,    -1,   244,   230,   178,    -1,   244,   230,   254,
      -1,   244,   230,   244,    -1,   216,   188,   216,    -1,   216,
     188,   176,    -1,   216,    -1,   216,   188,     1,    -1,   189,
      -1,   173,    -1,   177,    -1,   235,   234,   177,    -1,   235,
     189,     1,    -1,   260,    -1,   236,    -1,   241,    -1,   258,
      -1,   184,   239,   185,    -1,   240,    -1,   184,   239,   185,
      -1,   239,   240,    -1,   240,    -1,   239,     1,    -1,   236,
     181,    -1,   241,    -1,   258,   181,    -1,   181,    -1,   236,
       1,    -1,    33,   226,   238,    -1,    33,   226,   238,    34,
     238,    -1,   177,    -1,   177,   186,   176,   187,    -1,   177,
     186,   178,   187,    -1,   243,   189,   242,    -1,   242,    -1,
      -1,   147,   245,   243,    -1,   148,    -1,   149,    -1,   150,
      -1,   151,    -1,   152,    -1,   153,    -1,   154,    -1,   155,
      -1,   156,    -1,   177,    -1,   247,    -1,   246,   189,   247,
      -1,   146,    -1,   249,   248,    -1,   249,   248,   186,   176,
     187,    -1,   249,   248,   186,   187,    -1,   250,    -1,   252,
      -1,   250,    -1,   251,    -1,   250,    -1,   252,    -1,   251,
      -1,   250,    -1,   178,    -1,   161,    -1,   253,   257,   178,
      -1,   253,   257,   176,    -1,   253,   257,   236,    -1,   253,
     257,   255,    -1,   253,   257,   244,    -1,   253,   257,   183,
     226,   182,    -1,    44,    -1,    45,    -1,    46,    -1,     3,
     183,   235,   182,    -1,     3,   183,   178,   182,    -1,     3,
     183,   216,   182,    -1,     3,   183,   235,   180,   176,   182,
      -1,     3,   183,   178,   180,   176,   182,    -1,     3,   183,
     216,   180,   176,   182,    -1,     3,   183,    38,   180,    39,
     182,    -1,     3,   183,    38,   180,   176,   182,    -1,     3,
     183,    38,   182,    -1,     3,     1,    -1,     3,   183,     1,
     182,    -1,     6,   183,   235,   182,    -1,     6,   183,   178,
     182,    -1,     6,   183,   216,   182,    -1,     6,   183,   235,
     180,   176,   182,    -1,     6,   183,   178,   180,   176,   182,
      -1,     6,   183,   216,   180,   176,   182,    -1,     6,   183,
      38,   180,    39,   182,    -1,     6,   183,    38,   180,   176,
     182,    -1,     6,   183,    38,   182,    -1,     6,     1,    -1,
       6,   183,     1,   182,    -1,     4,   183,   235,   182,    -1,
       4,   183,   178,   182,    -1,     4,   183,   216,   182,    -1,
       4,   183,   235,   180,   176,   182,    -1,     4,   183,   178,
     180,   176,   182,    -1,     4,   183,   216,   180,   176,   182,
      -1,     4,   183,    38,   180,    39,   182,    -1,     4,   183,
      38,   180,   176,   182,    -1,     4,   183,    38,   182,    -1,
       4,     1,    -1,     4,   183,     1,   182,    -1,     5,   183,
     235,   182,    -1,     5,   183,   178,   182,    -1,     5,   183,
     216,   182,    -1,     5,   183,   235,   180,   176,   182,    -1,
       5,   183,   178,   180,   176,   182,    -1,     5,   183,   216,
     180,   176,   182,    -1,     5,   183,    38,   180,    39,   182,
      -1,     5,   183,    38,   180,   176,   182,    -1,     5,   183,
      38,   182,    -1,     5,     1,    -1,     5,   183,     1,   182,
      -1,     7,   183,   235,   182,    -1,     7,   183,   178,   182,
      -1,     7,   183,   216,   182,    -1,     7,   183,   235,   180,
     176,   182,    -1,     7,   183,   178,   180,   176,   182,    -1,
       7,   183,   216,   180,   176,   182,    -1,     7,     1,    -1,
       7,   183,     1,   182,    -1,     8,   183,   235,   182,    -1,
       8,   183,   178,   182,    -1,     8,   183,   216,   182,    -1,
       8,   183,   235,   180,   176,   182,    -1,     8,   183,   178,
     180,   176,   182,    -1,     8,   183,   216,   180,   176,   182,
      -1,     8,     1,    -1,     8,   183,     1,   182,    -1,     9,
     183,   182,    -1,     9,   183,   176,   182,    -1,     9,   176,
      -1,     9,    65,    -1,     9,    -1,    10,    -1,    10,   176,
      -1,    10,    65,    -1,    11,    -1,    12,   183,   178,   182,
      -1,    12,   183,   176,   180,   178,   182,    -1,    12,     1,
      -1,    12,   183,     1,   182,    -1,    41,   183,   176,   182,
      -1,    41,   183,   210,   182,    -1,    41,     1,    -1,    42,
     183,   176,   182,    -1,    42,   183,   210,   182,    -1,    42,
       1,    -1,    43,   183,   176,   182,    -1,    43,   183,   210,
     182,    -1,    43,     1,    -1,   259,   183,   256,   180,   210,
     182,    -1,   259,     1,    -1,    13,   183,   178,   180,   178,
     182,    -1,    13,     1,    -1,    13,   183,     1,   182,    -1,
      14,   183,   220,   182,    -1,    14,     1,    -1,    14,   183,
       1,   182,    -1,    19,   183,   178,   182,    -1,    20,   183,
     178,   182,    -1,    20,     1,    -1,    20,   183,     1,   182,
      -1,    22,   183,   178,   182,    -1,    22,     1,    -1,    22,
     183,     1,   182,    -1,    24,   183,   176,   182,    -1,    24,
       1,    -1,    24,   183,     1,   182,    -1,    23,   183,   176,
     182,    -1,    23,     1,    -1,    23,   183,     1,   182,    -1,
      25,   183,   178,   180,   178,   182,    -1,    25,   183,   178,
     182,    -1,    25,   183,   182,    -1,    25,    -1,    21,   183,
     178,   182,    -1,    21,     1,    -1,    21,   183,     1,   182,
      -1,    28,   183,   178,   182,    -1,    28,     1,    -1,    28,
     183,     1,   182,    -1,    26,   183,   178,   182,    -1,    26,
       1,    -1,    26,   183,     1,   182,    -1,    27,   183,   178,
     182,    -1,    27,     1,    -1,    27,   183,     1,   182,    -1,
      29,   183,   178,   182,    -1,    29,     1,    -1,    29,   183,
       1,   182,    -1,    30,   183,   182,    -1,    30,    -1,    31,
     183,   182,    -1,    31,    -1,    32,   183,   176,   182,    -1,
      32,   183,   182,    -1,    32,    -1,    32,   183,     1,   182,
      -1,    35,   183,   202,   182,    -1,    35,   183,     1,   182,
      -1,    35,     1,    -1,    36,   183,   176,   182,    -1,    36,
     183,     1,   182,    -1,    36,     1,    -1,    37,   183,   205,
     182,    -1,    37,   183,     1,   182,    -1,    37,     1,    -1,
      -1,   177,   261,   183,   262,   182,    -1,    -1,   262,   180,
     263,    -1,   263,    -1,   262,     1,    -1,   176,    -1,   178,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   427,   427,   430,   431,   432,   435,   436,   437,   438,
     439,   439,   440,   440,   441,   441,   442,   442,   443,   443,
     444,   445,   448,   464,   473,   484,   485,   486,   487,   490,
     491,   494,   495,   496,   497,   498,   501,   502,   505,   506,
     508,   509,   512,   515,   521,   522,   526,   527,   530,   531,
     534,   540,   541,   542,   543,   544,   545,   546,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
     564,   565,   566,   567,   568,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   578,   579,   580,   581,   582,   583,
     584,   585,   586,   587,   588,   589,   590,   591,   592,   597,
     598,   599,   600,   601,   602,   603,   604,   605,   606,   607,
     608,   609,   610,   611,   612,   613,   614,   615,   616,   617,
     618,   619,   620,   621,   622,   623,   624,   625,   626,   633,
     634,   641,   642,   649,   650,   657,   658,   665,   666,   673,
     674,   688,   702,   703,   710,   711,   718,   719,   726,   727,
     734,   735,   742,   743,   750,   757,   764,   771,   778,   785,
     786,   793,   794,   801,   802,   809,   810,   817,   818,   825,
     826,   833,   834,   835,   836,   837,   838,   846,   847,   851,
     852,   856,   857,   867,   868,   869,   870,   871,   872,   879,
     880,   887,   888,   889,   890,   891,   892,   893,   894,   895,
     896,   897,   898,   901,   907,   908,   913,   918,   921,   922,
     925,   954,   974,   975,   979,   989,   990,   994,   995,  1007,
    1010,  1013,  1025,  1028,  1031,  1043,  1046,  1049,  1061,  1063,
    1066,  1078,  1100,  1101,  1102,  1103,  1104,  1107,  1108,  1110,
    1111,  1112,  1113,  1114,  1117,  1118,  1121,  1122,  1125,  1126,
    1127,  1131,  1132,  1133,  1134,  1135,  1136,  1137,  1138,  1139,
    1140,  1141,  1142,  1143,  1145,  1146,  1147,  1148,  1150,  1151,
    1152,  1153,  1155,  1159,  1163,  1164,  1166,  1170,  1174,  1175,
    1177,  1178,  1179,  1181,  1183,  1187,  1191,  1192,  1194,  1195,
    1196,  1197,  1199,  1202,  1206,  1207,  1209,  1210,  1211,  1212,
    1213,  1215,  1216,  1217,  1218,  1220,  1221,  1233,  1234,  1236,
    1237,  1238,  1239,  1251,  1252,  1253,  1254,  1255,  1259,  1272,
    1276,  1280,  1281,  1282,  1286,  1299,  1303,  1307,  1308,  1310,
    1311,  1312,  1313,  1317,  1321,  1322,  1323,  1324,  1326,  1327,
    1328,  1329,  1330,  1331,  1333,  1334,  1335,  1336,  1339,  1340,
    1351,  1352,  1355,  1356,  1360,  1361,  1374,  1378,  1403,  1404,
    1405,  1406,  1409,  1410,  1413,  1414,  1415,  1418,  1419,  1420,
    1421,  1422,  1425,  1426,  1435,  1444,  1456,  1471,  1472,  1475,
    1475,  1485,  1486,  1487,  1488,  1489,  1490,  1491,  1492,  1493,
    1496,  1499,  1500,  1503,  1510,  1513,  1520,  1526,  1527,  1530,
    1531,  1534,  1535,  1536,  1539,  1540,  1570,  1573,  1574,  1575,
    1576,  1577,  1578,  1581,  1582,  1583,  1586,  1587,  1588,  1589,
    1590,  1591,  1592,  1593,  1594,  1595,  1596,  1597,  1598,  1599,
    1600,  1601,  1602,  1603,  1604,  1605,  1606,  1607,  1608,  1609,
    1610,  1611,  1612,  1613,  1614,  1615,  1616,  1617,  1618,  1619,
    1627,  1635,  1643,  1651,  1659,  1667,  1675,  1683,  1691,  1692,
    1693,  1694,  1695,  1696,  1697,  1698,  1699,  1700,  1701,  1702,
    1703,  1704,  1705,  1706,  1707,  1708,  1709,  1710,  1711,  1712,
    1713,  1714,  1715,  1716,  1717,  1718,  1719,  1720,  1721,  1722,
    1728,  1734,  1735,  1740,  1746,  1747,  1752,  1758,  1759,  1764,
    1765,  1766,  1767,  1768,  1776,  1777,  1778,  1779,  1780,  1781,
    1782,  1783,  1784,  1785,  1786,  1787,  1788,  1789,  1790,  1791,
    1798,  1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,  1807,
    1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,
    1818,  1819,  1820,  1827,  1834,  1841,  1842,  1852,  1853,  1854,
    1869,  1870,  1871,  1872,  1873,  1874,  1874,  1888,  1890,  1891,
    1892,  1895,  1904
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FORWARD", "FORWARD_TCP", "FORWARD_TLS",
  "FORWARD_UDP", "SEND", "SEND_TCP", "DROP", "RETURN", "BREAK", "LOG_TOK",
  "ERROR", "ROUTE", "ROUTE_FAILURE", "ROUTE_ONREPLY", "ROUTE_BRANCH",
  "ROUTE_SEND", "EXEC", "SET_HOST", "SET_HOSTPORT", "PREFIX", "STRIP",
  "STRIP_TAIL", "APPEND_BRANCH", "SET_USER", "SET_USERPASS", "SET_PORT",
  "SET_URI", "REVERT_URI", "FORCE_RPORT", "FORCE_TCP_ALIAS", "IF", "ELSE",
  "SET_ADV_ADDRESS", "SET_ADV_PORT", "FORCE_SEND_SOCKET", "URIHOST",
  "URIPORT", "MAX_LEN", "SETFLAG", "RESETFLAG", "ISFLAGSET", "SETAVPFLAG",
  "RESETAVPFLAG", "ISAVPFLAGSET", "METHOD", "URI", "FROM_URI", "TO_URI",
  "SRCIP", "SRCPORT", "DSTIP", "DSTPORT", "TOIP", "TOPORT", "SNDIP",
  "SNDPORT", "SNDPROTO", "SNDAF", "PROTO", "AF", "MYSELF", "MSGLEN",
  "RETCODE", "UDP", "TCP", "TLS", "DEBUG_V", "FORK", "LOGSTDERROR",
  "LOGFACILITY", "LISTEN", "ALIAS", "DNS", "REV_DNS", "DNS_TRY_IPV6",
  "DNS_RETR_TIME", "DNS_RETR_NO", "DNS_SERVERS_NO", "DNS_USE_SEARCH",
  "DNS_USE_CACHE", "DNS_USE_FAILOVER", "DNS_CACHE_FLAGS",
  "DNS_CACHE_NEG_TTL", "DNS_CACHE_MIN_TTL", "DNS_CACHE_MAX_TTL",
  "DNS_CACHE_MEM", "DNS_CACHE_GC_INT", "USE_DST_BLST", "DST_BLST_MEM",
  "DST_BLST_TTL", "DST_BLST_GC_INT", "PORT", "STAT", "CHILDREN",
  "CHECK_VIA", "SYN_BRANCH", "MEMLOG", "MEMDBG", "SIP_WARNING",
  "SERVER_SIGNATURE", "REPLY_TO_VIA", "LOADMODULE", "MODPARAM",
  "MAXBUFFER", "USER", "GROUP", "CHROOT", "WDIR", "MHOMED", "DISABLE_TCP",
  "TCP_ACCEPT_ALIASES", "TCP_CHILDREN", "TCP_CONNECT_TIMEOUT",
  "TCP_SEND_TIMEOUT", "TCP_CON_LIFETIME", "TCP_POLL_METHOD",
  "TCP_MAX_CONNECTIONS", "DISABLE_TLS", "ENABLE_TLS", "TLSLOG",
  "TLS_PORT_NO", "TLS_METHOD", "TLS_HANDSHAKE_TIMEOUT", "TLS_SEND_TIMEOUT",
  "SSLv23", "SSLv2", "SSLv3", "TLSv1", "TLS_VERIFY",
  "TLS_REQUIRE_CERTIFICATE", "TLS_CERTIFICATE", "TLS_PRIVATE_KEY",
  "TLS_CA_LIST", "ADVERTISED_ADDRESS", "ADVERTISED_PORT", "DISABLE_CORE",
  "OPEN_FD_LIMIT", "MCAST_LOOPBACK", "MCAST_TTL", "TOS", "KILL_TIMEOUT",
  "FLAGS_DECL", "AVPFLAGS_DECL", "ATTR_MARK", "SELECT_MARK", "ATTR_FROM",
  "ATTR_TO", "ATTR_FROMURI", "ATTR_TOURI", "ATTR_FROMUSER", "ATTR_TOUSER",
  "ATTR_FROMDOMAIN", "ATTR_TODOMAIN", "ATTR_GLOBAL", "ADDEQ",
  "STUN_REFRESH_INTERVAL", "STUN_ALLOW_STUN", "STUN_ALLOW_FP", "EQUAL",
  "EQUAL_T", "GT", "LT", "GTE", "LTE", "DIFF", "MATCH", "LOG_OR",
  "LOG_AND", "BIN_OR", "BIN_AND", "MINUS", "PLUS", "NOT", "NUMBER", "ID",
  "STRING", "IPV6ADDR", "COMMA", "SEMICOLON", "RPAREN", "LPAREN", "LBRACE",
  "RBRACE", "LBRACK", "RBRACK", "SLASH", "DOT", "CR", "COLON", "STAR",
  "$accept", "cfg", "statements", "statement", "$@1", "$@2", "$@3", "$@4",
  "$@5", "listen_id", "proto", "port", "phostport", "id_lst", "flags_decl",
  "flag_list", "flag_spec", "flag_name", "avpflags_decl", "avpflag_list",
  "avpflag_spec", "assign_stm", "module_stm", "ip", "ipv4", "ipv6addr",
  "ipv6", "route_name", "route_stm", "failure_route_stm",
  "onreply_route_stm", "branch_route_stm", "send_route_stm", "exp",
  "equalop", "intop", "binop", "strop", "uri_type", "exp_elem", "ipnet",
  "host_sep", "host", "fcmd", "exp_stm", "stm", "actions", "action",
  "if_cmd", "select_param", "select_params", "select_id", "$@6",
  "attr_class_spec", "attr_name_spec", "attr_spec", "attr_mark", "attr_id",
  "attr_id_num_idx", "attr_id_no_idx", "attr_id_ass", "attr_id_val",
  "attr_id_any", "attr_id_any_str", "assign_op", "assign_action",
  "avpflag_oper", "cmd", "$@7", "func_params", "func_param", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   193,   194,   195,   195,   195,   196,   196,   196,   196,
     197,   196,   198,   196,   199,   196,   200,   196,   201,   196,
     196,   196,   202,   202,   202,   203,   203,   203,   203,   204,
     204,   205,   205,   205,   205,   205,   206,   206,   207,   207,
     208,   208,   209,   209,   210,   210,   211,   211,   212,   212,
     213,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   215,   215,   215,   215,   215,   216,   216,
     217,   218,   219,   219,   220,   220,   220,   221,   221,   221,
     222,   222,   222,   223,   223,   223,   224,   224,   224,   225,
     225,   225,   226,   226,   226,   226,   226,   227,   227,   228,
     228,   228,   228,   228,   229,   229,   230,   230,   231,   231,
     231,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   233,   233,
     233,   233,   234,   234,   235,   235,   235,   236,   237,   237,
     237,   237,   238,   238,   239,   239,   239,   240,   240,   240,
     240,   240,   241,   241,   242,   242,   242,   243,   243,   245,
     244,   246,   246,   246,   246,   246,   246,   246,   246,   246,
     247,   248,   248,   249,   250,   251,   252,   253,   253,   254,
     254,   255,   255,   255,   256,   256,   257,   258,   258,   258,
     258,   258,   258,   259,   259,   259,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   261,   260,   262,   262,   262,
     262,   263,   263
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     1,     1,     1,     1,
       0,     2,     0,     2,     0,     2,     0,     2,     0,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     5,     3,     1,     2,     2,     2,
       1,     3,     1,     3,     1,     1,     2,     2,     1,     3,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     3,     2,     3,     2,     3,
       2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
       2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
       2,     3,     2,     3,     2,     3,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     8,     8,     2,     1,     1,
       7,     1,     1,     3,     1,     1,     1,     4,     7,     2,
       4,     7,     2,     4,     7,     2,     4,     7,     2,     4,
       7,     2,     3,     3,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     2,     3,     3,     3,
       3,     3,     3,     2,     3,     3,     3,     2,     3,     3,
       3,     2,     3,     3,     3,     2,     3,     3,     3,     2,
       3,     3,     3,     2,     3,     3,     3,     2,     3,     3,
       3,     2,     3,     3,     3,     2,     3,     3,     3,     3,
       2,     3,     3,     3,     2,     3,     3,     3,     3,     3,
       2,     3,     3,     3,     3,     3,     2,     3,     3,     3,
       3,     3,     2,     3,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     3,     3,     2,     1,     1,     1,     3,
       3,     3,     3,     3,     1,     3,     3,     3,     3,     3,
       1,     3,     1,     1,     1,     3,     3,     1,     1,     1,
       1,     3,     1,     3,     2,     1,     2,     2,     1,     2,
       1,     2,     3,     5,     1,     4,     4,     3,     1,     0,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     2,     5,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     5,     1,     1,     1,     4,     4,     4,     6,
       6,     6,     6,     6,     4,     2,     4,     4,     4,     4,
       6,     6,     6,     6,     6,     4,     2,     4,     4,     4,
       4,     6,     6,     6,     6,     6,     4,     2,     4,     4,
       4,     4,     6,     6,     6,     6,     6,     4,     2,     4,
       4,     4,     4,     6,     6,     6,     2,     4,     4,     4,
       4,     6,     6,     6,     2,     4,     3,     4,     2,     2,
       1,     1,     2,     2,     1,     4,     6,     2,     4,     4,
       4,     2,     4,     4,     2,     4,     4,     2,     6,     2,
       6,     2,     4,     4,     2,     4,     4,     4,     2,     4,
       4,     2,     4,     4,     2,     4,     4,     2,     4,     6,
       4,     3,     1,     4,     2,     4,     4,     2,     4,     4,
       2,     4,     4,     2,     4,     4,     2,     4,     3,     1,
       3,     1,     4,     3,     1,     4,     4,     4,     2,     4,
       4,     2,     4,     4,     2,     0,     5,     0,     3,     1,
       2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    20,    21,
       0,     0,     4,     0,     0,     0,     0,     0,     7,     8,
       6,     9,   202,     0,     0,     0,     0,     0,     0,     0,
       0,    64,     0,    66,     0,    68,     0,    70,     0,    72,
       0,    74,     0,    76,     0,    78,     0,    80,     0,    84,
       0,    82,     0,    86,     0,    88,     0,    90,     0,    92,
       0,    94,     0,    96,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   204,   203,   207,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    39,    45,    44,    38,    40,    42,    47,
      50,    46,    48,     0,     0,     0,     1,     5,     3,     0,
      11,     0,    13,     0,    15,     0,    17,     0,    19,    52,
      51,    54,    53,    56,    55,    58,    57,   177,    25,    26,
      27,     0,   354,    23,   211,     0,    28,    31,     0,    36,
     176,    22,   208,   212,   209,    24,   179,   178,    60,    59,
      62,    61,    63,    65,    67,    69,    71,    73,    75,    77,
      79,    83,    81,    85,    87,    89,    91,    93,    95,   101,
      97,    98,   103,   102,   105,   104,   107,   106,   109,   108,
     111,   110,   113,   112,   173,   172,   175,   174,     0,   100,
      99,   116,   115,   114,   119,   118,   117,   122,   121,   120,
     125,   124,   123,   127,   126,   129,   128,   131,   130,   133,
     132,   135,   134,   137,   136,   139,   138,   142,   140,   141,
     144,   143,   146,   145,   148,   147,   150,   149,   152,   151,
     157,   153,   154,   155,   156,   169,   168,   171,   170,   159,
     158,   161,   160,   163,   162,   165,   164,   167,   166,   181,
     180,   183,   182,   185,   184,   187,   186,   189,   188,   191,
     190,   193,   192,   195,   194,     0,     0,     0,   197,   196,
     199,   198,   201,   200,   219,     0,     0,   222,     0,     0,
     225,     0,     0,   228,     0,     0,   231,     0,     0,     0,
       0,     0,     0,    37,   353,     0,     0,     0,    41,    43,
      49,     0,     0,     0,     0,     0,     0,   480,   481,   484,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   522,
       0,     0,     0,     0,   539,   541,   544,     0,     0,     0,
       0,     0,     0,     0,   413,   414,   415,   393,   555,   370,
       0,     0,   365,   368,     0,   397,   398,     0,     0,     0,
     357,   214,   215,   216,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,    35,    29,    30,    32,    33,
     356,   355,     0,   425,     0,   447,     0,   458,     0,   436,
       0,   466,     0,   474,     0,   479,   478,     0,   483,   482,
     487,     0,   501,     0,   504,     0,     0,   508,     0,   524,
       0,   511,     0,   517,     0,   514,     0,     0,   530,     0,
     533,     0,   527,     0,   536,     0,     0,     0,     0,     0,
     248,   249,   250,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
     337,     0,     0,     0,     0,   236,   358,   336,   359,   344,
       0,   401,   403,   402,     0,   338,   360,   548,     0,   551,
       0,   554,     0,   491,     0,   494,     0,   497,     0,     0,
     371,   367,   366,   217,   364,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,     0,   391,   394,   406,     0,
     369,   499,     0,     0,   220,     0,   223,     0,   226,     0,
     229,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   521,     0,     0,     0,     0,     0,
       0,     0,     0,   538,   540,     0,     0,   543,   256,   237,
     238,   247,   246,     0,   310,   246,     0,   267,   240,   241,
     242,   243,   239,     0,   316,   246,     0,   271,     0,   328,
     246,     0,   279,     0,   322,   246,     0,   275,     0,   287,
       0,   295,     0,   283,     0,   291,     0,   335,     0,   300,
       0,   304,     0,     0,   234,     0,     0,     0,     0,     0,
     372,   362,   263,   246,     0,     0,   394,   244,   245,   246,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   557,     0,     0,   408,   407,
       0,   409,   411,   401,   403,   402,   410,   405,     0,   404,
       0,     0,     0,     0,     0,     0,     0,    34,     0,     0,
     426,     0,   424,     0,   417,     0,   418,     0,   416,   448,
       0,   446,     0,   439,     0,   440,     0,   438,   459,     0,
     457,     0,   450,     0,   451,     0,   449,   437,     0,   435,
       0,   428,     0,   429,     0,   427,   467,     0,   461,     0,
     462,     0,   460,   475,     0,   469,     0,   470,     0,   468,
     477,   488,     0,   485,   502,     0,   505,   503,   506,   509,
     507,   525,   523,   512,   510,   518,   516,   515,   513,     0,
     520,   531,   529,   534,   532,   528,   526,   537,   535,   545,
     542,   255,   254,   251,   253,     0,   399,   400,   252,   308,
     350,   305,   309,   306,   307,   266,   264,   265,   314,   311,
     315,   312,   313,   270,   268,   269,   326,   323,   327,   324,
     325,   278,   276,   277,   320,   317,   321,   318,   319,   274,
     272,   273,   286,   284,   285,   294,   292,   293,   282,   280,
     281,   290,   288,   289,   334,   330,   331,   333,   332,   329,
     299,   298,   296,   297,   303,   301,   302,   374,   378,   380,
     235,   361,   233,   232,     0,     0,   261,   262,   257,   258,
     260,   259,   345,   347,   346,     0,   341,   342,   339,   340,
     343,   547,   546,   550,   549,   553,   552,   489,   490,   492,
     493,   495,   496,   561,   562,     0,   559,   392,   396,     0,
     394,     0,     0,     0,     0,     0,     0,     0,   206,   205,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   394,
       0,     0,     0,   363,   373,     0,   560,     0,   556,   412,
       0,   218,   221,   224,   227,   230,   210,   422,   423,   420,
     421,   419,   444,   445,   442,   443,   441,   455,   456,   453,
     454,   452,   433,   434,   431,   432,   430,   464,   465,   463,
     472,   473,   471,   486,   500,   519,     0,   351,   349,   348,
       0,     0,   377,   395,   558,   498,   375,   376
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    80,    81,    82,    83,    84,    85,    86,    87,   227,
     228,   458,   229,   230,    88,   186,   187,   188,    89,   191,
     192,    90,    91,   231,   232,   233,   234,   444,   200,   202,
     204,   206,   208,   533,   672,   673,   721,   663,   534,   535,
     851,   386,   235,   430,   537,   710,   431,   432,   433,   908,
     909,   539,   703,   575,   576,   577,   434,   435,   847,   436,
     437,   544,   545,   750,   579,   438,   439,   440,   559,   945,
     946
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -603
static const yytype_int16 yypact[] =
{
     618,   -87,   -42,   -41,   -25,   228,   239,   269,   270,   282,
      52,   365,   367,   515,   524,   663,   976,   977,   978,   979,
    1006,  1026,  1027,  1028,  1049,  1074,  1075,   298,   302,   314,
     315,   353,   363,   368,   369,   377,   501,    40,     7,   516,
     605,   610,   611,   614,   636,   679,   700,   713,   714,   737,
     738,   811,   812,   815,   825,   862,   877,   878,   880,   887,
     888,   891,   895,   900,   909,   916,   919,   926,   961,   965,
     966,   967,   994,   182,   193,   995,   996,  1000,  -603,  -603,
      62,   471,  -603,   117,    91,   152,   435,   500,  -603,  -603,
    -603,  -603,  -603,    34,   477,   478,    44,    28,    47,   480,
     482,  -603,   465,  -603,   483,  -603,   484,  -603,   487,  -603,
     492,  -603,   624,  -603,   777,  -603,   809,  -603,   881,  -603,
     890,  -603,   908,  -603,   992,  -603,  1014,  -603,  1035,  -603,
    1057,  -603,  1076,  -603,  1080,   489,   985,   490,   491,   506,
     507,   508,   510,   597,   598,  -603,  -603,  -603,  1079,   645,
     196,   210,   296,   318,   646,   668,   669,   670,   744,   745,
     746,   324,   747,   763,   767,   768,   769,  1159,   778,   779,
     780,   781,   185,   444,   445,   215,   782,   783,   784,   785,
     786,   787,   788,  -603,  -603,  -603,  -603,  1078,  1077,  -603,
    -603,  -603,  1086,   789,   790,   791,  -603,   -87,  -603,     5,
    -603,    56,  -603,    72,  -603,    86,  -603,    92,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  1081,  -603,  -603,  -603,   986,  -603,  1114,  1118,  1383,
    -603,  -603,  -603,  -603,  -603,  -117,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  1087,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,   -97,  1095,   -97,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  2001,   -30,  -603,  2001,   -30,
    -603,  2001,   -30,  -603,  2001,   -30,  -603,  2001,   -30,  1134,
    1124,    43,   291,  -603,  -603,   128,  1154,  1162,  -603,  -603,
    -603,    13,    32,    54,    60,    84,    90,    -6,   -19,  -603,
      97,    98,   104,  1160,   108,   110,   125,   131,   134,  1167,
     137,   143,   148,   149,  1168,  1169,  1171,  1690,   151,   154,
     158,   162,   164,   166,  -603,  -603,  -603,  -603,  -603,  -603,
      22,   806,  -603,  -603,  1673,  -603,  -603,  1011,  1164,   168,
    -603,  -603,  -603,  -603,  1173,   859,  1174,   905,  1175,   989,
    1176,  1088,  1178,  1177,  -603,  -603,  -603,  -603,  -603,  1165,
    -603,  -603,  1179,  -603,    69,  -603,    85,  -603,   123,  -603,
     200,  -603,   229,  -603,   235,  -603,  -603,  -105,  -603,  -603,
    -603,   334,  -603,   446,  -603,   288,  1180,  -603,   447,  -603,
     448,  -603,   449,  -603,   792,  -603,   793,  -128,  -603,   450,
    -603,   459,  -603,   460,  -603,   461,  1197,  1198,   179,   355,
    -603,  -603,  -603,   875,   639,   892,   981,   896,   987,   906,
    1157,  1163,  1170,  1205,  1211,   822,  1219,  1386,  -603,  1690,
    -603,  1690,  2001,  1767,   974,  -603,  -603,  -603,  -603,   242,
    1673,  1759,  1896,  1227,  1907,  -603,  -603,  -603,   240,  -603,
     794,  -603,    74,  -603,    -5,  -603,   204,  -603,   262,  1210,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  1208,  -603,  1214,  -603,  1949,
    -603,  -603,   -99,  1217,  -603,  1232,  -603,  1234,  -603,  1235,
    -603,  1251,  1263,  -116,   -22,  1258,   203,   275,   333,   -72,
    1287,   357,   475,   585,   247,  1288,   616,   673,   674,   889,
    1289,   677,   824,   865,   899,  1290,   873,   885,  1166,  1306,
     903,   923,  1385,  1310,  -603,  1311,  1273,  1312,  1320,  1335,
    1334,  1336,  1352,  1356,  1357,  1358,  1373,  1374,  1381,  1384,
    1388,  1389,  1390,   924,  -603,  1391,  1394,  1395,  1397,  1398,
    1399,  1402,  1417,  -603,  -603,  1419,  1420,  -603,  -603,  -603,
    -603,  -603,  -603,   256,  -603,   249,   342,  -603,  -603,  -603,
    -603,  -603,  -603,   497,  -603,   795,   423,  -603,   498,  -603,
     983,   440,  -603,   499,  -603,  1378,   443,  -603,   502,  -603,
      16,  -603,   503,  -603,    36,  -603,   504,  -603,  1341,  -603,
     496,  -603,   505,  1340,  -603,  -118,  1172,  1690,  1690,  2001,
    1534,  -603,  -603,  1520,   276,   -81,  1432,  -603,  -603,  1446,
    1448,  1456,   -78,  1452,  1455,  1458,  1460,  1462,  1471,  1473,
    1476,  1477,  1479,  1481,  1482,   -18,  1459,  1478,  -603,  -603,
    1690,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  1673,  -603,
    1487,  2001,  2001,  2001,  2001,  2001,  1480,  -603,  1486,  1488,
    -603,   -12,  -603,  1496,  -603,  1497,  -603,  1502,  -603,  -603,
      19,  -603,  1503,  -603,  1504,  -603,  1507,  -603,  -603,    24,
    -603,  1508,  -603,  1510,  -603,  1511,  -603,  -603,   345,  -603,
    1512,  -603,  1513,  -603,  1514,  -603,  -603,  1515,  -603,  1516,
    -603,  1529,  -603,  -603,  1530,  -603,  1531,  -603,  1532,  -603,
    -603,  -603,  1546,  -603,  -603,  1550,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  1551,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  1673,  -603,  -603,  -603,  -603,
    1568,  -603,  -603,  -603,  -117,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -117,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -117,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -117,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  1571,  -603,  1569,
    -603,  -603,  1589,  -603,  1218,  1873,  -603,  -603,  -603,  -117,
    -603,  -603,  -603,  -603,  -603,   -98,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,   187,  -603,  -603,  -603,   -48,
    -603,   -97,  1271,  1401,  1454,  1500,  1584,  1586,  -603,  -603,
    1578,  1581,  1582,  1585,  1600,  1601,  1602,  1603,  1619,  1623,
    1624,  1625,  1633,  1635,  1637,  1648,  1649,  1650,  1651,  1652,
    1656,  1657,  1659,  1660,  1661,  1662,  1664,  1666,  1674,  1580,
      89,   280,  1340,  -603,  -603,  1672,  -603,   -18,  -603,  -603,
    1678,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  1592,  -603,  1081,  -603,
    1675,  1676,  -603,  -603,  -603,  -603,  -603,  -603
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -603,  -603,  -603,  1780,  -603,  -603,  -603,  -603,  -603,  -173,
    -602,  1275,  1317,   -55,  -603,  1509,  -603,   -74,  -603,  1533,
    -603,  -603,  -603,  -463,  -603,  1645,  -603,   969,  -603,  -603,
    -603,  -603,  -603,  -509,  1132,  1331,  -603,  1301,  1190,  -603,
    -243,  -603,  -432,  -413,  -603,   956,  -356,  -421,  -401,   897,
    -603,  -530,  -603,  -603,  1136,  -521,  -404,  -350,  -392,  -386,
    -603,   960,  1328,  -603,  -603,  -378,  -603,  -603,  -603,  -603,
     894
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -401
static const yytype_int16 yytable[] =
{
     190,   598,   340,   603,   536,   608,   364,   613,   147,   617,
     564,   621,   445,   540,   463,   447,   538,   882,   449,   716,
     704,   451,   705,   560,   564,   542,   564,   960,   564,   217,
     564,   543,   599,   465,   604,   209,   609,   888,   614,   546,
     618,   145,   622,   237,   455,   215,   478,   427,   236,   742,
     643,   707,   708,   101,   644,   467,   384,   367,   965,   475,
     456,   469,   196,   970,   910,   427,   528,   541,   427,   528,
     595,   623,   385,   370,    92,   727,   457,   624,   995,   747,
     184,   185,   218,   219,   220,   471,   600,   373,   883,   948,
    1037,   473,   889,   376,   218,   219,   220,   922,   480,   482,
     928,   384,   218,   219,   220,   484,   201,   596,   767,   487,
     768,   489,   711,   218,   219,   220,   536,   385,   536,    93,
      94,   707,   708,   601,   605,   540,   491,   540,   538,   460,
     538,   199,   493,   844,   999,   495,    95,   542,   498,   542,
     218,   219,   220,   543,   500,   543,   441,   442,   443,   502,
     504,   546,   547,   546,   758,   549,   759,   479,   943,   551,
     944,   606,   427,   553,   961,   555,   741,   557,   203,   581,
     476,   729,   184,   185,   383,   540,   706,   477,   748,   541,
     655,   541,   427,   183,   920,   923,   333,   744,   996,   365,
     148,   366,   929,   745,   189,   966,   464,   281,   912,   913,
     971,   610,   850,   561,   221,   222,   223,   224,   226,   459,
     210,   284,   850,   102,   225,   466,   339,   850,   146,   456,
     226,   216,   850,   221,   222,   223,   224,   950,   226,   743,
     615,   949,   749,   225,   854,   457,   619,   468,   611,   226,
     368,   723,   369,   470,   862,   221,   222,   597,   224,   870,
     221,   222,   223,   224,   878,   225,   371,   841,   372,   845,
     225,   221,   222,   602,   224,  1038,   226,   472,   224,   845,
     374,   225,   375,   474,   845,   225,   377,   917,   378,   845,
     481,   483,   919,   190,   845,   564,   845,   485,   845,   630,
     845,   488,   845,   490,   536,   536,   845,   287,   845,   221,
     222,   607,   224,   540,   540,  -352,   538,   538,   492,   225,
     845,   845,   849,   846,   494,   542,   542,   496,   845,   290,
     499,   543,   543,   846,   989,   307,   501,   536,   846,   546,
     546,   503,   505,   846,   548,   625,   540,   550,   846,   538,
     846,   552,   846,   852,   846,   554,   846,   556,   542,   558,
     846,   582,   846,   914,   543,   656,   658,   541,   541,   184,
     185,   657,   546,   334,   846,   846,   103,   997,   105,   998,
     184,   185,   846,   282,   283,   724,   221,   222,   612,   224,
     731,   184,   185,   761,   975,   762,   225,   285,   286,    96,
     541,   221,   222,   223,   224,   952,   953,   954,   955,   956,
      97,   225,   427,   528,   659,   221,   222,   616,   224,   660,
     661,   221,   222,   620,   224,   225,   221,   222,   223,   224,
     384,   225,   427,   528,   860,   221,   225,   776,   224,   777,
      98,    99,   859,   842,   843,   225,   385,   867,   733,   184,
     185,   868,   875,   100,   876,   335,   337,   628,   633,   635,
     637,   645,   205,   222,   918,   763,  1040,   764,  1041,   135,
     647,   649,   651,   136,   441,   442,   443,   221,   222,   223,
     224,    -2,   197,   288,   289,   137,   138,   225,   211,   213,
     730,   238,   732,   240,   734,   -10,   -12,   -14,   -16,   -18,
     259,   262,   264,   564,   711,   291,   292,   900,   855,   863,
     871,   308,   309,   879,   885,   891,   904,   266,   268,   270,
     626,   272,   627,   765,   139,   766,   107,   659,   207,   222,
     853,   976,   660,   661,   140,   109,   104,  1039,   106,   141,
     142,   564,   564,   564,   564,   564,   901,   770,   143,   771,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,   274,   276,
     222,   861,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,   222,   869,     1,
     222,   877,   336,   338,   629,   634,   636,   638,   646,    75,
      76,    77,   -10,   -12,   -14,   -16,   -18,   648,   650,   652,
     667,   242,   427,   427,   427,   427,   279,   293,   427,   427,
     427,   427,    78,   212,   214,   772,   239,   773,   241,   243,
     244,    79,   144,   245,   111,   260,   263,   265,   246,   295,
     297,   299,   902,   856,   864,   872,   108,   149,   880,   886,
     892,   905,   267,   269,   271,   110,   273,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,   301,   303,   305,   310,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,   312,   774,   150,   775,   314,   316,
     318,   151,   152,   275,   277,   153,    75,    76,    77,   325,
     327,   329,   331,   341,   343,   345,   347,   349,   351,   353,
     358,   360,   362,   639,   641,   725,   779,   154,   780,    78,
     247,   659,   668,   669,   670,   671,   660,   562,    79,   391,
     392,   393,   394,   395,   396,   397,   398,   399,   400,   401,
     402,   280,   294,   697,   112,   403,   404,   405,   406,   407,
     408,   409,   410,   411,   412,   413,   414,   415,   416,   417,
     155,   418,   419,   420,   296,   298,   300,   421,   422,   423,
     424,   425,   426,   781,   783,   782,   784,   788,   858,   789,
     562,   156,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   157,   158,   664,  1000,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   674,   418,   419,   420,   679,   159,   160,
     421,   422,   423,   424,   425,   426,   562,   684,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     302,   304,   306,   311,   403,   404,   405,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   417,   313,
     418,   419,   420,   315,   317,   319,   421,   422,   423,   424,
     425,   426,   427,   248,   326,   328,   330,   332,   342,   344,
     346,   348,   350,   352,   354,   359,   361,   363,   640,   642,
     726,   221,   161,   162,   224,   712,   163,   113,   115,   117,
     119,   225,   677,   428,   659,   249,   164,   429,   682,   660,
     562,   563,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   790,   427,   791,   121,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   165,   418,   419,   420,   123,   125,   127,
     421,   422,   423,   424,   425,   426,   428,   659,   166,   167,
     429,   168,   660,   661,   584,   792,   866,   793,   169,   170,
     129,   427,   171,   797,   659,   798,   172,   250,   659,   660,
     661,   173,   384,   660,   661,   799,   251,   800,   659,   785,
     174,   786,   384,   660,   661,   131,   133,   175,   385,   794,
     176,   795,   428,   804,   252,   805,   429,   177,   385,   562,
     586,   391,   392,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   806,   829,   807,   830,   403,   404,   405,
     406,   407,   408,   409,   410,   411,   412,   413,   414,   415,
     416,   417,   178,   418,   419,   420,   179,   180,   181,   421,
     422,   423,   424,   425,   426,   427,   659,   114,   116,   118,
     120,   660,   661,   659,   668,   669,   670,   671,   660,   659,
     668,   669,   670,   671,   660,   182,   193,   194,   687,   221,
     320,   195,   224,   261,   689,   224,   428,   122,   253,   225,
     429,   691,   578,   562,   588,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   124,   126,   128,
     254,   403,   404,   405,   406,   407,   408,   409,   410,   411,
     412,   413,   414,   415,   416,   417,   693,   418,   419,   420,
     130,   255,   695,   421,   422,   423,   424,   425,   426,   562,
     699,   391,   392,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   256,   427,   132,   134,   403,   404,   405,
     406,   407,   408,   409,   410,   411,   412,   413,   414,   415,
     416,   417,   257,   418,   419,   420,   258,   278,   355,   421,
     422,   423,   424,   425,   426,   428,   357,   387,   356,   429,
     379,   389,   562,   590,   391,   392,   393,   394,   395,   396,
     397,   398,   399,   400,   401,   402,   321,   322,   323,   324,
     403,   404,   405,   406,   407,   408,   409,   410,   411,   412,
     413,   414,   415,   416,   417,   381,   418,   419,   420,   382,
     453,   454,   421,   422,   423,   424,   425,   426,   427,   659,
     668,   669,   670,   671,   660,   659,   668,   669,   670,   671,
     660,   461,   659,   668,   669,   670,   671,   660,   446,   384,
     462,   448,   894,   486,   450,   580,   801,   452,   802,   428,
     497,   506,   507,   429,   508,   385,   593,   911,   632,   594,
     583,   585,   587,   589,   427,   591,   592,   659,   668,   669,
     670,   671,   660,   659,   668,   669,   670,   671,   660,   653,
     654,   659,   668,   669,   670,   671,   660,   701,  -398,   510,
     511,   512,   895,   735,   896,   428,   897,   736,   898,   429,
     737,   751,   562,   993,   391,   392,   393,   394,   395,   396,
     397,   398,   399,   400,   401,   402,   752,   427,   753,   754,
     403,   404,   405,   406,   407,   408,   409,   410,   411,   412,
     413,   414,   415,   416,   417,   755,   418,   419,   420,   756,
     760,   874,   421,   422,   423,   424,   425,   426,   428,   218,
     219,   220,   429,   812,   631,   562,  1001,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   769,
     778,   787,   796,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   803,   418,
     419,   420,   810,   811,   813,   421,   422,   423,   424,   425,
     426,   562,   814,   391,   392,   393,   394,   395,   396,   397,
     398,   399,   400,   401,   402,   815,   816,   907,   817,   403,
     404,   405,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,   818,   418,   419,   420,   819,   820,
     821,   421,   422,   423,   424,   425,   426,   427,   659,   668,
     669,   670,   671,   660,   221,   822,   823,   224,   384,   221,
     222,   223,   224,   824,   225,   808,   825,   809,   915,   225,
     826,   827,   828,   831,   385,   226,   832,   833,   428,   834,
     835,   836,   429,   916,   837,   562,  1002,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   838,
     427,   839,   840,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   925,   418,
     419,   420,  -239,   848,   926,   421,   422,   423,   424,   425,
     426,   428,   927,   857,   931,   429,   574,   932,   865,  1003,
     933,   662,   934,   873,   935,   665,   427,   675,   881,   680,
     884,   685,   887,   936,   890,   937,   893,   698,   938,   939,
     903,   940,   906,   941,   942,   948,   713,   951,   958,   957,
     959,   662,   962,   963,   921,   924,   719,   428,   964,   967,
     968,   429,   930,   969,   972,  1004,   973,   974,   977,   978,
     979,   980,   981,   391,   392,   393,   394,   395,   396,   397,
     398,   399,   400,   401,   402,   982,   983,   984,   985,   403,
     404,   405,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,   986,   418,   419,   420,   987,   988,
     427,   421,   422,   423,   424,   425,   426,   509,   510,   511,
     512,   513,   514,   515,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   990,   991,   992,   708,
    1007,   428,  1006,  1008,  1009,   429,  1036,  1010,   995,  1005,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,  1011,  1012,  1013,  1014,   403,   404,   405,   406,
     407,   408,   409,   410,   411,   412,   413,   414,   415,   416,
     417,  1015,   418,   419,   420,  1016,  1017,  1018,   421,   422,
     423,   424,   425,   426,   666,  1019,   676,  1020,   681,  1021,
     686,   565,   566,   567,   568,   569,   570,   571,   572,   573,
    1022,  1023,  1024,  1025,  1026,   714,   427,   528,  1027,  1028,
     715,  1029,  1030,  1031,  1032,   722,  1033,   678,  1034,   683,
     574,   688,   690,   692,   694,   696,  1035,   700,   702,  1043,
    1045,   198,  1046,  1047,   388,   529,   530,   428,   757,   728,
     380,   994,   947,   531,   532,   720,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   899,  1042,
     390,  1044,   403,   404,   405,   406,   407,   408,   409,   410,
     411,   412,   413,   414,   415,   416,   417,   746,   418,   419,
     420,     0,     0,   427,   421,   422,   423,   424,   425,   426,
    -397,  -399,  -399,  -399,  -399,  -399,  -399,  -399,     0,     0,
    -399,  -399,     0,     0,     0,     0,   707,   708,     0,     0,
       0,     0,     0,     0,   428,     0,     0,     0,   429,     0,
       0,   709,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,     0,     0,     0,     0,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,     0,     0,   418,   419,   420,     0,     0,     0,
     421,   422,   423,   424,   425,   426,     0,     0,     0,     0,
       0,     0,     0,     0,   391,   392,   393,   394,   395,   396,
     397,   398,   399,   400,   401,   402,     0,     0,     0,   427,
     403,   404,   405,   406,   407,   408,   409,   410,   411,   412,
     413,   414,   415,   416,   417,     0,   418,   419,   420,     0,
       0,     0,   421,   422,   423,   424,   425,   426,     0,     0,
     428,     0,     0,     0,   429,     0,     0,   709,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,     0,     0,  -400,  -400,   659,
     668,   669,   670,   671,   660,   661,     0,     0,   717,   718,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   427,   528,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   738,   428,   739,     0,     0,
       0,     0,   740,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   427,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   428,     0,
       0,     0,   429
};

static const yytype_int16 yycheck[] =
{
      74,   464,   175,   466,   417,   468,     1,   470,     1,   472,
     431,   474,   368,   417,     1,   371,   417,     1,   374,   540,
     529,   377,   531,     1,   445,   417,   447,    39,   449,     1,
     451,   417,   464,     1,   466,     1,   468,     1,   470,   417,
     472,     1,   474,    98,     1,     1,    65,   146,     1,   579,
     178,   169,   170,     1,   182,     1,   173,     1,    39,    65,
     176,     1,     0,    39,   182,   146,   147,   417,   146,   147,
       1,   176,   189,     1,   161,     1,   192,   182,   176,   178,
     177,   178,    66,    67,    68,     1,     1,     1,   690,   187,
       1,     1,   694,     1,    66,    67,    68,   178,     1,     1,
     178,   173,    66,    67,    68,     1,    15,    38,   180,     1,
     182,     1,   533,    66,    67,    68,   529,   189,   531,   161,
     161,   169,   170,    38,     1,   529,     1,   531,   529,     1,
     531,    14,     1,   663,   182,     1,   161,   529,     1,   531,
      66,    67,    68,   529,     1,   531,   176,   177,   178,     1,
       1,   529,     1,   531,   176,     1,   178,   176,   176,     1,
     178,    38,   146,     1,   176,     1,   579,     1,    16,     1,
     176,   176,   177,   178,   229,   579,   532,   183,   582,   529,
       1,   531,   146,     1,   714,   715,     1,   579,     1,   184,
     183,   186,   722,   579,     1,   176,   183,     1,   707,   708,
     176,     1,   665,   181,   176,   177,   178,   179,   192,   382,
     176,     1,   675,   161,   186,   183,     1,   680,   178,   176,
     192,   177,   685,   176,   177,   178,   179,   748,   192,   579,
       1,   740,   582,   186,   666,   192,     1,   183,    38,   192,
     184,     1,   186,   183,   676,   176,   177,   178,   179,   681,
     176,   177,   178,   179,   686,   186,   184,     1,   186,   663,
     186,   176,   177,   178,   179,   176,   192,   183,   179,   673,
     184,   186,   186,   183,   678,   186,   184,     1,   186,   683,
     183,   183,   714,   357,   688,   706,   690,   183,   692,     1,
     694,   183,   696,   183,   707,   708,   700,     1,   702,   176,
     177,   178,   179,   707,   708,   177,   707,   708,   183,   186,
     714,   715,    63,   663,   183,   707,   708,   183,   722,     1,
     183,   707,   708,   673,   845,     1,   183,   740,   678,   707,
     708,   183,   183,   683,   183,     1,   740,   183,   688,   740,
     690,   183,   692,     1,   694,   183,   696,   183,   740,   183,
     700,   183,   702,   709,   740,   176,     1,   707,   708,   177,
     178,   182,   740,   178,   714,   715,     1,   180,     1,   182,
     177,   178,   722,   177,   178,   548,   176,   177,   178,   179,
     176,   177,   178,   180,    39,   182,   186,   177,   178,   161,
     740,   176,   177,   178,   179,   751,   752,   753,   754,   755,
     161,   186,   146,   147,   162,   176,   177,   178,   179,   167,
     168,   176,   177,   178,   179,   186,   176,   177,   178,   179,
     173,   186,   146,   147,     1,   176,   186,   180,   179,   182,
     161,   161,   675,   177,   178,   186,   189,   680,   176,   177,
     178,     1,   685,   161,     1,     1,     1,     1,     1,     1,
       1,     1,    17,   177,   178,   180,   176,   182,   178,   161,
       1,     1,     1,   161,   176,   177,   178,   176,   177,   178,
     179,     0,     1,   177,   178,   161,   161,   186,     1,     1,
     554,     1,   556,     1,   558,    14,    15,    16,    17,    18,
       1,     1,     1,   914,   915,   177,   178,     1,     1,     1,
       1,   177,   178,     1,     1,     1,     1,     1,     1,     1,
     176,     1,   178,   180,   161,   182,     1,   162,    18,   177,
     178,   176,   167,   168,   161,     1,   161,   990,   161,   161,
     161,   952,   953,   954,   955,   956,    40,   180,   161,   182,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,     1,     1,
     177,   178,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   177,   178,     1,
     177,   178,   178,   178,   178,   178,   178,   178,   178,   158,
     159,   160,    14,    15,    16,    17,    18,   178,   178,   178,
       1,   176,   146,   146,   146,   146,     1,     1,   146,   146,
     146,   146,   181,   176,   176,   180,   176,   182,   176,   176,
     176,   190,   161,   176,     1,   176,   176,   176,   176,     1,
       1,     1,   176,   176,   176,   176,   161,   161,   176,   176,
     176,   176,   176,   176,   176,   161,   176,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,     1,     1,     1,     1,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,     1,   180,   161,   182,     1,     1,
       1,   161,   161,   176,   176,   161,   158,   159,   160,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,   180,   161,   182,   181,
     176,   162,   163,   164,   165,   166,   167,     1,   190,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,   176,   176,     1,   161,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
     161,    35,    36,    37,   176,   176,   176,    41,    42,    43,
      44,    45,    46,   180,   180,   182,   182,   180,    63,   182,
       1,   161,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,   161,   161,     1,   951,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,     1,    35,    36,    37,     1,   161,   161,
      41,    42,    43,    44,    45,    46,     1,     1,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
     176,   176,   176,   176,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,   176,
      35,    36,    37,   176,   176,   176,    41,    42,    43,    44,
      45,    46,   146,   176,   176,   176,   176,   176,   176,   176,
     176,   176,   176,   176,   176,   176,   176,   176,   176,   176,
     176,   176,   161,   161,   179,     1,   161,     1,     1,     1,
       1,   186,     1,   177,   162,   176,   161,   181,     1,   167,
       1,   185,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,   180,   146,   182,     1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,   161,    35,    36,    37,     1,     1,     1,
      41,    42,    43,    44,    45,    46,   177,   162,   161,   161,
     181,   161,   167,   168,   185,   180,    63,   182,   161,   161,
       1,   146,   161,   180,   162,   182,   161,   176,   162,   167,
     168,   161,   173,   167,   168,   180,   176,   182,   162,   180,
     161,   182,   173,   167,   168,     1,     1,   161,   189,   180,
     161,   182,   177,   180,   176,   182,   181,   161,   189,     1,
     185,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,   180,   180,   182,   182,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,   161,    35,    36,    37,   161,   161,   161,    41,
      42,    43,    44,    45,    46,   146,   162,   161,   161,   161,
     161,   167,   168,   162,   163,   164,   165,   166,   167,   162,
     163,   164,   165,   166,   167,   161,   161,   161,     1,   176,
       1,   161,   179,   178,     1,   179,   177,   161,   176,   186,
     181,     1,   161,     1,   185,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,   161,   161,   161,
     176,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,     1,    35,    36,    37,
     161,   176,     1,    41,    42,    43,    44,    45,    46,     1,
       1,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,   176,   146,   161,   161,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,   176,    35,    36,    37,   176,   178,   180,    41,
      42,    43,    44,    45,    46,   177,   180,   180,   191,   181,
     189,   176,     1,   185,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,   127,   128,   129,   130,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,   191,    35,    36,    37,   191,
     176,   187,    41,    42,    43,    44,    45,    46,   146,   162,
     163,   164,   165,   166,   167,   162,   163,   164,   165,   166,
     167,   177,   162,   163,   164,   165,   166,   167,   369,   173,
     178,   372,     1,   183,   375,   181,   180,   378,   182,   177,
     183,   183,   183,   181,   183,   189,   191,   185,   178,   180,
     187,   187,   187,   187,   146,   187,   189,   162,   163,   164,
     165,   166,   167,   162,   163,   164,   165,   166,   167,   182,
     182,   162,   163,   164,   165,   166,   167,     1,   161,    48,
      49,    50,    51,   183,    53,   177,    55,   189,    57,   181,
     186,   184,     1,   185,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,   184,   146,   184,   184,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,   184,    35,    36,    37,   176,
     182,    63,    41,    42,    43,    44,    45,    46,   177,    66,
      67,    68,   181,   180,   485,     1,   185,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,   182,
     182,   182,   182,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,   182,    35,
      36,    37,   182,   182,   182,    41,    42,    43,    44,    45,
      46,     1,   182,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,   180,   182,   177,   182,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,   182,    35,    36,    37,   182,   182,
     182,    41,    42,    43,    44,    45,    46,   146,   162,   163,
     164,   165,   166,   167,   176,   182,   182,   179,   173,   176,
     177,   178,   179,   182,   186,   180,   182,   182,    34,   186,
     182,   182,   182,   182,   189,   192,   182,   182,   177,   182,
     182,   182,   181,    63,   182,     1,   185,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,   182,
     146,   182,   182,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,   186,    35,
      36,    37,   176,   663,   176,    41,    42,    43,    44,    45,
      46,   177,   176,   673,   182,   181,   177,   182,   678,   185,
     182,   509,   182,   683,   182,   513,   146,   515,   688,   517,
     690,   519,   692,   182,   694,   182,   696,   525,   182,   182,
     700,   182,   702,   182,   182,   187,   534,   180,   182,   189,
     182,   539,   176,   176,   714,   715,   544,   177,   176,   176,
     176,   181,   722,   176,   176,   185,   176,   176,   176,   176,
     176,   176,   176,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,   176,   176,   176,   176,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,   178,    35,    36,    37,   178,   178,
     146,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,   188,   186,   189,   170,
     182,   177,   176,   182,   182,   181,   186,   182,   176,   185,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,   182,   182,   182,   182,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,   182,    35,    36,    37,   182,   182,   182,    41,    42,
      43,    44,    45,    46,   513,   182,   515,   182,   517,   182,
     519,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     182,   182,   182,   182,   182,   534,   146,   147,   182,   182,
     539,   182,   182,   182,   182,   544,   182,   516,   182,   518,
     177,   520,   521,   522,   523,   524,   182,   526,   527,   187,
     182,    81,   187,   187,   355,   175,   176,   177,   593,   552,
     225,   915,   736,   183,   184,   544,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,   698,   992,
     357,   997,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,   579,    35,    36,
      37,    -1,    -1,   146,    41,    42,    43,    44,    45,    46,
     161,   162,   163,   164,   165,   166,   167,   168,    -1,    -1,
     171,   172,    -1,    -1,    -1,    -1,   169,   170,    -1,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,   181,    -1,
      -1,   184,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    -1,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    -1,    -1,    -1,   146,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    44,    45,    46,    -1,    -1,
     177,    -1,    -1,    -1,   181,    -1,    -1,   184,   162,   163,
     164,   165,   166,   167,   168,    -1,    -1,   171,   172,   162,
     163,   164,   165,   166,   167,   168,    -1,    -1,   171,   172,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   146,   147,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   176,   177,   178,    -1,    -1,
      -1,    -1,   183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,   181
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     1,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   158,   159,   160,   181,   190,
     194,   195,   196,   197,   198,   199,   200,   201,   207,   211,
     214,   215,   161,   161,   161,   161,   161,   161,   161,   161,
     161,     1,   161,     1,   161,     1,   161,     1,   161,     1,
     161,     1,   161,     1,   161,     1,   161,     1,   161,     1,
     161,     1,   161,     1,   161,     1,   161,     1,   161,     1,
     161,     1,   161,     1,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,     1,   178,     1,   183,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   161,     1,   177,   178,   208,   209,   210,     1,
     210,   212,   213,   161,   161,   161,     0,     1,   196,    14,
     221,    15,   222,    16,   223,    17,   224,    18,   225,     1,
     176,     1,   176,     1,   176,     1,   177,     1,    66,    67,
      68,   176,   177,   178,   179,   186,   192,   202,   203,   205,
     206,   216,   217,   218,   219,   235,     1,   206,     1,   176,
       1,   176,   176,   176,   176,   176,   176,   176,   176,   176,
     176,   176,   176,   176,   176,   176,   176,   176,   176,     1,
     176,   178,     1,   176,     1,   176,     1,   176,     1,   176,
       1,   176,     1,   176,     1,   176,     1,   176,   178,     1,
     176,     1,   177,   178,     1,   177,   178,     1,   177,   178,
       1,   177,   178,     1,   176,     1,   176,     1,   176,     1,
     176,     1,   176,     1,   176,     1,   176,     1,   177,   178,
       1,   176,     1,   176,     1,   176,     1,   176,     1,   176,
       1,   127,   128,   129,   130,     1,   176,     1,   176,     1,
     176,     1,   176,     1,   178,     1,   178,     1,   178,     1,
     202,     1,   176,     1,   176,     1,   176,     1,   176,     1,
     176,     1,   176,     1,   176,   180,   191,   180,     1,   176,
       1,   176,     1,   176,     1,   184,   186,     1,   184,   186,
       1,   184,   186,     1,   184,   186,     1,   184,   186,   189,
     218,   191,   191,   206,   173,   189,   234,   180,   208,   176,
     212,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    35,    36,
      37,    41,    42,    43,    44,    45,    46,   146,   177,   181,
     236,   239,   240,   241,   249,   250,   252,   253,   258,   259,
     260,   176,   177,   178,   220,   239,   220,   239,   220,   239,
     220,   239,   220,   176,   187,     1,   176,   192,   204,   202,
       1,   177,   178,     1,   183,     1,   183,     1,   183,     1,
     183,     1,   183,     1,   183,    65,   176,   183,    65,   176,
       1,   183,     1,   183,     1,   183,   183,     1,   183,     1,
     183,     1,   183,     1,   183,     1,   183,   183,     1,   183,
       1,   183,     1,   183,     1,   183,   183,   183,   183,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,   147,   175,
     176,   183,   184,   226,   231,   232,   236,   237,   241,   244,
     249,   250,   251,   252,   254,   255,   258,     1,   183,     1,
     183,     1,   183,     1,   183,     1,   183,     1,   183,   261,
       1,   181,     1,   185,   240,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   177,   246,   247,   248,   161,   257,
     181,     1,   183,   187,   185,   187,   185,   187,   185,   187,
     185,   187,   189,   191,   180,     1,    38,   178,   216,   235,
       1,    38,   178,   216,   235,     1,    38,   178,   216,   235,
       1,    38,   178,   216,   235,     1,   178,   216,   235,     1,
     178,   216,   235,   176,   182,     1,   176,   178,     1,   178,
       1,   220,   178,     1,   178,     1,   178,     1,   178,     1,
     176,     1,   176,   178,   182,     1,   178,     1,   178,     1,
     178,     1,   178,   182,   182,     1,   176,   182,     1,   162,
     167,   168,   227,   230,     1,   227,   230,     1,   163,   164,
     165,   166,   227,   228,     1,   227,   230,     1,   228,     1,
     227,   230,     1,   228,     1,   227,   230,     1,   228,     1,
     228,     1,   228,     1,   228,     1,   228,     1,   227,     1,
     228,     1,   228,   245,   226,   226,   239,   169,   170,   184,
     238,   240,     1,   227,   230,   230,   248,   171,   172,   227,
     228,   229,   230,     1,   202,     1,   176,     1,   205,   176,
     210,   176,   210,   176,   210,   183,   189,   186,   176,   178,
     183,   236,   244,   250,   251,   252,   255,   178,   249,   250,
     256,   184,   184,   184,   184,   184,   176,   204,   176,   178,
     182,   180,   182,   180,   182,   180,   182,   180,   182,   182,
     180,   182,   180,   182,   180,   182,   180,   182,   182,   180,
     182,   180,   182,   180,   182,   180,   182,   182,   180,   182,
     180,   182,   180,   182,   180,   182,   182,   180,   182,   180,
     182,   180,   182,   182,   180,   182,   180,   182,   180,   182,
     182,   182,   180,   182,   182,   180,   182,   182,   182,   182,
     182,   182,   182,   182,   182,   182,   182,   182,   182,   180,
     182,   182,   182,   182,   182,   182,   182,   182,   182,   182,
     182,     1,   177,   178,   244,   249,   250,   251,   254,    63,
     216,   233,     1,   178,   235,     1,   176,   254,    63,   233,
       1,   178,   235,     1,   176,   254,    63,   233,     1,   178,
     235,     1,   176,   254,    63,   233,     1,   178,   235,     1,
     176,   254,     1,   203,   254,     1,   176,   254,     1,   203,
     254,     1,   176,   254,     1,    51,    53,    55,    57,   231,
       1,    40,   176,   254,     1,   176,   254,   177,   242,   243,
     182,   185,   226,   226,   239,    34,    63,     1,   178,   235,
     244,   254,   178,   244,   254,   186,   176,   176,   178,   244,
     254,   182,   182,   182,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   176,   178,   262,   263,   247,   187,   226,
     248,   180,   239,   239,   239,   239,   239,   189,   182,   182,
      39,   176,   176,   176,   176,    39,   176,   176,   176,   176,
      39,   176,   176,   176,   176,    39,   176,   176,   176,   176,
     176,   176,   176,   176,   176,   176,   178,   178,   178,   248,
     188,   186,   189,   185,   238,   176,     1,   180,   182,   182,
     210,   185,   185,   185,   185,   185,   176,   182,   182,   182,
     182,   182,   182,   182,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   182,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   182,   182,   182,   186,     1,   176,   216,
     176,   178,   242,   187,   263,   182,   187,   187
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:

/* Line 1455 of yacc.c  */
#line 430 "cfg.y"
    {;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 431 "cfg.y"
    {;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 432 "cfg.y"
    { yyerror(""); YYABORT;;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 439 "cfg.y"
    {rt=REQUEST_ROUTE;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 440 "cfg.y"
    {rt=FAILURE_ROUTE;;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 441 "cfg.y"
    {rt=ONREPLY_ROUTE;;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 442 "cfg.y"
    {rt=BRANCH_ROUTE;;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 443 "cfg.y"
    {rt=ONSEND_ROUTE;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 448 "cfg.y"
    {
		tmp=ip_addr2a((yyvsp[(1) - (1)].ipaddr));
		if (tmp==0) {
			LOG(L_CRIT, "ERROR: cfg. parser: bad ip "
					"address.\n");
			(yyval.strval)=0;
		} else {
			(yyval.strval)=pkg_malloc(strlen(tmp)+1);
			if ((yyval.strval)==0) {
				LOG(L_CRIT, "ERROR: cfg. parser: out of "
						"memory.\n");
			} else {
				strncpy((yyval.strval), tmp, strlen(tmp)+1);
			}
		}
	;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 464 "cfg.y"
    {
		(yyval.strval)=pkg_malloc(strlen((yyvsp[(1) - (1)].strval))+1);
		if ((yyval.strval)==0) {
				LOG(L_CRIT, "ERROR: cfg. parser: out of "
						"memory.\n");
		} else {
				strncpy((yyval.strval), (yyvsp[(1) - (1)].strval), strlen((yyvsp[(1) - (1)].strval))+1);
		}
	;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 473 "cfg.y"
    {
		(yyval.strval)=pkg_malloc(strlen((yyvsp[(1) - (1)].strval))+1);
		if ((yyval.strval)==0) {
				LOG(L_CRIT, "ERROR: cfg. parser: out of "
						"memory.\n");
		} else {
				strncpy((yyval.strval), (yyvsp[(1) - (1)].strval), strlen((yyvsp[(1) - (1)].strval))+1);
		}
	;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 484 "cfg.y"
    { (yyval.intval)=PROTO_UDP; ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 485 "cfg.y"
    { (yyval.intval)=PROTO_TCP; ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 486 "cfg.y"
    { (yyval.intval)=PROTO_TLS; ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 487 "cfg.y"
    { (yyval.intval)=0; ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 490 "cfg.y"
    { (yyval.intval)=(yyvsp[(1) - (1)].intval); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 491 "cfg.y"
    { (yyval.intval)=0; ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 494 "cfg.y"
    { (yyval.sockid)=mk_listen_id((yyvsp[(1) - (1)].strval), 0, 0); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 495 "cfg.y"
    { (yyval.sockid)=mk_listen_id((yyvsp[(1) - (3)].strval), 0, (yyvsp[(3) - (3)].intval)); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 496 "cfg.y"
    { (yyval.sockid)=mk_listen_id((yyvsp[(3) - (3)].strval), (yyvsp[(1) - (3)].intval), 0); ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 497 "cfg.y"
    { (yyval.sockid)=mk_listen_id((yyvsp[(3) - (5)].strval), (yyvsp[(1) - (5)].intval), (yyvsp[(5) - (5)].intval));;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 498 "cfg.y"
    { (yyval.sockid)=0; yyerror(" port number expected"); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 501 "cfg.y"
    {  (yyval.sockid)=(yyvsp[(1) - (1)].sockid) ; ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 502 "cfg.y"
    { (yyval.sockid)=(yyvsp[(1) - (2)].sockid); (yyval.sockid)->next=(yyvsp[(2) - (2)].sockid); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 506 "cfg.y"
    { yyerror("flag list expected\n"); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 512 "cfg.y"
    { if (register_flag((yyvsp[(1) - (1)].strval),-1)<0)
								yyerror("register flag failed");
						;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 515 "cfg.y"
    {
						if (register_flag((yyvsp[(1) - (3)].strval), (yyvsp[(3) - (3)].intval))<0)
								yyerror("register flag failed");
										;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 521 "cfg.y"
    { (yyval.strval)=(yyvsp[(1) - (1)].strval); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 522 "cfg.y"
    { (yyval.strval)=(yyvsp[(1) - (1)].strval); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 527 "cfg.y"
    { yyerror("avpflag list expected\n"); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 534 "cfg.y"
    {
		if (register_avpflag((yyvsp[(1) - (1)].strval))==0)
			yyerror("cannot declare avpflag");
	;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 540 "cfg.y"
    { debug=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 541 "cfg.y"
    { yyerror("number  expected"); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 542 "cfg.y"
    { dont_fork= ! (yyvsp[(3) - (3)].intval); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 543 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 544 "cfg.y"
    { if (!config_check) log_stderr=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 545 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 546 "cfg.y"
    {
		if ( (i_tmp=str2facility((yyvsp[(3) - (3)].strval)))==-1)
			yyerror("bad facility (see syslog(3) man page)");
		if (!config_check)
			log_facility=i_tmp;
	;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 552 "cfg.y"
    { yyerror("ID expected"); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 553 "cfg.y"
    { received_dns|= ((yyvsp[(3) - (3)].intval))?DO_DNS:0; ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 554 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 555 "cfg.y"
    { received_dns|= ((yyvsp[(3) - (3)].intval))?DO_REV_DNS:0; ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 556 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 557 "cfg.y"
    { dns_try_ipv6=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 558 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 559 "cfg.y"
    { dns_retr_time=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 560 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 561 "cfg.y"
    { dns_retr_no=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 562 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 563 "cfg.y"
    { dns_servers_no=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 564 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 565 "cfg.y"
    { dns_search_list=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 566 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 567 "cfg.y"
    { IF_DNS_CACHE(use_dns_cache=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 568 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 569 "cfg.y"
    { IF_DNS_FAILOVER(use_dns_failover=(yyvsp[(3) - (3)].intval));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 570 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 571 "cfg.y"
    { IF_DNS_CACHE(dns_flags=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 572 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 573 "cfg.y"
    { IF_DNS_CACHE(dns_neg_cache_ttl=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 574 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 575 "cfg.y"
    { IF_DNS_CACHE(dns_cache_max_ttl=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 576 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 577 "cfg.y"
    { IF_DNS_CACHE(dns_cache_min_ttl=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 578 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 579 "cfg.y"
    { IF_DNS_CACHE(dns_cache_max_mem=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 580 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 581 "cfg.y"
    { IF_DNS_CACHE(dns_timer_interval=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 582 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 583 "cfg.y"
    { IF_DST_BLACKLIST(use_dst_blacklist=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 584 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 585 "cfg.y"
    { IF_DST_BLACKLIST(blst_max_mem=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 586 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 587 "cfg.y"
    { IF_DST_BLACKLIST(blst_timeout=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 588 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 589 "cfg.y"
    { IF_DST_BLACKLIST(blst_timer_interval=(yyvsp[(3) - (3)].intval));;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 590 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 591 "cfg.y"
    { port_no=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 592 "cfg.y"
    {
		#ifdef STATS
				stat_file=(yyvsp[(3) - (3)].strval);
		#endif
	;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 597 "cfg.y"
    { maxbuffer=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 598 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 599 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 600 "cfg.y"
    { children_no=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 601 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 602 "cfg.y"
    { check_via=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 603 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 604 "cfg.y"
    { syn_branch=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 605 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 606 "cfg.y"
    { memlog=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 607 "cfg.y"
    { yyerror("int value expected"); ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 608 "cfg.y"
    { memdbg=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 609 "cfg.y"
    { yyerror("int value expected"); ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 610 "cfg.y"
    { sip_warning=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 611 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 612 "cfg.y"
    { user=(yyvsp[(3) - (3)].strval); ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 613 "cfg.y"
    { user=(yyvsp[(3) - (3)].strval); ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 614 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 615 "cfg.y"
    { group=(yyvsp[(3) - (3)].strval); ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 616 "cfg.y"
    { group=(yyvsp[(3) - (3)].strval); ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 617 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 618 "cfg.y"
    { chroot_dir=(yyvsp[(3) - (3)].strval); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 619 "cfg.y"
    { chroot_dir=(yyvsp[(3) - (3)].strval); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 620 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 621 "cfg.y"
    { working_dir=(yyvsp[(3) - (3)].strval); ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 622 "cfg.y"
    { working_dir=(yyvsp[(3) - (3)].strval); ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 623 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 624 "cfg.y"
    { mhomed=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 625 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 626 "cfg.y"
    {
		#ifdef USE_TCP
			tcp_disable=(yyvsp[(3) - (3)].intval);
		#else
			warn("tcp support not compiled in");
		#endif
	;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 633 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 634 "cfg.y"
    {
		#ifdef USE_TCP
			tcp_accept_aliases=(yyvsp[(3) - (3)].intval);
		#else
			warn("tcp support not compiled in");
		#endif
	;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 641 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 642 "cfg.y"
    {
		#ifdef USE_TCP
			tcp_children_no=(yyvsp[(3) - (3)].intval);
		#else
			warn("tcp support not compiled in");
		#endif
	;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 649 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 650 "cfg.y"
    {
		#ifdef USE_TCP
			tcp_connect_timeout=(yyvsp[(3) - (3)].intval);
		#else
			warn("tcp support not compiled in");
		#endif
	;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 657 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 658 "cfg.y"
    {
		#ifdef USE_TCP
			tcp_send_timeout=(yyvsp[(3) - (3)].intval);
		#else
			warn("tcp support not compiled in");
		#endif
	;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 665 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 666 "cfg.y"
    {
		#ifdef USE_TCP
			tcp_con_lifetime=(yyvsp[(3) - (3)].intval);
		#else
			warn("tcp support not compiled in");
		#endif
	;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 673 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 674 "cfg.y"
    {
		#ifdef USE_TCP
			tcp_poll_method=get_poll_type((yyvsp[(3) - (3)].strval));
			if (tcp_poll_method==POLL_NONE) {
				LOG(L_CRIT, "bad poll method name:"
						" %s\n, try one of %s.\n",
						(yyvsp[(3) - (3)].strval), poll_support);
				yyerror("bad tcp_poll_method "
						"value");
			}
		#else
			warn("tcp support not compiled in");
		#endif
	;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 688 "cfg.y"
    {
		#ifdef USE_TCP
			tcp_poll_method=get_poll_type((yyvsp[(3) - (3)].strval));
			if (tcp_poll_method==POLL_NONE) {
				LOG(L_CRIT, "bad poll method name:"
						" %s\n, try one of %s.\n",
						(yyvsp[(3) - (3)].strval), poll_support);
				yyerror("bad tcp_poll_method "
						"value");
			}
		#else
			warn("tcp support not compiled in");
		#endif
	;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 702 "cfg.y"
    { yyerror("poll method name expected"); ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 703 "cfg.y"
    {
		#ifdef USE_TCP
			tcp_max_connections=(yyvsp[(3) - (3)].intval);
		#else
			warn("tcp support not compiled in");
		#endif
	;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 710 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 711 "cfg.y"
    {
		#ifdef USE_TLS
			tls_disable=(yyvsp[(3) - (3)].intval);
		#else
			warn("tls support not compiled in");
		#endif
	;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 718 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 719 "cfg.y"
    {
		#ifdef USE_TLS
			tls_disable=!((yyvsp[(3) - (3)].intval));
		#else
			warn("tls support not compiled in");
		#endif
	;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 726 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 727 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_log=(yyvsp[(3) - (3)].intval);
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 734 "cfg.y"
    { yyerror("int value expected"); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 735 "cfg.y"
    {
		#ifdef USE_TLS
			tls_port_no=(yyvsp[(3) - (3)].intval);
		#else
			warn("tls support not compiled in");
		#endif
	;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 742 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 743 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_method=TLS_USE_SSLv23;
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 750 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_method=TLS_USE_SSLv2;
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 757 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_method=TLS_USE_SSLv3;
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 764 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_method=TLS_USE_TLSv1;
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 771 "cfg.y"
    {
		#ifdef CORE_TLS
			yyerror("SSLv23, SSLv2, SSLv3 or TLSv1 expected");
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 778 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_verify_cert=(yyvsp[(3) - (3)].intval);
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 785 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 786 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_require_cert=(yyvsp[(3) - (3)].intval);
		#else
			warn( "tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 793 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 794 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_cert_file=(yyvsp[(3) - (3)].strval);
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 801 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 802 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_pkey_file=(yyvsp[(3) - (3)].strval);
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 809 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 810 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_ca_file=(yyvsp[(3) - (3)].strval);
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 817 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 818 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_handshake_timeout=(yyvsp[(3) - (3)].intval);
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 825 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 826 "cfg.y"
    {
		#ifdef CORE_TLS
			tls_send_timeout=(yyvsp[(3) - (3)].intval);
		#else
			warn("tls-in-core support not compiled in");
		#endif
	;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 833 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 834 "cfg.y"
    { server_signature=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 835 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 836 "cfg.y"
    { reply_to_via=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 837 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 838 "cfg.y"
    {
		for(lst_tmp=(yyvsp[(3) - (3)].sockid); lst_tmp; lst_tmp=lst_tmp->next) {
			if (add_listen_iface(lst_tmp->name, lst_tmp->port, lst_tmp->proto, 0)!=0) {
				LOG(L_CRIT,  "ERROR: cfg. parser: failed to add listen address\n");
				break;
			}
		}
	;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 846 "cfg.y"
    { yyerror("ip address or hostname expected"); ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 847 "cfg.y"
    {
		for(lst_tmp=(yyvsp[(3) - (3)].sockid); lst_tmp; lst_tmp=lst_tmp->next)
			add_alias(lst_tmp->name, strlen(lst_tmp->name), lst_tmp->port, lst_tmp->proto);
	;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 851 "cfg.y"
    { yyerror(" hostname expected"); ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 852 "cfg.y"
    {
		default_global_address.s=(yyvsp[(3) - (3)].strval);
		default_global_address.len=strlen((yyvsp[(3) - (3)].strval));
	;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 856 "cfg.y"
    {yyerror("ip address or hostname expected"); ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 857 "cfg.y"
    {
		tmp=int2str((yyvsp[(3) - (3)].intval), &i_tmp);
		if ((default_global_port.s=pkg_malloc(i_tmp))==0) {
			LOG(L_CRIT, "ERROR: cfg. parser: out of memory.\n");
			default_global_port.len=0;
		} else {
			default_global_port.len=i_tmp;
			memcpy(default_global_port.s, tmp, default_global_port.len);
		};
	;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 867 "cfg.y"
    {yyerror("ip address or hostname expected"); ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 868 "cfg.y"
    { disable_core_dump=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 869 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 870 "cfg.y"
    { open_files_limit=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 871 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 872 "cfg.y"
    {
		#ifdef USE_MCAST
			mcast_loopback=(yyvsp[(3) - (3)].intval);
		#else
			warn("no multicast support compiled in");
		#endif
	;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 879 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 880 "cfg.y"
    {
		#ifdef USE_MCAST
			mcast_ttl=(yyvsp[(3) - (3)].intval);
		#else
			warn("no multicast support compiled in");
		#endif
	;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 887 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 888 "cfg.y"
    { tos=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 889 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 890 "cfg.y"
    { ser_kill_timeout=(yyvsp[(3) - (3)].intval); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 891 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 892 "cfg.y"
    { IF_STUN(stun_refresh_interval=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 893 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 894 "cfg.y"
    { IF_STUN(stun_allow_stun=(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 895 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 896 "cfg.y"
    { IF_STUN(stun_allow_fp=(yyvsp[(3) - (3)].intval)) ; ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 897 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 898 "cfg.y"
    { yyerror("unknown config variable"); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 901 "cfg.y"
    {
		DBG("loading module %s\n", (yyvsp[(2) - (2)].strval));
			if (load_module((yyvsp[(2) - (2)].strval))!=0) {
				yyerror("failed to load module");
			}
	;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 907 "cfg.y"
    { yyerror("string expected"); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 908 "cfg.y"
    {
		if (set_mod_param_regex((yyvsp[(3) - (8)].strval), (yyvsp[(5) - (8)].strval), PARAM_STRING, (yyvsp[(7) - (8)].strval)) != 0) {
			 yyerror("Can't set module parameter");
		}
	;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 913 "cfg.y"
    {
		if (set_mod_param_regex((yyvsp[(3) - (8)].strval), (yyvsp[(5) - (8)].strval), PARAM_INT, (void*)(yyvsp[(7) - (8)].intval)) != 0) {
			 yyerror("Can't set module parameter");
		}
	;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 918 "cfg.y"
    { yyerror("Invalid arguments"); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 921 "cfg.y"
    { (yyval.ipaddr)=(yyvsp[(1) - (1)].ipaddr); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 922 "cfg.y"
    { (yyval.ipaddr)=(yyvsp[(1) - (1)].ipaddr); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 925 "cfg.y"
    {
		(yyval.ipaddr)=pkg_malloc(sizeof(struct ip_addr));
		if ((yyval.ipaddr)==0) {
			LOG(L_CRIT, "ERROR: cfg. parser: out of memory.\n");
		} else {
			memset((yyval.ipaddr), 0, sizeof(struct ip_addr));
			(yyval.ipaddr)->af=AF_INET;
			(yyval.ipaddr)->len=4;
			if (((yyvsp[(1) - (7)].intval)>255) || ((yyvsp[(1) - (7)].intval)<0) ||
				((yyvsp[(3) - (7)].intval)>255) || ((yyvsp[(3) - (7)].intval)<0) ||
				((yyvsp[(5) - (7)].intval)>255) || ((yyvsp[(5) - (7)].intval)<0) ||
				((yyvsp[(7) - (7)].intval)>255) || ((yyvsp[(7) - (7)].intval)<0)) {
				yyerror("invalid ipv4 address");
				(yyval.ipaddr)->u.addr32[0]=0;
				/* $$=0; */
			} else {
				(yyval.ipaddr)->u.addr[0]=(yyvsp[(1) - (7)].intval);
				(yyval.ipaddr)->u.addr[1]=(yyvsp[(3) - (7)].intval);
				(yyval.ipaddr)->u.addr[2]=(yyvsp[(5) - (7)].intval);
				(yyval.ipaddr)->u.addr[3]=(yyvsp[(7) - (7)].intval);
				/*
				$$=htonl( ($1<<24)|
				($3<<16)| ($5<<8)|$7 );
				*/
			}
		}
	;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 954 "cfg.y"
    {
		(yyval.ipaddr)=pkg_malloc(sizeof(struct ip_addr));
		if ((yyval.ipaddr)==0) {
			LOG(L_CRIT, "ERROR: cfg. parser: out of memory.\n");
		} else {
			memset((yyval.ipaddr), 0, sizeof(struct ip_addr));
			(yyval.ipaddr)->af=AF_INET6;
			(yyval.ipaddr)->len=16;
		#ifdef USE_IPV6
			if (inet_pton(AF_INET6, (yyvsp[(1) - (1)].strval), (yyval.ipaddr)->u.addr)<=0) {
				yyerror("bad ipv6 address");
			}
		#else
			yyerror("ipv6 address & no ipv6 support compiled in");
			YYABORT;
		#endif
		}
	;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 974 "cfg.y"
    { (yyval.ipaddr)=(yyvsp[(1) - (1)].ipaddr); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 975 "cfg.y"
    {(yyval.ipaddr)=(yyvsp[(2) - (3)].ipaddr); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 979 "cfg.y"
    {
					tmp=int2str((yyvsp[(1) - (1)].intval), &i_tmp);
					if (((yyval.strval)=pkg_malloc(i_tmp+1))==0) {
						yyerror("out of  memory");
						YYABORT;
					} else {
						memcpy((yyval.strval), tmp, i_tmp);
						(yyval.strval)[i_tmp]=0;
					}
						;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 989 "cfg.y"
    { (yyval.strval)=(yyvsp[(1) - (1)].strval); ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 990 "cfg.y"
    { (yyval.strval)=(yyvsp[(1) - (1)].strval); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 994 "cfg.y"
    { push((yyvsp[(3) - (4)].action), &main_rt.rlist[DEFAULT_RT]); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 995 "cfg.y"
    {
		i_tmp=route_get(&main_rt, (yyvsp[(3) - (7)].strval));
		if (i_tmp==-1){
			yyerror("internal error");
			YYABORT;
		}
		if (main_rt.rlist[i_tmp]){
			yyerror("duplicate route");
			YYABORT;
		}
		push((yyvsp[(6) - (7)].action), &main_rt.rlist[i_tmp]);
	;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1007 "cfg.y"
    { yyerror("invalid  route  statement"); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1010 "cfg.y"
    {
									push((yyvsp[(3) - (4)].action), &failure_rt.rlist[DEFAULT_RT]);
										;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1013 "cfg.y"
    {
		i_tmp=route_get(&failure_rt, (yyvsp[(3) - (7)].strval));
		if (i_tmp==-1){
			yyerror("internal error");
			YYABORT;
		}
		if (failure_rt.rlist[i_tmp]){
			yyerror("duplicate route");
			YYABORT;
		}
		push((yyvsp[(6) - (7)].action), &failure_rt.rlist[i_tmp]);
	;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1025 "cfg.y"
    { yyerror("invalid failure_route statement"); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1028 "cfg.y"
    {
									push((yyvsp[(3) - (4)].action), &onreply_rt.rlist[DEFAULT_RT]);
										;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1031 "cfg.y"
    {
		i_tmp=route_get(&onreply_rt, (yyvsp[(3) - (7)].strval));
		if (i_tmp==-1){
			yyerror("internal error");
			YYABORT;
		}
		if (onreply_rt.rlist[i_tmp]){
			yyerror("duplicate route");
			YYABORT;
		}
		push((yyvsp[(6) - (7)].action), &onreply_rt.rlist[i_tmp]);
	;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1043 "cfg.y"
    { yyerror("invalid onreply_route statement"); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1046 "cfg.y"
    {
									push((yyvsp[(3) - (4)].action), &branch_rt.rlist[DEFAULT_RT]);
										;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1049 "cfg.y"
    {
		i_tmp=route_get(&branch_rt, (yyvsp[(3) - (7)].strval));
		if (i_tmp==-1){
			yyerror("internal error");
			YYABORT;
		}
		if (branch_rt.rlist[i_tmp]){
			yyerror("duplicate route");
			YYABORT;
		}
		push((yyvsp[(6) - (7)].action), &branch_rt.rlist[i_tmp]);
	;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1061 "cfg.y"
    { yyerror("invalid branch_route statement"); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1063 "cfg.y"
    {
									push((yyvsp[(3) - (4)].action), &onsend_rt.rlist[DEFAULT_RT]);
												;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1066 "cfg.y"
    {
		i_tmp=route_get(&onsend_rt, (yyvsp[(3) - (7)].strval));
		if (i_tmp==-1){
			yyerror("internal error");
			YYABORT;
		}
		if (onsend_rt.rlist[i_tmp]){
			yyerror("duplicate route");
			YYABORT;
		}
		push((yyvsp[(6) - (7)].action), &onsend_rt.rlist[i_tmp]);
	;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1078 "cfg.y"
    { yyerror("invalid onsend_route statement"); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1100 "cfg.y"
    { (yyval.expr)=mk_exp(LOGAND_OP, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1101 "cfg.y"
    { (yyval.expr)=mk_exp(LOGOR_OP, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));  ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1102 "cfg.y"
    { (yyval.expr)=mk_exp(NOT_OP, (yyvsp[(2) - (2)].expr), 0);  ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1103 "cfg.y"
    { (yyval.expr)=(yyvsp[(2) - (3)].expr); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1104 "cfg.y"
    { (yyval.expr)=(yyvsp[(1) - (1)].expr); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1107 "cfg.y"
    {(yyval.intval)=EQUAL_OP; ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1108 "cfg.y"
    {(yyval.intval)=DIFF_OP; ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1110 "cfg.y"
    {(yyval.intval)=(yyvsp[(1) - (1)].intval); ;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1111 "cfg.y"
    {(yyval.intval)=GT_OP; ;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1112 "cfg.y"
    {(yyval.intval)=LT_OP; ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1113 "cfg.y"
    {(yyval.intval)=GTE_OP; ;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1114 "cfg.y"
    {(yyval.intval)=LTE_OP; ;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1117 "cfg.y"
    { (yyval.intval)= BINOR_OP; ;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1118 "cfg.y"
    { (yyval.intval) = BINAND_OP; ;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1121 "cfg.y"
    {(yyval.intval)=(yyvsp[(1) - (1)].intval); ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1122 "cfg.y"
    {(yyval.intval)=MATCH_OP; ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1125 "cfg.y"
    {(yyval.intval)=URI_O;;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1126 "cfg.y"
    {(yyval.intval)=FROM_URI_O;;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1127 "cfg.y"
    {(yyval.intval)=TO_URI_O;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1131 "cfg.y"
    {(yyval.expr)= mk_elem((yyvsp[(2) - (3)].intval), METHOD_O, 0, STRING_ST, (yyvsp[(3) - (3)].strval));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1132 "cfg.y"
    {(yyval.expr) = mk_elem((yyvsp[(2) - (3)].intval), METHOD_O, 0, AVP_ST, (yyvsp[(3) - (3)].attr)); ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1133 "cfg.y"
    {(yyval.expr) = mk_elem((yyvsp[(2) - (3)].intval), METHOD_O, 0, SELECT_ST, (yyvsp[(3) - (3)].select)); ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1134 "cfg.y"
    {(yyval.expr) = mk_elem((yyvsp[(2) - (3)].intval), METHOD_O, 0, STRING_ST,(yyvsp[(3) - (3)].strval)); ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1135 "cfg.y"
    { (yyval.expr)=0; yyerror("string expected"); ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1136 "cfg.y"
    { (yyval.expr)=0; yyerror("invalid operator,== , !=, or =~ expected"); ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1137 "cfg.y"
    {(yyval.expr) = mk_elem((yyvsp[(2) - (3)].intval), (yyvsp[(1) - (3)].intval), 0, STRING_ST, (yyvsp[(3) - (3)].strval)); ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1138 "cfg.y"
    {(yyval.expr) = mk_elem((yyvsp[(2) - (3)].intval), (yyvsp[(1) - (3)].intval), 0, STRING_ST, (yyvsp[(3) - (3)].strval)); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1139 "cfg.y"
    {(yyval.expr) = mk_elem((yyvsp[(2) - (3)].intval), (yyvsp[(1) - (3)].intval), 0, AVP_ST, (yyvsp[(3) - (3)].attr)); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1140 "cfg.y"
    {(yyval.expr) = mk_elem((yyvsp[(2) - (3)].intval), (yyvsp[(1) - (3)].intval), 0, SELECT_ST, (yyvsp[(3) - (3)].select)); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1141 "cfg.y"
    {(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), (yyvsp[(1) - (3)].intval), 0, MYSELF_ST, 0); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1142 "cfg.y"
    { (yyval.expr)=0; yyerror("string or MYSELF expected"); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1143 "cfg.y"
    { (yyval.expr)=0; yyerror("invalid operator, == , != or =~ expected"); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1145 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SRCPORT_O, 0, NUMBER_ST, (void*)(yyvsp[(3) - (3)].intval) ); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1146 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SRCPORT_O, 0, AVP_ST, (void*)(yyvsp[(3) - (3)].attr) ); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1147 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1148 "cfg.y"
    { (yyval.expr)=0; yyerror("==, !=, <,>, >= or <=  expected"); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1150 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), DSTPORT_O, 0, NUMBER_ST, (void*)(yyvsp[(3) - (3)].intval) ); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1151 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), DSTPORT_O, 0, AVP_ST, (void*)(yyvsp[(3) - (3)].attr) ); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1152 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1153 "cfg.y"
    { (yyval.expr)=0; yyerror("==, !=, <,>, >= or <=  expected"); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1155 "cfg.y"
    {
		onsend_check("snd_port");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDPORT_O, 0, NUMBER_ST, (void*)(yyvsp[(3) - (3)].intval) );
	;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1159 "cfg.y"
    {
		onsend_check("snd_port");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDPORT_O, 0, AVP_ST, (void*)(yyvsp[(3) - (3)].attr) );
	;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1163 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1164 "cfg.y"
    { (yyval.expr)=0; yyerror("==, !=, <,>, >= or <=  expected"); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1166 "cfg.y"
    {
		onsend_check("to_port");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), TOPORT_O, 0, NUMBER_ST, (void*)(yyvsp[(3) - (3)].intval) );
	;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1170 "cfg.y"
    {
		onsend_check("to_port");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), TOPORT_O, 0, AVP_ST, (void*)(yyvsp[(3) - (3)].attr) );
	;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1174 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1175 "cfg.y"
    { (yyval.expr)=0; yyerror("==, !=, <,>, >= or <=  expected"); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1177 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), PROTO_O, 0, NUMBER_ST, (void*)(yyvsp[(3) - (3)].intval) ); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1178 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), PROTO_O, 0, AVP_ST, (void*)(yyvsp[(3) - (3)].attr) ); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1179 "cfg.y"
    { (yyval.expr)=0; yyerror("protocol expected (udp, tcp or tls)"); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1181 "cfg.y"
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1183 "cfg.y"
    {
		onsend_check("snd_proto");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDPROTO_O, 0, NUMBER_ST, (void*)(yyvsp[(3) - (3)].intval) );
	;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1187 "cfg.y"
    {
		onsend_check("snd_proto");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDPROTO_O, 0, AVP_ST, (void*)(yyvsp[(3) - (3)].attr) );
	;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1191 "cfg.y"
    { (yyval.expr)=0; yyerror("protocol expected (udp, tcp or tls)"); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1192 "cfg.y"
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1194 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), AF_O, 0, NUMBER_ST,(void *) (yyvsp[(3) - (3)].intval) ); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1195 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), AF_O, 0, AVP_ST,(void *) (yyvsp[(3) - (3)].attr) ); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1196 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1197 "cfg.y"
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1199 "cfg.y"
    {
		onsend_check("snd_af");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDAF_O, 0, NUMBER_ST, (void *) (yyvsp[(3) - (3)].intval) ); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1202 "cfg.y"
    {
		onsend_check("snd_af");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDAF_O, 0, AVP_ST, (void *) (yyvsp[(3) - (3)].attr) );
	;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1206 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1207 "cfg.y"
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1209 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), MSGLEN_O, 0, NUMBER_ST, (void *) (yyvsp[(3) - (3)].intval) ); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1210 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), MSGLEN_O, 0, AVP_ST, (void *) (yyvsp[(3) - (3)].attr) ); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1211 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), MSGLEN_O, 0, NUMBER_ST, (void *) BUF_SIZE); ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1212 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1213 "cfg.y"
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1215 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), RETCODE_O, 0, NUMBER_ST, (void *) (yyvsp[(3) - (3)].intval) ); ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1216 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), RETCODE_O, 0, AVP_ST, (void *) (yyvsp[(3) - (3)].attr) ); ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1217 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1218 "cfg.y"
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); ;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1220 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SRCIP_O, 0, NET_ST, (yyvsp[(3) - (3)].ipnet)); ;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1221 "cfg.y"
    {
		s_tmp.s=(yyvsp[(3) - (3)].strval);
		s_tmp.len=strlen((yyvsp[(3) - (3)].strval));
		ip_tmp=str2ip(&s_tmp);
		if (ip_tmp==0)
			ip_tmp=str2ip6(&s_tmp);
		if (ip_tmp) {
			(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SRCIP_O, 0, NET_ST, mk_net_bitlen(ip_tmp, ip_tmp->len*8) );
		} else {
			(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SRCIP_O, 0, STRING_ST, (yyvsp[(3) - (3)].strval));
		}
	;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1233 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SRCIP_O, 0, STRING_ST, (yyvsp[(3) - (3)].strval)); ;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1234 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SRCIP_O, 0, MYSELF_ST, 0);
							;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1236 "cfg.y"
    { (yyval.expr)=0; yyerror( "ip address or hostname expected" ); ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1237 "cfg.y"
    { (yyval.expr)=0; yyerror("invalid operator, ==, != or =~ expected");;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1238 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[(2) - (3)].intval), DSTIP_O, 0, NET_ST, (void*)(yyvsp[(3) - (3)].ipnet)); ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1239 "cfg.y"
    {
		s_tmp.s=(yyvsp[(3) - (3)].strval);
		s_tmp.len=strlen((yyvsp[(3) - (3)].strval));
		ip_tmp=str2ip(&s_tmp);
		if (ip_tmp==0)
			ip_tmp=str2ip6(&s_tmp);
		if (ip_tmp) {
			(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), DSTIP_O, 0, NET_ST, mk_net_bitlen(ip_tmp, ip_tmp->len*8) );
		} else {
			(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), DSTIP_O, 0, STRING_ST, (yyvsp[(3) - (3)].strval));
		}
	;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1251 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[(2) - (3)].intval), DSTIP_O, 0, STRING_ST, (yyvsp[(3) - (3)].strval)); ;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1252 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[(2) - (3)].intval), DSTIP_O, 0, MYSELF_ST, 0); ;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1253 "cfg.y"
    { (yyval.expr)=0; yyerror( "ip address or hostname expected" ); ;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1254 "cfg.y"
    { (yyval.expr)=0; yyerror("invalid operator, ==, != or =~ expected"); ;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1255 "cfg.y"
    {
		onsend_check("snd_ip");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDIP_O, 0, NET_ST, (yyvsp[(3) - (3)].ipnet));
	;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1259 "cfg.y"
    {
		onsend_check("snd_ip");
		s_tmp.s=(yyvsp[(3) - (3)].strval);
		s_tmp.len=strlen((yyvsp[(3) - (3)].strval));
		ip_tmp=str2ip(&s_tmp);
		if (ip_tmp==0)
			ip_tmp=str2ip6(&s_tmp);
		if (ip_tmp) {
			(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDIP_O, 0, NET_ST, mk_net_bitlen(ip_tmp, ip_tmp->len*8) );
		} else {
			(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDIP_O, 0, STRING_ST, (yyvsp[(3) - (3)].strval));
		}
	;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1272 "cfg.y"
    {
		onsend_check("snd_ip");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDIP_O, 0, STRING_ST, (yyvsp[(3) - (3)].strval));
	;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1276 "cfg.y"
    {
		onsend_check("snd_ip");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDIP_O, 0, MYSELF_ST, 0);
	;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1280 "cfg.y"
    { (yyval.expr)=0; yyerror( "ip address or hostname expected" ); ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1281 "cfg.y"
    { (yyval.expr)=0; yyerror("invalid operator, ==, != or =~ expected"); ;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1282 "cfg.y"
    {
		onsend_check("to_ip");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), TOIP_O, 0, NET_ST, (yyvsp[(3) - (3)].ipnet));
	;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1286 "cfg.y"
    {
		onsend_check("to_ip");
		s_tmp.s=(yyvsp[(3) - (3)].strval);
		s_tmp.len=strlen((yyvsp[(3) - (3)].strval));
		ip_tmp=str2ip(&s_tmp);
		if (ip_tmp==0)
			ip_tmp=str2ip6(&s_tmp);
		if (ip_tmp) {
			(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), TOIP_O, 0, NET_ST, mk_net_bitlen(ip_tmp, ip_tmp->len*8) );
		} else {
			(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), TOIP_O, 0, STRING_ST, (yyvsp[(3) - (3)].strval));
		}
	;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1299 "cfg.y"
    {
		onsend_check("to_ip");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), TOIP_O, 0, STRING_ST, (yyvsp[(3) - (3)].strval));
	;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1303 "cfg.y"
    {
		onsend_check("to_ip");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), TOIP_O, 0, MYSELF_ST, 0);
	;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1307 "cfg.y"
    { (yyval.expr)=0; yyerror( "ip address or hostname expected" ); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1308 "cfg.y"
    { (yyval.expr)=0; yyerror("invalid operator, ==, != or =~ expected"); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1310 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), (yyvsp[(3) - (3)].intval), 0, MYSELF_ST, 0); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1311 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SRCIP_O, 0, MYSELF_ST, 0); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1312 "cfg.y"
    { (yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), DSTIP_O, 0, MYSELF_ST, 0); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1313 "cfg.y"
    {
		onsend_check("snd_ip");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), SNDIP_O, 0, MYSELF_ST, 0);
	;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1317 "cfg.y"
    {
		onsend_check("to_ip");
		(yyval.expr)=mk_elem((yyvsp[(2) - (3)].intval), TOIP_O, 0, MYSELF_ST, 0);
	;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1321 "cfg.y"
    { (yyval.expr)=0; yyerror(" URI, SRCIP or DSTIP expected"); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1322 "cfg.y"
    { (yyval.expr)=0; yyerror ("invalid operator, == or != expected"); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1323 "cfg.y"
    { (yyval.expr)=mk_elem( NO_OP, ACTION_O, 0, ACTIONS_ST, (yyvsp[(1) - (1)].action));  ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1324 "cfg.y"
    { (yyval.expr)=mk_elem( NO_OP, NUMBER_O, 0, NUMBER_ST, (void*)(yyvsp[(1) - (1)].intval) ); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1326 "cfg.y"
    {(yyval.expr)=mk_elem( NO_OP, AVP_O, (void*)(yyvsp[(1) - (1)].attr), 0, 0); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1327 "cfg.y"
    {(yyval.expr)=mk_elem( (yyvsp[(2) - (3)].intval), AVP_O, (void*)(yyvsp[(1) - (3)].attr), STRING_ST, (yyvsp[(3) - (3)].strval)); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1328 "cfg.y"
    {(yyval.expr)=mk_elem( (yyvsp[(2) - (3)].intval), AVP_O, (void*)(yyvsp[(1) - (3)].attr), SELECT_ST, (yyvsp[(3) - (3)].select)); ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1329 "cfg.y"
    {(yyval.expr)=mk_elem( (yyvsp[(2) - (3)].intval), AVP_O, (void*)(yyvsp[(1) - (3)].attr), NUMBER_ST, (void*)(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1330 "cfg.y"
    {(yyval.expr)=mk_elem( (yyvsp[(2) - (3)].intval), AVP_O, (void*)(yyvsp[(1) - (3)].attr), NUMBER_ST, (void*)(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1331 "cfg.y"
    {(yyval.expr)=mk_elem( (yyvsp[(2) - (3)].intval), AVP_O, (void*)(yyvsp[(1) - (3)].attr), AVP_ST, (void*)(yyvsp[(3) - (3)].attr)); ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1333 "cfg.y"
    { (yyval.expr)=mk_elem( NO_OP, SELECT_O, (yyvsp[(1) - (1)].select), 0, 0); ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1334 "cfg.y"
    { (yyval.expr)=mk_elem( (yyvsp[(2) - (3)].intval), SELECT_O, (yyvsp[(1) - (3)].select), STRING_ST, (yyvsp[(3) - (3)].strval)); ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1335 "cfg.y"
    { (yyval.expr)=mk_elem( (yyvsp[(2) - (3)].intval), SELECT_O, (yyvsp[(1) - (3)].select), AVP_ST, (void*)(yyvsp[(3) - (3)].attr)); ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1336 "cfg.y"
    { (yyval.expr)=mk_elem( (yyvsp[(2) - (3)].intval), SELECT_O, (yyvsp[(1) - (3)].select), SELECT_ST, (yyvsp[(3) - (3)].select)); ;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1339 "cfg.y"
    { (yyval.ipnet)=mk_net((yyvsp[(1) - (3)].ipaddr), (yyvsp[(3) - (3)].ipaddr)); ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1340 "cfg.y"
    {
		if (((yyvsp[(3) - (3)].intval)<0) || ((yyvsp[(3) - (3)].intval)>(yyvsp[(1) - (3)].ipaddr)->len*8)) {
			yyerror("invalid bit number in netmask");
			(yyval.ipnet)=0;
		} else {
			(yyval.ipnet)=mk_net_bitlen((yyvsp[(1) - (3)].ipaddr), (yyvsp[(3) - (3)].intval));
		/*
			$$=mk_net($1, htonl( ($3)?~( (1<<(32-$3))-1 ):0 ) );
		*/
		}
	;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1351 "cfg.y"
    { (yyval.ipnet)=mk_net_bitlen((yyvsp[(1) - (1)].ipaddr), (yyvsp[(1) - (1)].ipaddr)->len*8); ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1352 "cfg.y"
    { (yyval.ipnet)=0; yyerror("netmask (eg:255.0.0.0 or 8) expected"); ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1355 "cfg.y"
    {(yyval.strval)=".";;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1356 "cfg.y"
    {(yyval.strval)="-"; ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1360 "cfg.y"
    { (yyval.strval)=(yyvsp[(1) - (1)].strval); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1361 "cfg.y"
    {
		(yyval.strval)=(char*)pkg_malloc(strlen((yyvsp[(1) - (3)].strval))+1+strlen((yyvsp[(3) - (3)].strval))+1);
		if ((yyval.strval)==0) {
			LOG(L_CRIT, "ERROR: cfg. parser: memory allocation failure while parsing host\n");
		} else {
			memcpy((yyval.strval), (yyvsp[(1) - (3)].strval), strlen((yyvsp[(1) - (3)].strval)));
			(yyval.strval)[strlen((yyvsp[(1) - (3)].strval))]=*(yyvsp[(2) - (3)].strval);
			memcpy((yyval.strval)+strlen((yyvsp[(1) - (3)].strval))+1, (yyvsp[(3) - (3)].strval), strlen((yyvsp[(3) - (3)].strval)));
			(yyval.strval)[strlen((yyvsp[(1) - (3)].strval))+1+strlen((yyvsp[(3) - (3)].strval))]=0;
		}
		pkg_free((yyvsp[(1) - (3)].strval));
		pkg_free((yyvsp[(3) - (3)].strval));
	;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1374 "cfg.y"
    { (yyval.strval)=0; pkg_free((yyvsp[(1) - (3)].strval)); yyerror("invalid hostname"); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1378 "cfg.y"
    {
		/* check if allowed */
		if ((yyvsp[(1) - (1)].action) && rt==ONSEND_ROUTE) {
			switch((yyvsp[(1) - (1)].action)->type) {
				case DROP_T:
				case SEND_T:
				case SEND_TCP_T:
				case LOG_T:
				case SETFLAG_T:
				case RESETFLAG_T:
				case ISFLAGSET_T:
				case IF_T:
				case MODULE_T:
					(yyval.action)=(yyvsp[(1) - (1)].action);
					break;
				default:
					(yyval.action)=0;
					yyerror("command not allowed in onsend_route\n");
			}
		} else {
			(yyval.action)=(yyvsp[(1) - (1)].action);
		}
	;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1403 "cfg.y"
    { (yyval.action)=(yyvsp[(1) - (1)].action); ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1404 "cfg.y"
    { (yyval.action)=(yyvsp[(1) - (1)].action); ;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1405 "cfg.y"
    { (yyval.action) = (yyvsp[(1) - (1)].action); ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1406 "cfg.y"
    { (yyval.action)=(yyvsp[(2) - (3)].action); ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1409 "cfg.y"
    { (yyval.action)=(yyvsp[(1) - (1)].action); ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1410 "cfg.y"
    { (yyval.action)=(yyvsp[(2) - (3)].action); ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1413 "cfg.y"
    {(yyval.action)=append_action((yyvsp[(1) - (2)].action), (yyvsp[(2) - (2)].action)); ;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1414 "cfg.y"
    {(yyval.action)=(yyvsp[(1) - (1)].action);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1415 "cfg.y"
    { (yyval.action)=0; yyerror("bad command"); ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1418 "cfg.y"
    {(yyval.action)=(yyvsp[(1) - (2)].action);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1419 "cfg.y"
    {(yyval.action)=(yyvsp[(1) - (1)].action);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1420 "cfg.y"
    {(yyval.action)=(yyvsp[(1) - (2)].action);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1421 "cfg.y"
    {(yyval.action)=0;;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1422 "cfg.y"
    { (yyval.action)=0; yyerror("bad command: missing ';'?"); ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1425 "cfg.y"
    { (yyval.action)=mk_action( IF_T, 3, EXPR_ST, (yyvsp[(2) - (3)].expr), ACTIONS_ST, (yyvsp[(3) - (3)].action), NOSUBTYPE, 0); ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1426 "cfg.y"
    { (yyval.action)=mk_action( IF_T, 3, EXPR_ST, (yyvsp[(2) - (5)].expr), ACTIONS_ST, (yyvsp[(3) - (5)].action), ACTIONS_ST, (yyvsp[(5) - (5)].action)); ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1435 "cfg.y"
    {
		if (sel.n >= MAX_SELECT_PARAMS-1) {
			yyerror("Select identifier too long\n");
		}
		sel.params[sel.n].type = SEL_PARAM_STR;
		sel.params[sel.n].v.s.s = (yyvsp[(1) - (1)].strval);
		sel.params[sel.n].v.s.len = strlen((yyvsp[(1) - (1)].strval));
		sel.n++;
	;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1444 "cfg.y"
    {
		if (sel.n >= MAX_SELECT_PARAMS-2) {
			yyerror("Select identifier too long\n");
		}
		sel.params[sel.n].type = SEL_PARAM_STR;
		sel.params[sel.n].v.s.s = (yyvsp[(1) - (4)].strval);
		sel.params[sel.n].v.s.len = strlen((yyvsp[(1) - (4)].strval));
		sel.n++;
		sel.params[sel.n].type = SEL_PARAM_INT;
		sel.params[sel.n].v.i = (yyvsp[(3) - (4)].intval);
		sel.n++;
	;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1456 "cfg.y"
    {
		if (sel.n >= MAX_SELECT_PARAMS-2) {
			yyerror("Select identifier too long\n");
		}
		sel.params[sel.n].type = SEL_PARAM_STR;
		sel.params[sel.n].v.s.s = (yyvsp[(1) - (4)].strval);
		sel.params[sel.n].v.s.len = strlen((yyvsp[(1) - (4)].strval));
		sel.n++;
		sel.params[sel.n].type = SEL_PARAM_STR;
		sel.params[sel.n].v.s.s = (yyvsp[(3) - (4)].strval);
		sel.params[sel.n].v.s.len = strlen((yyvsp[(3) - (4)].strval));
		sel.n++;
	;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1475 "cfg.y"
    { sel.n = 0; sel.f[0] = 0; ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1475 "cfg.y"
    {
		sel_ptr = (select_t*)pkg_malloc(sizeof(select_t));
		if (!sel_ptr) {
			yyerror("No memory left to allocate select structure\n");
		}
		memcpy(sel_ptr, &sel, sizeof(select_t));
		(yyval.select) = sel_ptr;
	;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1485 "cfg.y"
    { s_attr->type |= AVP_TRACK_FROM; ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1486 "cfg.y"
    { s_attr->type |= AVP_TRACK_TO; ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1487 "cfg.y"
    { s_attr->type |= AVP_TRACK_FROM | AVP_CLASS_URI; ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1488 "cfg.y"
    { s_attr->type |= AVP_TRACK_TO | AVP_CLASS_URI; ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1489 "cfg.y"
    { s_attr->type |= AVP_TRACK_FROM | AVP_CLASS_USER; ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1490 "cfg.y"
    { s_attr->type |= AVP_TRACK_TO | AVP_CLASS_USER; ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1491 "cfg.y"
    { s_attr->type |= AVP_TRACK_FROM | AVP_CLASS_DOMAIN; ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1492 "cfg.y"
    { s_attr->type |= AVP_TRACK_TO | AVP_CLASS_DOMAIN; ;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1493 "cfg.y"
    { s_attr->type |= AVP_TRACK_ALL | AVP_CLASS_GLOBAL; ;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1496 "cfg.y"
    { s_attr->type |= AVP_NAME_STR; s_attr->name.s.s = (yyvsp[(1) - (1)].strval); s_attr->name.s.len = strlen ((yyvsp[(1) - (1)].strval)); ;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1503 "cfg.y"
    {
		s_attr = (struct avp_spec*)pkg_malloc(sizeof(struct avp_spec));
		if (!s_attr) { yyerror("No memory left"); }
		s_attr->type = 0;
	;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1510 "cfg.y"
    { (yyval.attr) = s_attr; ;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1513 "cfg.y"
    {
		s_attr->type|= (AVP_NAME_STR | ((yyvsp[(4) - (5)].intval)<0?AVP_INDEX_BACKWARD:AVP_INDEX_FORWARD));
		s_attr->index = ((yyvsp[(4) - (5)].intval)<0?-(yyvsp[(4) - (5)].intval):(yyvsp[(4) - (5)].intval));
		(yyval.attr) = s_attr;
	;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1520 "cfg.y"
    {
		s_attr->type|= AVP_INDEX_ALL;
		(yyval.attr) = s_attr;
	;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1540 "cfg.y"
    {
		avp_spec_t *avp_spec;
		str s;
		int type, idx;
		avp_spec = pkg_malloc(sizeof(*avp_spec));
		if (!avp_spec) {
			yyerror("Not enough memory");
			YYABORT;
		}
		s.s = (yyvsp[(1) - (1)].strval);
		if (s.s[0] == '$')
			s.s++;
		s.len = strlen(s.s);
		if (parse_avp_name(&s, &type, &avp_spec->name, &idx)) {
			yyerror("error when parsing AVP");
		        pkg_free(avp_spec);
			YYABORT;
		}
		avp_spec->type = type;
		avp_spec->index = idx;
		(yyval.attr) = avp_spec;
	;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1570 "cfg.y"
    { (yyval.intval) = ASSIGN_T; ;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1573 "cfg.y"
    { (yyval.action)=mk_action((yyvsp[(2) - (3)].intval), 2, AVP_ST, (yyvsp[(1) - (3)].attr), STRING_ST, (yyvsp[(3) - (3)].strval)); ;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1574 "cfg.y"
    { (yyval.action)=mk_action((yyvsp[(2) - (3)].intval), 2, AVP_ST, (yyvsp[(1) - (3)].attr), NUMBER_ST, (void*)(yyvsp[(3) - (3)].intval)); ;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1575 "cfg.y"
    { (yyval.action)=mk_action((yyvsp[(2) - (3)].intval), 2, AVP_ST, (yyvsp[(1) - (3)].attr), ACTION_ST, (yyvsp[(3) - (3)].action)); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1576 "cfg.y"
    { (yyval.action)=mk_action((yyvsp[(2) - (3)].intval), 2, AVP_ST, (yyvsp[(1) - (3)].attr), AVP_ST, (yyvsp[(3) - (3)].attr)); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1577 "cfg.y"
    { (yyval.action)=mk_action((yyvsp[(2) - (3)].intval), 2, AVP_ST, (void*)(yyvsp[(1) - (3)].attr), SELECT_ST, (void*)(yyvsp[(3) - (3)].select)); ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1578 "cfg.y"
    { (yyval.action) = mk_action((yyvsp[(2) - (5)].intval), 2, AVP_ST, (yyvsp[(1) - (5)].attr), EXPR_ST, (yyvsp[(4) - (5)].expr)); ;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1581 "cfg.y"
    { (yyval.intval) = 1; ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1582 "cfg.y"
    { (yyval.intval) = 0; ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1583 "cfg.y"
    { (yyval.intval) = -1; ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1586 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1587 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1588 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_T, 2, IP_ST, (void*)(yyvsp[(3) - (4)].ipaddr), NUMBER_ST, 0); ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1589 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1590 "cfg.y"
    {(yyval.action)=mk_action(FORWARD_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1591 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_T, 2, IP_ST, (void*)(yyvsp[(3) - (6)].ipaddr), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1592 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_T, 2, URIHOST_ST, 0, URIPORT_ST, 0); ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1593 "cfg.y"
    {(yyval.action)=mk_action(FORWARD_T, 2, URIHOST_ST, 0, NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1594 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_T, 2, URIHOST_ST, 0, NUMBER_ST, 0); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1595 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1596 "cfg.y"
    { (yyval.action)=0; yyerror("bad forward argument"); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1597 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_UDP_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1598 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_UDP_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1599 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_UDP_T, 2, IP_ST, (void*)(yyvsp[(3) - (4)].ipaddr), NUMBER_ST, 0); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1600 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_UDP_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1601 "cfg.y"
    {(yyval.action)=mk_action(FORWARD_UDP_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1602 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_UDP_T, 2, IP_ST, (void*)(yyvsp[(3) - (6)].ipaddr), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1603 "cfg.y"
    {(yyval.action)=mk_action(FORWARD_UDP_T, 2, URIHOST_ST, 0, URIPORT_ST, 0); ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1604 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_UDP_T, 2, URIHOST_ST, 0, NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1605 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_UDP_T, 2, URIHOST_ST, 0, NUMBER_ST, 0); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1606 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1607 "cfg.y"
    { (yyval.action)=0; yyerror("bad forward_udp argument"); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1608 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_TCP_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1609 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_TCP_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1610 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_TCP_T, 2, IP_ST, (void*)(yyvsp[(3) - (4)].ipaddr), NUMBER_ST, 0); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1611 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_TCP_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1612 "cfg.y"
    {(yyval.action)=mk_action(FORWARD_TCP_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1613 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_TCP_T, 2, IP_ST, (void*)(yyvsp[(3) - (6)].ipaddr), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1614 "cfg.y"
    {(yyval.action)=mk_action(FORWARD_TCP_T, 2, URIHOST_ST, 0, URIPORT_ST, 0); ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1615 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_TCP_T, 2, URIHOST_ST, 0, NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1616 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_TCP_T, 2, URIHOST_ST, 0, NUMBER_ST, 0); ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1617 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1618 "cfg.y"
    { (yyval.action)=0; yyerror("bad forward_tcp argument"); ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1619 "cfg.y"
    {
		#ifdef USE_TLS
			(yyval.action)=mk_action(FORWARD_TLS_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0);
		#else
			(yyval.action)=0;
			yyerror("tls support not compiled in");
		#endif
	;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1627 "cfg.y"
    {
		#ifdef USE_TLS
			(yyval.action)=mk_action(FORWARD_TLS_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0);
		#else
			(yyval.action)=0;
			yyerror("tls support not compiled in");
		#endif
	;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1635 "cfg.y"
    {
		#ifdef USE_TLS
			(yyval.action)=mk_action(FORWARD_TLS_T, 2, IP_ST, (void*)(yyvsp[(3) - (4)].ipaddr), NUMBER_ST, 0);
		#else
			(yyval.action)=0;
			yyerror("tls support not compiled in");
		#endif
	;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1643 "cfg.y"
    {
		#ifdef USE_TLS
			(yyval.action)=mk_action(FORWARD_TLS_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval));
		#else
			(yyval.action)=0;
			yyerror("tls support not compiled in");
		#endif
	;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1651 "cfg.y"
    {
		#ifdef USE_TLS
			(yyval.action)=mk_action(FORWARD_TLS_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval));
		#else
			(yyval.action)=0;
			yyerror("tls support not compiled in");
		#endif
	;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1659 "cfg.y"
    {
		#ifdef USE_TLS
			(yyval.action)=mk_action(FORWARD_TLS_T, 2, IP_ST, (void*)(yyvsp[(3) - (6)].ipaddr), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval));
		#else
			(yyval.action)=0;
			yyerror("tls support not compiled in");
		#endif
					;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1667 "cfg.y"
    {
		#ifdef USE_TLS
			(yyval.action)=mk_action(FORWARD_TLS_T, 2, URIHOST_ST, 0, URIPORT_ST, 0);
		#else
			(yyval.action)=0;
			yyerror("tls support not compiled in");
		#endif
	;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1675 "cfg.y"
    {
		#ifdef USE_TLS
			(yyval.action)=mk_action(FORWARD_TLS_T, 2, URIHOST_ST, 0, NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval));
		#else
			(yyval.action)=0;
			yyerror("tls support not compiled in");
		#endif
	;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1683 "cfg.y"
    {
		#ifdef USE_TLS
			(yyval.action)=mk_action(FORWARD_TLS_T, 2, URIHOST_ST, 0, NUMBER_ST, 0);
		#else
			(yyval.action)=0;
			yyerror("tls support not compiled in");
		#endif
	;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1691 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1692 "cfg.y"
    { (yyval.action)=0; yyerror("bad forward_tls argument"); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1693 "cfg.y"
    { (yyval.action)=mk_action(SEND_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1694 "cfg.y"
    { (yyval.action)=mk_action(SEND_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1695 "cfg.y"
    { (yyval.action)=mk_action(SEND_T, 2, IP_ST, (void*)(yyvsp[(3) - (4)].ipaddr), NUMBER_ST, 0); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1696 "cfg.y"
    { (yyval.action)=mk_action(SEND_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1697 "cfg.y"
    {(yyval.action)=mk_action(SEND_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1698 "cfg.y"
    { (yyval.action)=mk_action(SEND_T, 2, IP_ST, (void*)(yyvsp[(3) - (6)].ipaddr), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1699 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1700 "cfg.y"
    { (yyval.action)=0; yyerror("bad send argument"); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1701 "cfg.y"
    { (yyval.action)=mk_action(SEND_TCP_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1702 "cfg.y"
    { (yyval.action)=mk_action(SEND_TCP_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, 0); ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1703 "cfg.y"
    { (yyval.action)=mk_action(SEND_TCP_T, 2, IP_ST, (void*)(yyvsp[(3) - (4)].ipaddr), NUMBER_ST, 0); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1704 "cfg.y"
    { (yyval.action)=mk_action(	SEND_TCP_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval));;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1705 "cfg.y"
    {(yyval.action)=mk_action(SEND_TCP_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1706 "cfg.y"
    { (yyval.action)=mk_action(SEND_TCP_T, 2, IP_ST, (void*)(yyvsp[(3) - (6)].ipaddr), NUMBER_ST, (void*)(yyvsp[(5) - (6)].intval)); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1707 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1708 "cfg.y"
    { (yyval.action)=0; yyerror("bad send_tcp argument"); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1709 "cfg.y"
    {(yyval.action)=mk_action(DROP_T, 2, NUMBER_ST, 0, NUMBER_ST, (void*)EXIT_R_F); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1710 "cfg.y"
    {(yyval.action)=mk_action(DROP_T, 2, NUMBER_ST, (void*)(yyvsp[(3) - (4)].intval), NUMBER_ST, (void*)EXIT_R_F); ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1711 "cfg.y"
    {(yyval.action)=mk_action(DROP_T, 2, NUMBER_ST, (void*)(yyvsp[(2) - (2)].intval), NUMBER_ST, (void*)EXIT_R_F); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1712 "cfg.y"
    {(yyval.action)=mk_action(DROP_T, 2, RETCODE_ST, 0, NUMBER_ST, (void*)EXIT_R_F); ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1713 "cfg.y"
    {(yyval.action)=mk_action(DROP_T, 2, NUMBER_ST, 0, NUMBER_ST, (void*)EXIT_R_F); ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1714 "cfg.y"
    {(yyval.action)=mk_action(DROP_T, 2, NUMBER_ST, (void*)1, NUMBER_ST, (void*)RETURN_R_F); ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1715 "cfg.y"
    {(yyval.action)=mk_action(DROP_T, 2, NUMBER_ST, (void*)(yyvsp[(2) - (2)].intval), NUMBER_ST, (void*)RETURN_R_F);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1716 "cfg.y"
    {(yyval.action)=mk_action(DROP_T, 2, RETCODE_ST, 0, NUMBER_ST, (void*)RETURN_R_F);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1717 "cfg.y"
    {(yyval.action)=mk_action(DROP_T, 2, NUMBER_ST, 0, NUMBER_ST, (void*)RETURN_R_F); ;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1718 "cfg.y"
    {(yyval.action)=mk_action(LOG_T, 2, NUMBER_ST, (void*)4, STRING_ST, (yyvsp[(3) - (4)].strval)); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1719 "cfg.y"
    {(yyval.action)=mk_action(LOG_T, 2, NUMBER_ST, (void*)(yyvsp[(3) - (6)].intval), STRING_ST, (yyvsp[(5) - (6)].strval)); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1720 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1721 "cfg.y"
    { (yyval.action)=0; yyerror("bad log argument"); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1722 "cfg.y"
    {
							if (check_flag((yyvsp[(3) - (4)].intval))==-1)
								yyerror("bad flag value");
							(yyval.action)=mk_action(SETFLAG_T, 1, NUMBER_ST,
													(void*)(yyvsp[(3) - (4)].intval));
									;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1728 "cfg.y"
    {
							i_tmp=get_flag_no((yyvsp[(3) - (4)].strval), strlen((yyvsp[(3) - (4)].strval)));
							if (i_tmp<0) yyerror("flag not declared");
							(yyval.action)=mk_action(SETFLAG_T, 1, NUMBER_ST,
										(void*)(long)i_tmp);
									;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1734 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1735 "cfg.y"
    {
							if (check_flag((yyvsp[(3) - (4)].intval))==-1)
								yyerror("bad flag value");
							(yyval.action)=mk_action(RESETFLAG_T, 1, NUMBER_ST, (void*)(yyvsp[(3) - (4)].intval));
									;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1740 "cfg.y"
    {
							i_tmp=get_flag_no((yyvsp[(3) - (4)].strval), strlen((yyvsp[(3) - (4)].strval)));
							if (i_tmp<0) yyerror("flag not declared");
							(yyval.action)=mk_action(RESETFLAG_T, 1, NUMBER_ST,
										(void*)(long)i_tmp);
									;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1746 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1747 "cfg.y"
    {
							if (check_flag((yyvsp[(3) - (4)].intval))==-1)
								yyerror("bad flag value");
							(yyval.action)=mk_action(ISFLAGSET_T, 1, NUMBER_ST, (void*)(yyvsp[(3) - (4)].intval));
									;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1752 "cfg.y"
    {
							i_tmp=get_flag_no((yyvsp[(3) - (4)].strval), strlen((yyvsp[(3) - (4)].strval)));
							if (i_tmp<0) yyerror("flag not declared");
							(yyval.action)=mk_action(ISFLAGSET_T, 1, NUMBER_ST,
										(void*)(long)i_tmp);
									;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1758 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1759 "cfg.y"
    {
		i_tmp=get_avpflag_no((yyvsp[(5) - (6)].strval));
		if (i_tmp==0) yyerror("avpflag not declared");
		(yyval.action)=mk_action(AVPFLAG_OPER_T, 3, AVP_ST, (yyvsp[(3) - (6)].attr), NUMBER_ST, (void*)(long)i_tmp, NUMBER_ST, (void*)(yyvsp[(1) - (6)].intval));
	;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1764 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1765 "cfg.y"
    {(yyval.action)=mk_action(ERROR_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), STRING_ST, (yyvsp[(5) - (6)].strval)); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1766 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1767 "cfg.y"
    { (yyval.action)=0; yyerror("bad error argument"); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1768 "cfg.y"
    {
						i_tmp=route_get(&main_rt, (yyvsp[(3) - (4)].strval));
						if (i_tmp==-1){
							yyerror("internal error");
							YYABORT;
						}
						(yyval.action)=mk_action(ROUTE_T, 1, NUMBER_ST,(void*)(long)i_tmp);
										;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1776 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1777 "cfg.y"
    { (yyval.action)=0; yyerror("bad route argument"); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1778 "cfg.y"
    { (yyval.action)=mk_action(EXEC_T, 1, STRING_ST, (yyvsp[(3) - (4)].strval)); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1779 "cfg.y"
    { (yyval.action)=mk_action(SET_HOST_T, 1, STRING_ST, (yyvsp[(3) - (4)].strval)); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1780 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1781 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, string expected"); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1782 "cfg.y"
    { (yyval.action)=mk_action(PREFIX_T, 1, STRING_ST,  (yyvsp[(3) - (4)].strval)); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1783 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1784 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, string expected"); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1785 "cfg.y"
    { (yyval.action)=mk_action(STRIP_TAIL_T, 1, NUMBER_ST, (void*)(yyvsp[(3) - (4)].intval)); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1786 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1787 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, number expected"); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1788 "cfg.y"
    { (yyval.action)=mk_action(STRIP_T, 1, NUMBER_ST, (void*) (yyvsp[(3) - (4)].intval)); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1789 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1790 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, number expected"); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1791 "cfg.y"
    {
		qvalue_t q;
		if (str2q(&q, (yyvsp[(5) - (6)].strval), strlen((yyvsp[(5) - (6)].strval))) < 0) {
			yyerror("bad argument, q value expected");
		}
		(yyval.action)=mk_action(APPEND_BRANCH_T, 2, STRING_ST, (yyvsp[(3) - (6)].strval), NUMBER_ST, (void *)(long)q);
	;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1798 "cfg.y"
    { (yyval.action)=mk_action(APPEND_BRANCH_T, 2, STRING_ST, (yyvsp[(3) - (4)].strval), NUMBER_ST, (void *)Q_UNSPECIFIED); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1799 "cfg.y"
    { (yyval.action)=mk_action(APPEND_BRANCH_T, 2, STRING_ST, 0, NUMBER_ST, (void *)Q_UNSPECIFIED); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1800 "cfg.y"
    {  (yyval.action)=mk_action( APPEND_BRANCH_T, 1, STRING_ST, 0); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1801 "cfg.y"
    { (yyval.action)=mk_action(SET_HOSTPORT_T, 1, STRING_ST, (yyvsp[(3) - (4)].strval)); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1802 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1803 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, string expected"); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1804 "cfg.y"
    { (yyval.action)=mk_action(SET_PORT_T, 1, STRING_ST, (yyvsp[(3) - (4)].strval)); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1805 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 1806 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, string expected"); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 1807 "cfg.y"
    { (yyval.action)=mk_action(SET_USER_T, 1, STRING_ST, (yyvsp[(3) - (4)].strval)); ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 1808 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 1809 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, string expected"); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 1810 "cfg.y"
    { (yyval.action)=mk_action(SET_USERPASS_T, 1, STRING_ST, (yyvsp[(3) - (4)].strval)); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 1811 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 1812 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, string expected"); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 1813 "cfg.y"
    { (yyval.action)=mk_action(SET_URI_T, 1, STRING_ST,(yyvsp[(3) - (4)].strval)); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 1814 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 1815 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, string expected"); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 1816 "cfg.y"
    { (yyval.action)=mk_action(REVERT_URI_T, 0); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 1817 "cfg.y"
    { (yyval.action)=mk_action(REVERT_URI_T, 0); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 1818 "cfg.y"
    { (yyval.action)=mk_action(FORCE_RPORT_T, 0); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 1819 "cfg.y"
    {(yyval.action)=mk_action(FORCE_RPORT_T, 0); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 1820 "cfg.y"
    {
		#ifdef USE_TCP
			(yyval.action)=mk_action(FORCE_TCP_ALIAS_T, 1, NUMBER_ST, (void*)(yyvsp[(3) - (4)].intval));
		#else
			yyerror("tcp support not compiled in");
		#endif
	;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 1827 "cfg.y"
    {
		#ifdef USE_TCP
			(yyval.action)=mk_action(FORCE_TCP_ALIAS_T, 0);
		#else
			yyerror("tcp support not compiled in");
		#endif
	;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 1834 "cfg.y"
    {
		#ifdef USE_TCP
			(yyval.action)=mk_action(FORCE_TCP_ALIAS_T, 0);
		#else
			yyerror("tcp support not compiled in");
		#endif
	;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 1841 "cfg.y"
    {(yyval.action)=0; yyerror("bad argument, number expected"); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 1842 "cfg.y"
    {
		(yyval.action)=0;
		if ((str_tmp=pkg_malloc(sizeof(str)))==0) {
			LOG(L_CRIT, "ERROR: cfg. parser: out of memory.\n");
		} else {
			str_tmp->s=(yyvsp[(3) - (4)].strval);
			str_tmp->len=strlen((yyvsp[(3) - (4)].strval));
			(yyval.action)=mk_action(SET_ADV_ADDR_T, 1, STR_ST, str_tmp);
		}
	;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 1852 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, string expected"); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 1853 "cfg.y"
    {(yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 1854 "cfg.y"
    {
		(yyval.action)=0;
		tmp=int2str((yyvsp[(3) - (4)].intval), &i_tmp);
		if ((str_tmp=pkg_malloc(sizeof(str)))==0) {
			LOG(L_CRIT, "ERROR: cfg. parser: out of memory.\n");
		} else {
			if ((str_tmp->s=pkg_malloc(i_tmp))==0) {
				LOG(L_CRIT, "ERROR: cfg. parser: out of memory.\n");
			} else {
				memcpy(str_tmp->s, tmp, i_tmp);
				str_tmp->len=i_tmp;
				(yyval.action)=mk_action(SET_ADV_PORT_T, 1, STR_ST, str_tmp);
			}
		}
	;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 1869 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, string expected"); ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 1870 "cfg.y"
    {(yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 1871 "cfg.y"
    { (yyval.action)=mk_action(FORCE_SEND_SOCKET_T, 1, SOCKID_ST, (yyvsp[(3) - (4)].sockid)); ;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 1872 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, [proto:]host[:port] expected"); ;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 1873 "cfg.y"
    {(yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 1874 "cfg.y"
    {mod_func_action = mk_action(MODULE_T, 2, MODEXP_ST, NULL, NUMBER_ST, 0); ;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 1874 "cfg.y"
    {
		mod_func_action->val[0].u.data = find_export_record((yyvsp[(1) - (5)].strval), mod_func_action->val[1].u.number, rt);
		if (mod_func_action->val[0].u.data == 0) {
			if (find_export_record((yyvsp[(1) - (5)].strval), mod_func_action->val[1].u.number, 0) ) {
					yyerror("Command cannot be used in the block\n");
			} else {
				yyerror("unknown command, missing loadmodule?\n");
			}
			pkg_free(mod_func_action);
			mod_func_action=0;
		}
		(yyval.action) = mod_func_action;
	;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 1890 "cfg.y"
    { ;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 1891 "cfg.y"
    {;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 1892 "cfg.y"
    { yyerror("call params error\n"); YYABORT; ;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 1895 "cfg.y"
    {
		if (mod_func_action->val[1].u.number < MAX_ACTIONS-2) {
			mod_func_action->val[mod_func_action->val[1].u.number+2].type = NUMBER_ST;
			mod_func_action->val[mod_func_action->val[1].u.number+2].u.number = (yyvsp[(1) - (1)].intval);
			mod_func_action->val[1].u.number++;
		} else {
			yyerror("Too many arguments\n");
		}
	;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 1904 "cfg.y"
    {
		if (mod_func_action->val[1].u.number < MAX_ACTIONS-2) {
			mod_func_action->val[mod_func_action->val[1].u.number+2].type = STRING_ST;
			mod_func_action->val[mod_func_action->val[1].u.number+2].u.string = (yyvsp[(1) - (1)].strval);
			mod_func_action->val[1].u.number++;
		} else {
			yyerror("Too many arguments\n");
		}
	;}
    break;



/* Line 1455 of yacc.c  */
#line 7349 "cfg.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 1914 "cfg.y"


extern int line;
extern int column;
extern int startcolumn;
static void warn(char* s)
{
	LOG(L_WARN, "cfg. warning: (%d,%d-%d): %s\n", line, startcolumn,
			column, s);
	cfg_warnings++;
}

static void yyerror(char* s)
{
	LOG(L_CRIT, "parse error (%d,%d-%d): %s\n", line, startcolumn,
			column, s);
	cfg_errors++;
}


static struct socket_id* mk_listen_id(char* host, int proto, int port)
{
	struct socket_id* l;
	l=pkg_malloc(sizeof(struct socket_id));
	if (l==0) {
		LOG(L_CRIT,"ERROR: cfg. parser: out of memory.\n");
	} else {
		l->name=host;
		l->port=port;
		l->proto=proto;
		l->next=0;
	}
	return l;
}


/*
int main(int argc, char ** argv)
{
	if (yyparse()!=0)
		fprintf(stderr, "parsing error\n");
}
*/

