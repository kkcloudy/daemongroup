/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     KW_SOURCE = 258,
     KW_DESTINATION = 259,
     KW_LOG = 260,
     KW_OPTIONS = 261,
     KW_FILTER = 262,
     KW_INTERNAL = 263,
     KW_FILE = 264,
     KW_PIPE = 265,
     KW_UNIX_STREAM = 266,
     KW_UNIX_DGRAM = 267,
     KW_TCP = 268,
     KW_UDP = 269,
     KW_TCP6 = 270,
     KW_UDP6 = 271,
     KW_USER = 272,
     KW_DOOR = 273,
     KW_SUN_STREAMS = 274,
     KW_PROGRAM = 275,
     KW_FSYNC = 276,
     KW_MARK_FREQ = 277,
     KW_STATS_FREQ = 278,
     KW_FLUSH_LINES = 279,
     KW_FLUSH_TIMEOUT = 280,
     KW_LOG_MSG_SIZE = 281,
     KW_FILE_TEMPLATE = 282,
     KW_PROTO_TEMPLATE = 283,
     KW_CHAIN_HOSTNAMES = 284,
     KW_NORMALIZE_HOSTNAMES = 285,
     KW_KEEP_HOSTNAME = 286,
     KW_CHECK_HOSTNAME = 287,
     KW_BAD_HOSTNAME = 288,
     KW_KEEP_TIMESTAMP = 289,
     KW_USE_DNS = 290,
     KW_USE_FQDN = 291,
     KW_DNS_CACHE = 292,
     KW_DNS_CACHE_SIZE = 293,
     KW_DNS_CACHE_EXPIRE = 294,
     KW_DNS_CACHE_EXPIRE_FAILED = 295,
     KW_DNS_CACHE_HOSTS = 296,
     KW_PERSIST_ONLY = 297,
     KW_TZ_CONVERT = 298,
     KW_TS_FORMAT = 299,
     KW_FRAC_DIGITS = 300,
     KW_LOG_FIFO_SIZE = 301,
     KW_LOG_FETCH_LIMIT = 302,
     KW_LOG_IW_SIZE = 303,
     KW_LOG_PREFIX = 304,
     KW_FLAGS = 305,
     KW_CATCHALL = 306,
     KW_FALLBACK = 307,
     KW_FINAL = 308,
     KW_FLOW_CONTROL = 309,
     KW_PAD_SIZE = 310,
     KW_TIME_ZONE = 311,
     KW_RECV_TIME_ZONE = 312,
     KW_SEND_TIME_ZONE = 313,
     KW_TIME_REOPEN = 314,
     KW_TIME_REAP = 315,
     KW_TIME_SLEEP = 316,
     KW_TMPL_ESCAPE = 317,
     KW_OPTIONAL = 318,
     KW_CREATE_DIRS = 319,
     KW_OWNER = 320,
     KW_GROUP = 321,
     KW_PERM = 322,
     KW_DIR_OWNER = 323,
     KW_DIR_GROUP = 324,
     KW_DIR_PERM = 325,
     KW_TEMPLATE = 326,
     KW_TEMPLATE_ESCAPE = 327,
     KW_FOLLOW_FREQ = 328,
     KW_REMOVE_IF_OLDER = 329,
     KW_KEEP_ALIVE = 330,
     KW_MAX_CONNECTIONS = 331,
     KW_LOCALIP = 332,
     KW_IP = 333,
     KW_LOCALPORT = 334,
     KW_PORT = 335,
     KW_DESTPORT = 336,
     KW_IP_TTL = 337,
     KW_SO_BROADCAST = 338,
     KW_IP_TOS = 339,
     KW_SO_SNDBUF = 340,
     KW_SO_RCVBUF = 341,
     KW_USE_TIME_RECVD = 342,
     KW_FACILITY = 343,
     KW_LEVEL = 344,
     KW_HOST = 345,
     KW_MATCH = 346,
     KW_NETMASK = 347,
     KW_YES = 348,
     KW_NO = 349,
     KW_REQUIRED = 350,
     KW_ALLOW = 351,
     KW_DENY = 352,
     KW_GC_IDLE_THRESHOLD = 353,
     KW_GC_BUSY_THRESHOLD = 354,
     KW_COMPRESS = 355,
     KW_MAC = 356,
     KW_AUTH = 357,
     KW_ENCRYPT = 358,
     DOTDOT = 359,
     IDENTIFIER = 360,
     NUMBER = 361,
     STRING = 362,
     KW_OR = 363,
     KW_AND = 364,
     KW_NOT = 365
   };
#endif
#define KW_SOURCE 258
#define KW_DESTINATION 259
#define KW_LOG 260
#define KW_OPTIONS 261
#define KW_FILTER 262
#define KW_INTERNAL 263
#define KW_FILE 264
#define KW_PIPE 265
#define KW_UNIX_STREAM 266
#define KW_UNIX_DGRAM 267
#define KW_TCP 268
#define KW_UDP 269
#define KW_TCP6 270
#define KW_UDP6 271
#define KW_USER 272
#define KW_DOOR 273
#define KW_SUN_STREAMS 274
#define KW_PROGRAM 275
#define KW_FSYNC 276
#define KW_MARK_FREQ 277
#define KW_STATS_FREQ 278
#define KW_FLUSH_LINES 279
#define KW_FLUSH_TIMEOUT 280
#define KW_LOG_MSG_SIZE 281
#define KW_FILE_TEMPLATE 282
#define KW_PROTO_TEMPLATE 283
#define KW_CHAIN_HOSTNAMES 284
#define KW_NORMALIZE_HOSTNAMES 285
#define KW_KEEP_HOSTNAME 286
#define KW_CHECK_HOSTNAME 287
#define KW_BAD_HOSTNAME 288
#define KW_KEEP_TIMESTAMP 289
#define KW_USE_DNS 290
#define KW_USE_FQDN 291
#define KW_DNS_CACHE 292
#define KW_DNS_CACHE_SIZE 293
#define KW_DNS_CACHE_EXPIRE 294
#define KW_DNS_CACHE_EXPIRE_FAILED 295
#define KW_DNS_CACHE_HOSTS 296
#define KW_PERSIST_ONLY 297
#define KW_TZ_CONVERT 298
#define KW_TS_FORMAT 299
#define KW_FRAC_DIGITS 300
#define KW_LOG_FIFO_SIZE 301
#define KW_LOG_FETCH_LIMIT 302
#define KW_LOG_IW_SIZE 303
#define KW_LOG_PREFIX 304
#define KW_FLAGS 305
#define KW_CATCHALL 306
#define KW_FALLBACK 307
#define KW_FINAL 308
#define KW_FLOW_CONTROL 309
#define KW_PAD_SIZE 310
#define KW_TIME_ZONE 311
#define KW_RECV_TIME_ZONE 312
#define KW_SEND_TIME_ZONE 313
#define KW_TIME_REOPEN 314
#define KW_TIME_REAP 315
#define KW_TIME_SLEEP 316
#define KW_TMPL_ESCAPE 317
#define KW_OPTIONAL 318
#define KW_CREATE_DIRS 319
#define KW_OWNER 320
#define KW_GROUP 321
#define KW_PERM 322
#define KW_DIR_OWNER 323
#define KW_DIR_GROUP 324
#define KW_DIR_PERM 325
#define KW_TEMPLATE 326
#define KW_TEMPLATE_ESCAPE 327
#define KW_FOLLOW_FREQ 328
#define KW_REMOVE_IF_OLDER 329
#define KW_KEEP_ALIVE 330
#define KW_MAX_CONNECTIONS 331
#define KW_LOCALIP 332
#define KW_IP 333
#define KW_LOCALPORT 334
#define KW_PORT 335
#define KW_DESTPORT 336
#define KW_IP_TTL 337
#define KW_SO_BROADCAST 338
#define KW_IP_TOS 339
#define KW_SO_SNDBUF 340
#define KW_SO_RCVBUF 341
#define KW_USE_TIME_RECVD 342
#define KW_FACILITY 343
#define KW_LEVEL 344
#define KW_HOST 345
#define KW_MATCH 346
#define KW_NETMASK 347
#define KW_YES 348
#define KW_NO 349
#define KW_REQUIRED 350
#define KW_ALLOW 351
#define KW_DENY 352
#define KW_GC_IDLE_THRESHOLD 353
#define KW_GC_BUSY_THRESHOLD 354
#define KW_COMPRESS 355
#define KW_MAC 356
#define KW_AUTH 357
#define KW_ENCRYPT 358
#define DOTDOT 359
#define IDENTIFIER 360
#define NUMBER 361
#define STRING 362
#define KW_OR 363
#define KW_AND 364
#define KW_NOT 365




/* Copy the first part of user declarations.  */
#line 1 "cfg-grammar.y"


#include "syslog-ng.h"
#include "cfg.h"
#include "sgroup.h"
#include "dgroup.h"
#include "center.h"
#include "filter.h"
#include "templates.h"
#include "logreader.h"

#include "affile.h"
#include "afinter.h"
#include "afsocket.h"
#include "afinet.h"
#include "afunix.h"
#include "afstreams.h"
#include "afuser.h"
#include "afprog.h"

#include "messages.h"

#include "syslog-names.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

int lookup_parse_flag(char *flag);

void yyerror(char *msg);
int yylex();

LogDriver *last_driver;
LogReaderOptions *last_reader_options;
LogWriterOptions *last_writer_options;
LogTemplate *last_template;
SocketOptions *last_sock_options;
gint last_addr_family = AF_INET;



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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 43 "cfg-grammar.y"
typedef union YYSTYPE {
	guint num;
	char *cptr;
	void *ptr;
	FilterExprNode *node;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 345 "cfg-grammar.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 357 "cfg-grammar.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  24
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   749

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  109
/* YYNRULES -- Number of rules. */
#define YYNRULES  277
/* YYNRULES -- Number of states. */
#define YYNSTATES  717

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   365

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     114,   115,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   111,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   112,     2,   113,     2,     2,     2,     2,
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
     105,   106,   107,   108,   109,   110
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     9,    10,    13,    16,    19,    22,
      25,    28,    33,    38,    43,    47,    48,    54,    58,    59,
      64,    69,    74,    79,    84,    86,    91,    96,   100,   101,
     103,   105,   107,   109,   113,   118,   123,   124,   128,   129,
     133,   138,   140,   145,   150,   151,   157,   158,   164,   165,
     171,   172,   178,   179,   183,   184,   188,   191,   192,   197,
     202,   207,   212,   214,   216,   218,   219,   222,   225,   226,
     228,   233,   238,   243,   248,   253,   258,   260,   262,   263,
     266,   269,   270,   272,   277,   282,   284,   289,   294,   299,
     300,   304,   307,   308,   313,   316,   317,   322,   327,   332,
     337,   342,   347,   352,   357,   362,   365,   366,   370,   371,
     373,   375,   377,   379,   381,   386,   387,   391,   394,   395,
     397,   402,   407,   412,   417,   422,   427,   432,   437,   442,
     447,   448,   452,   455,   456,   458,   463,   468,   473,   478,
     483,   484,   490,   491,   497,   498,   504,   505,   511,   512,
     516,   517,   521,   524,   525,   527,   529,   530,   534,   537,
     538,   543,   548,   550,   552,   554,   559,   564,   569,   570,
     574,   577,   578,   580,   585,   590,   595,   600,   605,   606,
     610,   613,   614,   619,   624,   629,   634,   639,   644,   649,
     654,   659,   664,   667,   668,   670,   674,   675,   680,   685,
     690,   696,   697,   700,   701,   703,   705,   707,   709,   713,
     714,   719,   724,   729,   734,   739,   744,   749,   754,   759,
     764,   769,   774,   779,   784,   789,   794,   799,   804,   809,
     814,   819,   824,   829,   834,   839,   844,   849,   854,   859,
     864,   869,   874,   879,   884,   889,   894,   899,   904,   909,
     914,   920,   922,   925,   929,   933,   937,   942,   947,   952,
     957,   962,   967,   972,   977,   980,   982,   984,   987,   989,
     993,   995,   997,   999,  1001,  1003,  1005,  1007
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     117,     0,    -1,   118,    -1,   119,   111,   118,    -1,    -1,
       3,   120,    -1,     4,   121,    -1,     5,   122,    -1,     7,
     215,    -1,    71,   124,    -1,     6,   123,    -1,   224,   112,
     130,   113,    -1,   224,   112,   168,   113,    -1,   112,   208,
     210,   113,    -1,   112,   213,   113,    -1,    -1,   224,   125,
     112,   126,   113,    -1,   127,   111,   126,    -1,    -1,    71,
     114,   224,   115,    -1,    72,   114,   222,   115,    -1,    85,
     114,   106,   115,    -1,    86,   114,   106,   115,    -1,    83,
     114,   222,   115,    -1,   128,    -1,    82,   114,   106,   115,
      -1,    84,   114,   106,   115,    -1,   131,   111,   130,    -1,
      -1,   132,    -1,   133,    -1,   139,    -1,   160,    -1,     8,
     114,   115,    -1,     9,   114,   134,   115,    -1,    10,   114,
     136,   115,    -1,    -1,   224,   135,   165,    -1,    -1,   224,
     137,   138,    -1,    63,   114,   222,   115,    -1,   165,    -1,
      12,   114,   144,   115,    -1,    11,   114,   146,   115,    -1,
      -1,    14,   140,   114,   150,   115,    -1,    -1,    13,   141,
     114,   155,   115,    -1,    -1,    16,   142,   114,   150,   115,
      -1,    -1,    15,   143,   114,   155,   115,    -1,    -1,   224,
     145,   148,    -1,    -1,   224,   147,   148,    -1,   149,   148,
      -1,    -1,    65,   114,   224,   115,    -1,    66,   114,   224,
     115,    -1,    67,   114,   106,   115,    -1,    63,   114,   222,
     115,    -1,   159,    -1,   166,    -1,   128,    -1,    -1,   151,
     152,    -1,   153,   152,    -1,    -1,   154,    -1,    79,   114,
     224,   115,    -1,    80,   114,   224,   115,    -1,    77,   114,
     224,   115,    -1,    79,   114,   106,   115,    -1,    80,   114,
     106,   115,    -1,    78,   114,   224,   115,    -1,   166,    -1,
     129,    -1,    -1,   156,   157,    -1,   158,   157,    -1,    -1,
     154,    -1,    79,   114,   224,   115,    -1,    80,   114,   224,
     115,    -1,   159,    -1,    75,   114,   222,   115,    -1,    76,
     114,   106,   115,    -1,    19,   114,   161,   115,    -1,    -1,
     224,   162,   163,    -1,   164,   163,    -1,    -1,    18,   114,
     224,   115,    -1,   166,   165,    -1,    -1,    50,   114,   167,
     115,    -1,    26,   114,   106,   115,    -1,    48,   114,   106,
     115,    -1,    47,   114,   106,   115,    -1,    49,   114,   224,
     115,    -1,    55,   114,   106,   115,    -1,    73,   114,   106,
     115,    -1,    56,   114,   224,   115,    -1,    34,   114,   222,
     115,    -1,   105,   167,    -1,    -1,   169,   111,   168,    -1,
      -1,   170,    -1,   175,    -1,   180,    -1,   200,    -1,   201,
      -1,     9,   114,   171,   115,    -1,    -1,   224,   172,   173,
      -1,   174,   173,    -1,    -1,   205,    -1,    63,   114,   222,
     115,    -1,    65,   114,   224,   115,    -1,    66,   114,   224,
     115,    -1,    67,   114,   106,   115,    -1,    68,   114,   224,
     115,    -1,    69,   114,   224,   115,    -1,    70,   114,   106,
     115,    -1,    64,   114,   222,   115,    -1,    74,   114,   106,
     115,    -1,    10,   114,   176,   115,    -1,    -1,   224,   177,
     178,    -1,   179,   178,    -1,    -1,   205,    -1,    65,   114,
     224,   115,    -1,    66,   114,   224,   115,    -1,    67,   114,
     106,   115,    -1,    12,   114,   185,   115,    -1,    11,   114,
     187,   115,    -1,    -1,    14,   181,   114,   191,   115,    -1,
      -1,    13,   182,   114,   196,   115,    -1,    -1,    16,   183,
     114,   191,   115,    -1,    -1,    15,   184,   114,   196,   115,
      -1,    -1,   224,   186,   189,    -1,    -1,   224,   188,   189,
      -1,   189,   190,    -1,    -1,   205,    -1,   128,    -1,    -1,
     224,   192,   193,    -1,   193,   195,    -1,    -1,    77,   114,
     224,   115,    -1,    80,   114,   106,   115,    -1,   129,    -1,
     205,    -1,   194,    -1,    79,   114,   224,   115,    -1,    80,
     114,   224,   115,    -1,    81,   114,   224,   115,    -1,    -1,
     224,   197,   198,    -1,   198,   199,    -1,    -1,   194,    -1,
      79,   114,   224,   115,    -1,    80,   114,   224,   115,    -1,
      81,   114,   224,   115,    -1,    17,   114,   224,   115,    -1,
      20,   114,   202,   115,    -1,    -1,   224,   203,   204,    -1,
     205,   204,    -1,    -1,    50,   114,   206,   115,    -1,    46,
     114,   106,   115,    -1,    24,   114,   106,   115,    -1,    25,
     114,   106,   115,    -1,    71,   114,   224,   115,    -1,    72,
     114,   222,   115,    -1,    21,   114,   222,   115,    -1,    56,
     114,   224,   115,    -1,    44,   114,   224,   115,    -1,    45,
     114,   106,   115,    -1,   207,   206,    -1,    -1,    62,    -1,
     209,   111,   208,    -1,    -1,     3,   114,   224,   115,    -1,
       7,   114,   224,   115,    -1,     4,   114,   224,   115,    -1,
      50,   114,   211,   115,   111,    -1,    -1,   212,   211,    -1,
      -1,    51,    -1,    52,    -1,    53,    -1,    54,    -1,   214,
     111,   213,    -1,    -1,    22,   114,   106,   115,    -1,    23,
     114,   106,   115,    -1,    24,   114,   106,   115,    -1,    25,
     114,   106,   115,    -1,    29,   114,   222,   115,    -1,    30,
     114,   222,   115,    -1,    31,   114,   222,   115,    -1,    32,
     114,   222,   115,    -1,    33,   114,   107,   115,    -1,    87,
     114,   222,   115,    -1,    36,   114,   222,   115,    -1,    35,
     114,   223,   115,    -1,    59,   114,   106,   115,    -1,    60,
     114,   106,   115,    -1,    61,   114,   106,   115,    -1,    46,
     114,   106,   115,    -1,    48,   114,   106,   115,    -1,    47,
     114,   106,   115,    -1,    26,   114,   106,   115,    -1,    34,
     114,   222,   115,    -1,    44,   114,   224,   115,    -1,    45,
     114,   106,   115,    -1,    99,   114,   106,   115,    -1,    98,
     114,   106,   115,    -1,    64,   114,   222,   115,    -1,    65,
     114,   224,   115,    -1,    66,   114,   224,   115,    -1,    67,
     114,   106,   115,    -1,    68,   114,   224,   115,    -1,    69,
     114,   224,   115,    -1,    70,   114,   106,   115,    -1,    37,
     114,   222,   115,    -1,    38,   114,   106,   115,    -1,    39,
     114,   106,   115,    -1,    40,   114,   106,   115,    -1,    41,
     114,   224,   115,    -1,    27,   114,   224,   115,    -1,    28,
     114,   224,   115,    -1,    57,   114,   224,   115,    -1,    58,
     114,   224,   115,    -1,   224,   112,   216,   111,   113,    -1,
     217,    -1,   110,   216,    -1,   216,   108,   216,    -1,   216,
     109,   216,    -1,   114,   216,   115,    -1,    88,   114,   218,
     115,    -1,    88,   114,   106,   115,    -1,    89,   114,   220,
     115,    -1,    20,   114,   224,   115,    -1,    90,   114,   224,
     115,    -1,    91,   114,   224,   115,    -1,     7,   114,   224,
     115,    -1,    92,   114,   224,   115,    -1,   219,   218,    -1,
     219,    -1,   105,    -1,   221,   220,    -1,   221,    -1,   105,
     104,   105,    -1,   105,    -1,    93,    -1,    94,    -1,   106,
      -1,   222,    -1,    42,    -1,   105,    -1,   107,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   190,   190,   194,   195,   199,   200,   201,   202,   203,
     204,   208,   212,   216,   220,   225,   224,   233,   234,   238,
     239,   243,   244,   245,   249,   250,   251,   255,   256,   260,
     261,   262,   263,   267,   271,   272,   277,   276,   287,   286,
     296,   297,   301,   302,   303,   303,   304,   304,   305,   305,
     306,   306,   311,   310,   324,   323,   337,   338,   342,   343,
     344,   345,   346,   347,   348,   353,   353,   364,   365,   369,
     370,   371,   375,   376,   377,   378,   379,   380,   385,   385,
     396,   397,   401,   402,   403,   404,   408,   409,   413,   418,
     417,   426,   427,   431,   435,   436,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   452,   453,   458,   459,   463,
     464,   465,   466,   467,   471,   476,   475,   486,   487,   491,
     492,   497,   498,   499,   500,   501,   502,   503,   504,   508,
     513,   512,   523,   524,   528,   529,   530,   531,   536,   537,
     538,   538,   539,   539,   540,   540,   541,   541,   546,   545,
     557,   556,   567,   568,   572,   573,   578,   577,   590,   591,
     596,   597,   598,   599,   603,   604,   605,   606,   611,   610,
     623,   624,   628,   629,   630,   631,   641,   645,   650,   649,
     659,   660,   664,   665,   666,   667,   668,   678,   679,   680,
     681,   682,   686,   687,   691,   696,   697,   701,   702,   703,
     707,   708,   713,   714,   718,   719,   720,   721,   725,   726,
     730,   731,   732,   733,   734,   735,   736,   737,   738,   739,
     740,   741,   742,   743,   744,   753,   754,   755,   756,   757,
     758,   759,   760,   761,   762,   763,   764,   765,   766,   767,
     768,   769,   770,   771,   772,   774,   775,   776,   777,   778,
     782,   786,   787,   788,   789,   790,   794,   795,   796,   797,
     798,   799,   800,   801,   805,   806,   810,   827,   828,   832,
     852,   869,   870,   871,   875,   876,   880,   881
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "KW_SOURCE", "KW_DESTINATION", "KW_LOG",
  "KW_OPTIONS", "KW_FILTER", "KW_INTERNAL", "KW_FILE", "KW_PIPE",
  "KW_UNIX_STREAM", "KW_UNIX_DGRAM", "KW_TCP", "KW_UDP", "KW_TCP6",
  "KW_UDP6", "KW_USER", "KW_DOOR", "KW_SUN_STREAMS", "KW_PROGRAM",
  "KW_FSYNC", "KW_MARK_FREQ", "KW_STATS_FREQ", "KW_FLUSH_LINES",
  "KW_FLUSH_TIMEOUT", "KW_LOG_MSG_SIZE", "KW_FILE_TEMPLATE",
  "KW_PROTO_TEMPLATE", "KW_CHAIN_HOSTNAMES", "KW_NORMALIZE_HOSTNAMES",
  "KW_KEEP_HOSTNAME", "KW_CHECK_HOSTNAME", "KW_BAD_HOSTNAME",
  "KW_KEEP_TIMESTAMP", "KW_USE_DNS", "KW_USE_FQDN", "KW_DNS_CACHE",
  "KW_DNS_CACHE_SIZE", "KW_DNS_CACHE_EXPIRE", "KW_DNS_CACHE_EXPIRE_FAILED",
  "KW_DNS_CACHE_HOSTS", "KW_PERSIST_ONLY", "KW_TZ_CONVERT", "KW_TS_FORMAT",
  "KW_FRAC_DIGITS", "KW_LOG_FIFO_SIZE", "KW_LOG_FETCH_LIMIT",
  "KW_LOG_IW_SIZE", "KW_LOG_PREFIX", "KW_FLAGS", "KW_CATCHALL",
  "KW_FALLBACK", "KW_FINAL", "KW_FLOW_CONTROL", "KW_PAD_SIZE",
  "KW_TIME_ZONE", "KW_RECV_TIME_ZONE", "KW_SEND_TIME_ZONE",
  "KW_TIME_REOPEN", "KW_TIME_REAP", "KW_TIME_SLEEP", "KW_TMPL_ESCAPE",
  "KW_OPTIONAL", "KW_CREATE_DIRS", "KW_OWNER", "KW_GROUP", "KW_PERM",
  "KW_DIR_OWNER", "KW_DIR_GROUP", "KW_DIR_PERM", "KW_TEMPLATE",
  "KW_TEMPLATE_ESCAPE", "KW_FOLLOW_FREQ", "KW_REMOVE_IF_OLDER",
  "KW_KEEP_ALIVE", "KW_MAX_CONNECTIONS", "KW_LOCALIP", "KW_IP",
  "KW_LOCALPORT", "KW_PORT", "KW_DESTPORT", "KW_IP_TTL", "KW_SO_BROADCAST",
  "KW_IP_TOS", "KW_SO_SNDBUF", "KW_SO_RCVBUF", "KW_USE_TIME_RECVD",
  "KW_FACILITY", "KW_LEVEL", "KW_HOST", "KW_MATCH", "KW_NETMASK", "KW_YES",
  "KW_NO", "KW_REQUIRED", "KW_ALLOW", "KW_DENY", "KW_GC_IDLE_THRESHOLD",
  "KW_GC_BUSY_THRESHOLD", "KW_COMPRESS", "KW_MAC", "KW_AUTH", "KW_ENCRYPT",
  "DOTDOT", "IDENTIFIER", "NUMBER", "STRING", "KW_OR", "KW_AND", "KW_NOT",
  "';'", "'{'", "'}'", "'('", "')'", "$accept", "start", "stmts", "stmt",
  "source_stmt", "dest_stmt", "log_stmt", "options_stmt", "template_stmt",
  "@1", "template_items", "template_item", "socket_option",
  "inet_socket_option", "source_items", "source_item", "source_afinter",
  "source_affile", "source_affile_params", "@2", "source_afpipe_params",
  "@3", "source_afpipe_options", "source_afsocket", "@4", "@5", "@6", "@7",
  "source_afunix_dgram_params", "@8", "source_afunix_stream_params", "@9",
  "source_afunix_options", "source_afunix_option",
  "source_afinet_udp_params", "@10", "source_afinet_udp_options",
  "source_afinet_udp_option", "source_afinet_option",
  "source_afinet_tcp_params", "@11", "source_afinet_tcp_options",
  "source_afinet_tcp_option", "source_afsocket_stream_params",
  "source_afstreams", "source_afstreams_params", "@12",
  "source_afstreams_options", "source_afstreams_option",
  "source_reader_options", "source_reader_option",
  "source_reader_option_flags", "dest_items", "dest_item", "dest_affile",
  "dest_affile_params", "@13", "dest_affile_options", "dest_affile_option",
  "dest_afpipe", "dest_afpipe_params", "@14", "dest_afpipe_options",
  "dest_afpipe_option", "dest_afsocket", "@15", "@16", "@17", "@18",
  "dest_afunix_dgram_params", "@19", "dest_afunix_stream_params", "@20",
  "dest_afunix_options", "dest_afunix_option", "dest_afinet_udp_params",
  "@21", "dest_afinet_udp_options", "dest_afinet_option",
  "dest_afinet_udp_option", "dest_afinet_tcp_params", "@22",
  "dest_afinet_tcp_options", "dest_afinet_tcp_option", "dest_afuser",
  "dest_afprogram", "dest_afprogram_params", "@23", "dest_writer_options",
  "dest_writer_option", "dest_writer_options_flags",
  "dest_writer_options_flag", "log_items", "log_item", "log_flags",
  "log_flags_items", "log_flags_item", "options_items", "options_item",
  "filter_stmt", "filter_expr", "filter_simple_expr", "filter_fac_list",
  "filter_fac", "filter_level_list", "filter_level", "yesno", "dnsmode",
  "string", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
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
     365,    59,   123,   125,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   116,   117,   118,   118,   119,   119,   119,   119,   119,
     119,   120,   121,   122,   123,   125,   124,   126,   126,   127,
     127,   128,   128,   128,   129,   129,   129,   130,   130,   131,
     131,   131,   131,   132,   133,   133,   135,   134,   137,   136,
     138,   138,   139,   139,   140,   139,   141,   139,   142,   139,
     143,   139,   145,   144,   147,   146,   148,   148,   149,   149,
     149,   149,   149,   149,   149,   151,   150,   152,   152,   153,
     153,   153,   154,   154,   154,   154,   154,   154,   156,   155,
     157,   157,   158,   158,   158,   158,   159,   159,   160,   162,
     161,   163,   163,   164,   165,   165,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   167,   167,   168,   168,   169,
     169,   169,   169,   169,   170,   172,   171,   173,   173,   174,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   175,
     177,   176,   178,   178,   179,   179,   179,   179,   180,   180,
     181,   180,   182,   180,   183,   180,   184,   180,   186,   185,
     188,   187,   189,   189,   190,   190,   192,   191,   193,   193,
     194,   194,   194,   194,   195,   195,   195,   195,   197,   196,
     198,   198,   199,   199,   199,   199,   200,   201,   203,   202,
     204,   204,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   206,   206,   207,   208,   208,   209,   209,   209,
     210,   210,   211,   211,   212,   212,   212,   212,   213,   213,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     215,   216,   216,   216,   216,   216,   217,   217,   217,   217,
     217,   217,   217,   217,   218,   218,   219,   220,   220,   221,
     221,   222,   222,   222,   223,   223,   224,   224
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     3,     0,     2,     2,     2,     2,     2,
       2,     4,     4,     4,     3,     0,     5,     3,     0,     4,
       4,     4,     4,     4,     1,     4,     4,     3,     0,     1,
       1,     1,     1,     3,     4,     4,     0,     3,     0,     3,
       4,     1,     4,     4,     0,     5,     0,     5,     0,     5,
       0,     5,     0,     3,     0,     3,     2,     0,     4,     4,
       4,     4,     1,     1,     1,     0,     2,     2,     0,     1,
       4,     4,     4,     4,     4,     4,     1,     1,     0,     2,
       2,     0,     1,     4,     4,     1,     4,     4,     4,     0,
       3,     2,     0,     4,     2,     0,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     2,     0,     3,     0,     1,
       1,     1,     1,     1,     4,     0,     3,     2,     0,     1,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       0,     3,     2,     0,     1,     4,     4,     4,     4,     4,
       0,     5,     0,     5,     0,     5,     0,     5,     0,     3,
       0,     3,     2,     0,     1,     1,     0,     3,     2,     0,
       4,     4,     1,     1,     1,     4,     4,     4,     0,     3,
       2,     0,     1,     4,     4,     4,     4,     4,     0,     3,
       2,     0,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     2,     0,     1,     3,     0,     4,     4,     4,
       5,     0,     2,     0,     1,     1,     1,     1,     3,     0,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       5,     1,     2,     3,     3,     3,     4,     4,     4,     4,
       4,     4,     4,     4,     2,     1,     1,     2,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short int yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     2,     0,
     276,   277,     5,     0,     6,     0,   196,     7,   209,    10,
       8,     0,     9,    15,     1,     4,    28,   108,     0,     0,
       0,   201,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     3,     0,     0,
       0,     0,     0,    46,    44,    50,    48,     0,     0,     0,
      29,    30,    31,    32,     0,     0,     0,     0,   142,   140,
     146,   144,     0,     0,     0,     0,   109,   110,   111,   112,
     113,     0,     0,     0,     0,     0,   196,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,   209,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   251,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    28,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,   108,     0,     0,     0,   203,    13,
     195,     0,     0,     0,     0,     0,     0,     0,   271,   272,
     273,     0,     0,     0,     0,     0,     0,   275,   274,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,   252,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,    36,     0,    38,     0,    54,
       0,    52,    78,    65,    78,    65,     0,    89,    27,     0,
     115,     0,   130,     0,   150,     0,   148,     0,     0,     0,
       0,     0,     0,   178,   107,   197,   199,   198,   204,   205,
     206,   207,     0,   203,   210,   211,   212,   213,   228,   246,
     247,   214,   215,   216,   217,   218,   229,   221,   220,   241,
     242,   243,   244,   245,   230,   231,   225,   227,   226,   248,
     249,   222,   223,   224,   234,   235,   236,   237,   238,   239,
     240,   219,   233,   232,     0,     0,   266,     0,     0,   265,
     270,     0,   268,     0,     0,     0,   255,   253,   254,   250,
       0,     0,    16,    18,    34,    95,    35,    95,    43,    57,
      42,    57,     0,    81,     0,    68,     0,     0,    88,    92,
     114,   118,   129,   133,   139,   153,   138,   153,     0,   168,
       0,   156,     0,     0,   176,   177,   181,     0,   202,   262,
     259,   257,   256,   264,     0,   258,   267,   260,   261,   263,
       0,     0,    17,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    37,    95,     0,    39,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    64,    55,    57,    62,
      63,    53,    47,     0,     0,     0,     0,     0,     0,    24,
      77,    82,    79,    81,    85,    76,    45,     0,     0,    66,
      68,    69,    51,    49,     0,    90,    92,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   116,   118,   119,     0,
       0,     0,   131,   133,   134,   151,   149,   143,   171,   141,
     159,   147,   145,   179,   181,   200,   269,    19,    20,     0,
       0,     0,     0,     0,   106,     0,     0,     0,    94,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    56,
       0,     0,     0,     0,     0,     0,    80,     0,     0,    67,
       0,    91,     0,     0,     0,     0,     0,     0,   193,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   117,     0,     0,     0,   132,   155,   152,   154,   169,
     157,   180,     0,     0,     0,     0,     0,   106,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   194,
       0,   193,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   162,   172,   170,   163,     0,     0,     0,   164,   158,
      97,   104,    99,    98,   100,   105,    96,   101,   103,   102,
      40,    61,    58,    59,    60,    86,    87,    23,    21,    22,
      72,    75,    73,    83,    74,    84,    25,    26,    70,    71,
      93,   188,   184,   185,   190,   191,   183,   182,   192,   189,
     120,   127,   121,   122,   123,   124,   125,   126,   186,   187,
     128,   135,   136,   137,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   160,
     173,   161,   174,   175,   165,   166,   167
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     7,     8,     9,    12,    14,    17,    19,    22,    76,
     261,   262,   449,   450,    88,    89,    90,    91,   264,   365,
     266,   367,   425,    92,   177,   176,   179,   178,   270,   371,
     268,   369,   437,   438,   374,   375,   459,   460,   451,   372,
     373,   452,   453,   439,    93,   276,   379,   465,   466,   422,
     455,   578,   104,   105,   106,   279,   381,   486,   487,   107,
     281,   383,   492,   493,   108,   188,   187,   190,   189,   285,
     387,   283,   385,   495,   567,   390,   500,   570,   632,   639,
     388,   498,   569,   633,   109,   110,   292,   396,   503,   488,
     610,   611,    31,    32,   115,   302,   303,    73,    74,    20,
     168,   169,   348,   349,   351,   352,   211,   219,   389
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -491
static const short int yypact[] =
{
      18,   -73,   -73,   -86,   -71,   -73,   -73,    50,  -491,   -40,
    -491,  -491,  -491,   -27,  -491,   -25,    44,  -491,   233,  -491,
    -491,   -22,  -491,  -491,  -491,    18,    46,   195,   -26,   -23,
     -18,    47,     7,     5,     6,    10,    11,    20,    26,    31,
      35,    43,    45,    48,    51,    54,    55,    56,    61,    62,
      63,    64,    66,    67,    75,    79,    80,    81,    82,    85,
      86,    87,   102,   103,   104,   106,   115,   116,   117,   118,
     119,   120,   121,    84,    49,    13,    52,  -491,   122,   123,
     124,   125,   126,  -491,  -491,  -491,  -491,   127,   130,   133,
    -491,  -491,  -491,  -491,   131,   134,   135,   139,  -491,  -491,
    -491,  -491,   140,   161,   163,   171,  -491,  -491,  -491,  -491,
    -491,   -73,   -73,   -73,   169,   172,    44,   178,   180,   189,
     199,   200,   -73,   -73,   -63,   -63,   -63,   -63,   206,   -63,
     -24,   -63,   -63,   208,   209,   210,   -73,   -73,   213,   216,
     217,   220,   -73,   -73,   224,   228,   230,   -63,   -73,   -73,
     231,   -73,   -73,   232,   -63,   234,   235,  -491,   233,   229,
     237,   238,   239,   243,   244,   247,    13,    13,   -45,  -491,
     -35,   227,   -73,   -73,   -73,   -73,   256,   259,   260,   261,
     -73,  -491,    46,   -73,   -73,   -73,   -73,   263,   264,   265,
     266,   -73,   -73,  -491,   195,   267,   269,   272,   101,  -491,
    -491,   273,   274,   275,   277,   278,   279,   281,  -491,  -491,
    -491,   282,   284,   287,   289,   291,   293,  -491,  -491,   294,
     295,   296,   297,   298,   299,   305,   306,   307,   311,   312,
     313,   314,   315,   318,   319,   321,   322,   323,   324,   326,
     335,   345,   346,   347,   348,   351,  -491,   -73,   -73,   -38,
      53,   -73,   -73,   -73,  -491,   -70,    13,    13,   236,   301,
     353,   268,   357,  -491,   354,  -491,   355,  -491,   356,  -491,
     358,  -491,  -491,  -491,  -491,  -491,   362,  -491,  -491,   363,
    -491,   364,  -491,   365,  -491,   366,  -491,   -73,   -73,   -73,
     -73,   368,   371,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,   372,   101,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,   373,   374,  -491,   375,   376,   242,
      57,   378,    53,   381,   382,   383,  -491,   390,  -491,  -491,
     -73,   -63,  -491,   -35,  -491,   262,  -491,    83,  -491,   409,
    -491,   409,   385,   369,   386,   516,   387,   388,  -491,   330,
    -491,   559,  -491,   592,  -491,  -491,  -491,  -491,   389,  -491,
     391,  -491,   392,   393,  -491,  -491,    28,   394,  -491,  -491,
    -491,  -491,  -491,  -491,   271,  -491,  -491,  -491,  -491,  -491,
     395,   396,  -491,   399,   400,   402,   403,   404,   405,   408,
     410,   411,  -491,   262,   412,  -491,  -491,   413,   414,   419,
     420,   421,   424,   426,   427,   429,  -491,  -491,   409,  -491,
    -491,  -491,  -491,   431,   432,   433,   435,   439,   442,  -491,
    -491,  -491,  -491,   369,  -491,  -491,  -491,   444,   445,  -491,
     516,  -491,  -491,  -491,   446,  -491,   330,   453,   454,   455,
     456,   459,   460,   461,   462,   463,   464,   465,   467,   468,
     471,   472,   473,   474,   476,   477,  -491,   559,  -491,   478,
     483,   492,  -491,   592,  -491,   300,   300,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,    28,  -491,  -491,  -491,  -491,   292,
     -63,   417,   501,   -73,   503,   504,   -73,   505,  -491,   -63,
     -63,   -73,   -73,   506,   -63,   508,   -63,   512,   513,  -491,
     -73,   -73,   -30,     1,   514,   515,  -491,   -30,     1,  -491,
     -73,  -491,   -63,   526,   528,   -73,   529,   533,   447,   -73,
     -63,   -63,   -73,   -73,   534,   -73,   -73,   535,   -73,   -63,
     537,  -491,   -73,   -73,   538,  -491,  -491,  -491,  -491,   142,
     283,  -491,   530,   531,   532,   536,   539,   503,   540,   541,
     545,   546,   547,   550,   551,   552,   553,   554,   555,   556,
     557,   558,   560,   561,   562,   563,   564,   565,   566,   567,
     568,   569,   570,   571,   572,   573,   574,   575,   576,  -491,
     577,   447,   585,   586,   587,   588,   589,   590,   591,   593,
     594,   595,   596,   597,   598,   599,   600,   602,   603,   604,
     605,  -491,  -491,  -491,  -491,   606,   607,   608,  -491,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,   -73,   -73,     9,   -73,   -73,     9,
     -73,   609,   610,   611,   612,   613,   614,   615,   616,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -491,  -491,   624,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
     290,  -491,  -352,  -490,   470,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -357,  -491,   448,  -491,   190,  -491,  -366,   458,
    -491,   221,  -491,  -358,  -491,  -491,  -491,   241,  -491,  -340,
    -325,   156,   542,  -491,  -491,  -491,  -491,   248,  -491,  -491,
    -491,  -491,   245,  -491,  -491,  -491,  -491,  -491,  -491,  -491,
    -491,  -491,  -491,   350,  -491,   449,  -491,  -491,   164,  -491,
     451,  -491,  -491,  -491,  -491,  -491,  -491,  -491,   240,  -367,
     132,  -491,   625,  -491,  -491,   443,  -491,   584,  -491,  -491,
    -164,  -491,   398,  -491,   397,  -491,  -119,  -491,    -1
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short int yytable[] =
{
      13,    15,   254,   255,    21,    23,   212,   213,   214,   461,
     216,   218,   220,   221,   441,   454,   494,   436,   217,   436,
     159,     1,     2,     3,     4,     5,    16,   426,   236,   504,
     208,   209,    10,   160,    11,   243,   259,   260,   256,   257,
     423,    18,   423,   210,   440,   356,   440,    28,    29,   467,
      24,    30,   468,   469,    78,    79,    80,    81,    82,    83,
      84,    85,    86,   256,   257,    87,   258,   346,   347,   208,
     209,    25,   470,   471,   472,    10,   594,    11,   473,   631,
     631,   529,   210,   518,   474,    26,   436,    27,   111,     6,
      75,   112,   357,   358,   461,   454,   113,   114,   423,   483,
     484,   161,   162,   163,   164,   165,    10,   596,    11,   413,
     195,   196,   197,   440,    10,   703,    11,   414,   116,   117,
     118,   206,   207,   166,   119,   120,   494,   167,   568,   568,
     415,   416,   417,   418,   121,   225,   226,   504,   419,   420,
     122,   231,   232,   566,   566,   123,   424,   237,   238,   124,
     240,   241,   298,   299,   300,   301,   421,   125,   350,   126,
     158,   404,   127,   467,   170,   128,   468,   469,   129,   130,
     131,   265,   267,   269,   271,   132,   133,   134,   135,   277,
     136,   137,   280,   282,   284,   286,   470,   471,   472,   138,
     291,   293,   473,   139,   140,   141,   142,   157,   474,   143,
     144,   145,   634,   634,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   483,   484,   103,   146,   147,   148,   627,
     149,   628,   629,   630,   447,   433,   448,   434,   435,   150,
     151,   152,   153,   154,   155,   156,   171,   172,   173,   174,
     175,   180,   411,   181,   182,   183,   344,   345,   184,   185,
     353,   354,   355,   186,   191,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,   192,   193,    53,    54,    55,
      56,    57,   194,   198,   201,   199,   202,   391,   413,   391,
      58,    59,    60,    61,    62,   203,   414,    63,    64,    65,
      66,    67,    68,    69,   467,   204,   205,   468,   469,   415,
     416,   417,   418,   215,   222,   223,   224,   419,   420,   227,
      70,   467,   228,   229,   468,   469,   230,   470,   471,   472,
     233,    71,    72,   473,   234,   421,   235,   239,   242,   474,
     244,   245,   263,   247,   470,   471,   472,   346,   464,   359,
     473,   248,   249,   250,   483,   484,   474,   251,   252,   410,
     627,   253,   635,   636,   637,   447,   433,   448,   434,   435,
     272,   483,   484,   273,   274,   275,   506,   287,   288,   289,
     290,   362,   295,   433,   296,   434,   435,   297,   304,   305,
     306,   573,   307,   308,   309,   413,   310,   311,   572,   312,
     582,   583,   313,   414,   314,   587,   315,   589,   316,   317,
     318,   319,   320,   321,   322,   360,   415,   416,   417,   418,
     323,   324,   325,   603,   419,   420,   326,   327,   328,   329,
     330,   613,   614,   331,   332,   413,   333,   334,   335,   336,
     622,   337,   421,   414,   431,   432,   443,   444,   445,   446,
     338,   447,   433,   448,   434,   435,   415,   416,   417,   418,
     339,   340,   341,   342,   419,   420,   343,   361,   363,   364,
     366,   368,   427,   370,   428,   429,   430,   378,   380,   382,
     384,   386,   421,   394,   431,   432,   395,   397,   399,   400,
     401,   402,   433,   405,   434,   435,   407,   408,   409,   257,
     442,   456,   462,   463,   497,   505,   499,   501,   502,   609,
     507,   508,   576,   509,   510,   580,   511,   512,   513,   514,
     584,   585,   515,   574,   516,   517,   519,   520,   521,   592,
     593,   595,   597,   522,   523,   524,   600,   601,   525,   602,
     526,   527,   413,   528,   606,   530,   531,   532,   612,   533,
     414,   615,   616,   534,   618,   619,   535,   621,   537,   538,
     540,   624,   625,   415,   416,   417,   418,   542,   543,   544,
     545,   419,   420,   546,   547,   548,   549,   550,   551,   552,
     467,   553,   554,   468,   469,   555,   556,   557,   558,   421,
     559,   560,   562,   443,   444,   457,   458,   563,   447,   433,
     448,   434,   435,   470,   471,   472,   564,   575,   577,   473,
     579,   581,   586,   467,   588,   474,   468,   469,   590,   591,
     598,   599,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   604,   485,   605,   607,   470,   471,   472,   608,
     617,   620,   473,   623,   626,   640,   641,   642,   474,    77,
     539,   643,   278,   412,   644,   646,   647,   489,   490,   491,
     648,   649,   650,   483,   484,   651,   652,   653,   654,   655,
     656,   657,   658,   659,   536,   660,   661,   662,   663,   664,
     665,   666,   667,   668,   669,   670,   671,   672,   673,   674,
     675,   676,   677,   701,   702,   704,   705,   706,   707,   708,
     679,   680,   681,   682,   683,   684,   685,   541,   686,   687,
     688,   689,   690,   691,   692,   693,   694,   695,   696,   697,
     698,   699,   700,   377,   709,   710,   711,   712,   713,   714,
     715,   716,   376,   645,   638,   561,   294,   496,   565,   393,
     392,   200,   246,   678,   571,     0,   398,   403,     0,   406
};

static const short int yycheck[] =
{
       1,     2,   166,   167,     5,     6,   125,   126,   127,   375,
     129,   130,   131,   132,   371,   373,   383,   369,    42,   371,
       7,     3,     4,     5,     6,     7,   112,   367,   147,   396,
      93,    94,   105,    20,   107,   154,    71,    72,   108,   109,
     365,   112,   367,   106,   369,   115,   371,     3,     4,    21,
       0,     7,    24,    25,     8,     9,    10,    11,    12,    13,
      14,    15,    16,   108,   109,    19,   111,   105,   106,    93,
      94,   111,    44,    45,    46,   105,   106,   107,    50,   569,
     570,   438,   106,   423,    56,   112,   438,   112,   114,    71,
     112,   114,   256,   257,   460,   453,   114,    50,   423,    71,
      72,    88,    89,    90,    91,    92,   105,   106,   107,    26,
     111,   112,   113,   438,   105,   106,   107,    34,   111,   114,
     114,   122,   123,   110,   114,   114,   493,   114,   495,   496,
      47,    48,    49,    50,   114,   136,   137,   504,    55,    56,
     114,   142,   143,   495,   496,   114,    63,   148,   149,   114,
     151,   152,    51,    52,    53,    54,    73,   114,   105,   114,
     111,   104,   114,    21,   112,   114,    24,    25,   114,   114,
     114,   172,   173,   174,   175,   114,   114,   114,   114,   180,
     114,   114,   183,   184,   185,   186,    44,    45,    46,   114,
     191,   192,    50,   114,   114,   114,   114,   113,    56,   114,
     114,   114,   569,   570,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    71,    72,    20,   114,   114,   114,    77,
     114,    79,    80,    81,    82,    83,    84,    85,    86,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   361,   113,   111,   114,   247,   248,   114,   114,
     251,   252,   253,   114,   114,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,   114,   113,    44,    45,    46,
      47,    48,   111,   114,   106,   113,   106,   288,    26,   290,
      57,    58,    59,    60,    61,   106,    34,    64,    65,    66,
      67,    68,    69,    70,    21,   106,   106,    24,    25,    47,
      48,    49,    50,   107,   106,   106,   106,    55,    56,   106,
      87,    21,   106,   106,    24,    25,   106,    44,    45,    46,
     106,    98,    99,    50,   106,    73,   106,   106,   106,    56,
     106,   106,   115,   114,    44,    45,    46,   105,    18,   113,
      50,   114,   114,   114,    71,    72,    56,   114,   114,   360,
      77,   114,    79,    80,    81,    82,    83,    84,    85,    86,
     114,    71,    72,   114,   114,   114,   105,   114,   114,   114,
     114,   113,   115,    83,   115,    85,    86,   115,   115,   115,
     115,   510,   115,   115,   115,    26,   115,   115,   106,   115,
     519,   520,   115,    34,   115,   524,   115,   526,   115,   115,
     115,   115,   115,   115,   115,   114,    47,    48,    49,    50,
     115,   115,   115,   542,    55,    56,   115,   115,   115,   115,
     115,   550,   551,   115,   115,    26,   115,   115,   115,   115,
     559,   115,    73,    34,    75,    76,    77,    78,    79,    80,
     115,    82,    83,    84,    85,    86,    47,    48,    49,    50,
     115,   115,   115,   115,    55,    56,   115,   114,   111,   115,
     115,   115,    63,   115,    65,    66,    67,   115,   115,   115,
     115,   115,    73,   115,    75,    76,   115,   115,   115,   115,
     115,   115,    83,   115,    85,    86,   115,   115,   115,   109,
     115,   115,   115,   115,   115,   111,   115,   115,   115,    62,
     115,   115,   513,   114,   114,   516,   114,   114,   114,   114,
     521,   522,   114,   106,   114,   114,   114,   114,   114,   530,
     531,   532,   533,   114,   114,   114,   537,   538,   114,   540,
     114,   114,    26,   114,   545,   114,   114,   114,   549,   114,
      34,   552,   553,   114,   555,   556,   114,   558,   114,   114,
     114,   562,   563,    47,    48,    49,    50,   114,   114,   114,
     114,    55,    56,   114,   114,   114,   114,   114,   114,   114,
      21,   114,   114,    24,    25,   114,   114,   114,   114,    73,
     114,   114,   114,    77,    78,    79,    80,   114,    82,    83,
      84,    85,    86,    44,    45,    46,   114,   106,   105,    50,
     106,   106,   106,    21,   106,    56,    24,    25,   106,   106,
     106,   106,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,   106,    74,   106,   106,    44,    45,    46,   106,
     106,   106,    50,   106,   106,   115,   115,   115,    56,    25,
     460,   115,   182,   363,   115,   115,   115,    65,    66,    67,
     115,   115,   115,    71,    72,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   453,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   694,   695,   696,   697,   698,   699,   700,
     115,   115,   115,   115,   115,   115,   115,   466,   115,   115,
     115,   115,   115,   115,   115,   115,   114,   114,   114,   114,
     114,   114,   114,   275,   115,   115,   115,   115,   115,   115,
     115,   115,   274,   577,   570,   487,   194,   387,   493,   290,
     289,   116,   158,   611,   504,    -1,   303,   349,    -1,   352
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,    71,   117,   118,   119,
     105,   107,   120,   224,   121,   224,   112,   122,   112,   123,
     215,   224,   124,   224,     0,   111,   112,   112,     3,     4,
       7,   208,   209,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    44,    45,    46,    47,    48,    57,    58,
      59,    60,    61,    64,    65,    66,    67,    68,    69,    70,
      87,    98,    99,   213,   214,   112,   125,   118,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    19,   130,   131,
     132,   133,   139,   160,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    20,   168,   169,   170,   175,   180,   200,
     201,   114,   114,   114,    50,   210,   111,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   113,   111,     7,
      20,    88,    89,    90,    91,    92,   110,   114,   216,   217,
     112,   114,   114,   114,   114,   114,   141,   140,   143,   142,
     114,   113,   111,   114,   114,   114,   114,   182,   181,   184,
     183,   114,   114,   113,   111,   224,   224,   224,   114,   113,
     208,   106,   106,   106,   106,   106,   224,   224,    93,    94,
     106,   222,   222,   222,   222,   107,   222,    42,   222,   223,
     222,   222,   106,   106,   106,   224,   224,   106,   106,   106,
     106,   224,   224,   106,   106,   106,   222,   224,   224,   106,
     224,   224,   106,   222,   106,   106,   213,   114,   114,   114,
     114,   114,   114,   114,   216,   216,   108,   109,   111,    71,
      72,   126,   127,   115,   134,   224,   136,   224,   146,   224,
     144,   224,   114,   114,   114,   114,   161,   224,   130,   171,
     224,   176,   224,   187,   224,   185,   224,   114,   114,   114,
     114,   224,   202,   224,   168,   115,   115,   115,    51,    52,
      53,    54,   211,   212,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   224,   224,   105,   106,   218,   219,
     105,   220,   221,   224,   224,   224,   115,   216,   216,   113,
     114,   114,   113,   111,   115,   135,   115,   137,   115,   147,
     115,   145,   155,   156,   150,   151,   155,   150,   115,   162,
     115,   172,   115,   177,   115,   188,   115,   186,   196,   224,
     191,   224,   196,   191,   115,   115,   203,   115,   211,   115,
     115,   115,   115,   218,   104,   115,   220,   115,   115,   115,
     224,   222,   126,    26,    34,    47,    48,    49,    50,    55,
      56,    73,   165,   166,    63,   138,   165,    63,    65,    66,
      67,    75,    76,    83,    85,    86,   128,   148,   149,   159,
     166,   148,   115,    77,    78,    79,    80,    82,    84,   128,
     129,   154,   157,   158,   159,   166,   115,    79,    80,   152,
     153,   154,   115,   115,    18,   163,   164,    21,    24,    25,
      44,    45,    46,    50,    56,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    74,   173,   174,   205,    65,
      66,    67,   178,   179,   205,   189,   189,   115,   197,   115,
     192,   115,   115,   204,   205,   111,   105,   115,   115,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   165,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   148,
     114,   114,   114,   114,   114,   114,   157,   114,   114,   152,
     114,   163,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   173,   114,   114,   114,   178,   128,   190,   205,   198,
     193,   204,   106,   222,   106,   106,   224,   105,   167,   106,
     224,   106,   222,   222,   224,   224,   106,   222,   106,   222,
     106,   106,   224,   224,   106,   224,   106,   224,   106,   106,
     224,   224,   224,   222,   106,   106,   224,   106,   106,    62,
     206,   207,   224,   222,   222,   224,   224,   106,   224,   224,
     106,   224,   222,   106,   224,   224,   106,    77,    79,    80,
      81,   129,   194,   199,   205,    79,    80,    81,   194,   195,
     115,   115,   115,   115,   115,   167,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   206,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   114,   114,   114,   114,   114,   114,
     114,   224,   224,   106,   224,   224,   224,   224,   224,   115,
     115,   115,   115,   115,   115,   115,   115
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
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
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
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
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
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



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

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
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */
			/*
				CID 10041 :Out-of-bounds access(Using "yyss" as an array. This might corrupt or misinterpret adjacent memory locations.)
			*/
			//1 no problem ????????????????????
      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

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
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
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

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
        case 5:
#line 199 "cfg-grammar.y"
    { cfg_add_source(configuration, yyvsp[0].ptr); }
    break;

  case 6:
#line 200 "cfg-grammar.y"
    { cfg_add_dest(configuration, yyvsp[0].ptr); }
    break;

  case 7:
#line 201 "cfg-grammar.y"
    { cfg_add_connection(configuration, yyvsp[0].ptr); }
    break;

  case 8:
#line 202 "cfg-grammar.y"
    { cfg_add_filter(configuration, yyvsp[0].ptr); }
    break;

  case 9:
#line 203 "cfg-grammar.y"
    { cfg_add_template(configuration, yyvsp[0].ptr); }
    break;

  case 10:
#line 204 "cfg-grammar.y"
    {  }
    break;

  case 11:
#line 208 "cfg-grammar.y"
    { yyval.ptr = log_source_group_new(yyvsp[-3].cptr, yyvsp[-1].ptr); free(yyvsp[-3].cptr); }
    break;

  case 12:
#line 212 "cfg-grammar.y"
    { yyval.ptr = log_dest_group_new(yyvsp[-3].cptr, yyvsp[-1].ptr); free(yyvsp[-3].cptr); }
    break;

  case 13:
#line 216 "cfg-grammar.y"
    { yyval.ptr = log_connection_new(yyvsp[-2].ptr, yyvsp[-1].num); }
    break;

  case 14:
#line 220 "cfg-grammar.y"
    { yyval.ptr = NULL; }
    break;

  case 15:
#line 225 "cfg-grammar.y"
    {
	    last_template = log_template_new(yyvsp[0].cptr, NULL);
	    free(yyvsp[0].cptr);
	  }
    break;

  case 16:
#line 229 "cfg-grammar.y"
    { yyval.ptr = last_template;  }
    break;

  case 19:
#line 238 "cfg-grammar.y"
    { last_template->template = g_string_new(yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 20:
#line 239 "cfg-grammar.y"
    { log_template_set_escape(last_template, yyvsp[-1].num); }
    break;

  case 21:
#line 243 "cfg-grammar.y"
    { last_sock_options->sndbuf = yyvsp[-1].num; }
    break;

  case 22:
#line 244 "cfg-grammar.y"
    { last_sock_options->rcvbuf = yyvsp[-1].num; }
    break;

  case 23:
#line 245 "cfg-grammar.y"
    { last_sock_options->broadcast = yyvsp[-1].num; }
    break;

  case 25:
#line 250 "cfg-grammar.y"
    { ((InetSocketOptions *) last_sock_options)->ttl = yyvsp[-1].num; }
    break;

  case 26:
#line 251 "cfg-grammar.y"
    { ((InetSocketOptions *) last_sock_options)->tos = yyvsp[-1].num; }
    break;

  case 27:
#line 255 "cfg-grammar.y"
    { log_drv_append(yyvsp[-2].ptr, yyvsp[0].ptr); log_drv_unref(yyvsp[0].ptr); yyval.ptr = yyvsp[-2].ptr; }
    break;

  case 28:
#line 256 "cfg-grammar.y"
    { yyval.ptr = NULL; }
    break;

  case 29:
#line 260 "cfg-grammar.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 30:
#line 261 "cfg-grammar.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 31:
#line 262 "cfg-grammar.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 32:
#line 263 "cfg-grammar.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 33:
#line 267 "cfg-grammar.y"
    { yyval.ptr = afinter_sd_new(); }
    break;

  case 34:
#line 271 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 35:
#line 272 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 36:
#line 277 "cfg-grammar.y"
    {
	    last_driver = affile_sd_new(yyvsp[0].cptr, 0); 
	    free(yyvsp[0].cptr); 
	    last_reader_options = &((AFFileSourceDriver *) last_driver)->reader_options;
	  }
    break;

  case 37:
#line 282 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 38:
#line 287 "cfg-grammar.y"
    {
	    last_driver = affile_sd_new(yyvsp[0].cptr, AFFILE_PIPE); 
	    free(yyvsp[0].cptr); 
	    last_reader_options = &((AFFileSourceDriver *) last_driver)->reader_options;
	  }
    break;

  case 39:
#line 292 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 40:
#line 296 "cfg-grammar.y"
    { last_driver->optional = yyvsp[-1].num; }
    break;

  case 41:
#line 297 "cfg-grammar.y"
    {}
    break;

  case 42:
#line 301 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 43:
#line 302 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 44:
#line 303 "cfg-grammar.y"
    { last_addr_family = AF_INET; }
    break;

  case 45:
#line 303 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 46:
#line 304 "cfg-grammar.y"
    { last_addr_family = AF_INET; }
    break;

  case 47:
#line 304 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 48:
#line 305 "cfg-grammar.y"
    { last_addr_family = AF_INET6; }
    break;

  case 49:
#line 305 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 50:
#line 306 "cfg-grammar.y"
    { last_addr_family = AF_INET6; }
    break;

  case 51:
#line 306 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 52:
#line 311 "cfg-grammar.y"
    { 
	    last_driver = afunix_sd_new(
		yyvsp[0].cptr,
		AFSOCKET_DGRAM | AFSOCKET_LOCAL); 
	    free(yyvsp[0].cptr); 
	    last_reader_options = &((AFSocketSourceDriver *) last_driver)->reader_options;
	    last_sock_options = &((AFUnixSourceDriver *) last_driver)->sock_options;
	  }
    break;

  case 53:
#line 319 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 54:
#line 324 "cfg-grammar.y"
    { 
	    last_driver = afunix_sd_new(
		yyvsp[0].cptr,
		AFSOCKET_STREAM | AFSOCKET_KEEP_ALIVE | AFSOCKET_LOCAL);
	    free(yyvsp[0].cptr);
	    last_reader_options = &((AFSocketSourceDriver *) last_driver)->reader_options;
	    last_sock_options = &((AFUnixSourceDriver *) last_driver)->sock_options;
	  }
    break;

  case 55:
#line 332 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 58:
#line 342 "cfg-grammar.y"
    { afunix_sd_set_uid(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 59:
#line 343 "cfg-grammar.y"
    { afunix_sd_set_gid(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 60:
#line 344 "cfg-grammar.y"
    { afunix_sd_set_perm(last_driver, yyvsp[-1].num); }
    break;

  case 61:
#line 345 "cfg-grammar.y"
    { last_driver->optional = yyvsp[-1].num; }
    break;

  case 62:
#line 346 "cfg-grammar.y"
    {}
    break;

  case 63:
#line 347 "cfg-grammar.y"
    {}
    break;

  case 64:
#line 348 "cfg-grammar.y"
    {}
    break;

  case 65:
#line 353 "cfg-grammar.y"
    { 
	    last_driver = afinet_sd_new(last_addr_family,
			NULL, 514,
			AFSOCKET_DGRAM);
	    last_reader_options = &((AFSocketSourceDriver *) last_driver)->reader_options;
	    last_sock_options = &((AFInetSourceDriver *) last_driver)->sock_options.super;
	  }
    break;

  case 66:
#line 360 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 70:
#line 370 "cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, 0, yyvsp[-1].cptr, "udp"); free(yyvsp[-1].cptr); }
    break;

  case 71:
#line 371 "cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, 0, yyvsp[-1].cptr, "udp"); free(yyvsp[-1].cptr); }
    break;

  case 72:
#line 375 "cfg-grammar.y"
    { afinet_sd_set_localip(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 73:
#line 376 "cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, yyvsp[-1].num, NULL, NULL); }
    break;

  case 74:
#line 377 "cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, yyvsp[-1].num, NULL, NULL); }
    break;

  case 75:
#line 378 "cfg-grammar.y"
    { afinet_sd_set_localip(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 78:
#line 385 "cfg-grammar.y"
    { 
	    last_driver = afinet_sd_new(last_addr_family,
			NULL, 514,
			AFSOCKET_STREAM);
	    last_reader_options = &((AFSocketSourceDriver *) last_driver)->reader_options;
	    last_sock_options = &((AFInetSourceDriver *) last_driver)->sock_options.super;
	  }
    break;

  case 79:
#line 392 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 83:
#line 402 "cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, 0, yyvsp[-1].cptr, "tcp"); free(yyvsp[-1].cptr); }
    break;

  case 84:
#line 403 "cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, 0, yyvsp[-1].cptr, "tcp"); free(yyvsp[-1].cptr); }
    break;

  case 85:
#line 404 "cfg-grammar.y"
    {}
    break;

  case 86:
#line 408 "cfg-grammar.y"
    { afsocket_sd_set_keep_alive(last_driver, yyvsp[-1].num); }
    break;

  case 87:
#line 409 "cfg-grammar.y"
    { afsocket_sd_set_max_connections(last_driver, yyvsp[-1].num); }
    break;

  case 88:
#line 413 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 89:
#line 418 "cfg-grammar.y"
    { 
	    last_driver = afstreams_sd_new(yyvsp[0].cptr); 
	    free(yyvsp[0].cptr); 
	  }
    break;

  case 90:
#line 422 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 93:
#line 431 "cfg-grammar.y"
    { afstreams_sd_set_sundoor(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 96:
#line 440 "cfg-grammar.y"
    { last_reader_options->options = yyvsp[-1].num; }
    break;

  case 97:
#line 441 "cfg-grammar.y"
    { last_reader_options->msg_size = yyvsp[-1].num; }
    break;

  case 98:
#line 442 "cfg-grammar.y"
    { last_reader_options->source_opts.init_window_size = yyvsp[-1].num; }
    break;

  case 99:
#line 443 "cfg-grammar.y"
    { last_reader_options->fetch_limit = yyvsp[-1].num; }
    break;

  case 100:
#line 444 "cfg-grammar.y"
    { last_reader_options->prefix = yyvsp[-1].cptr; }
    break;

  case 101:
#line 445 "cfg-grammar.y"
    { last_reader_options->padding = yyvsp[-1].num; }
    break;

  case 102:
#line 446 "cfg-grammar.y"
    { last_reader_options->follow_freq = yyvsp[-1].num; }
    break;

  case 103:
#line 447 "cfg-grammar.y"
    { cfg_timezone_value(yyvsp[-1].cptr, &last_reader_options->zone_offset); free(yyvsp[-1].cptr); }
    break;

  case 104:
#line 448 "cfg-grammar.y"
    { last_reader_options->keep_timestamp = yyvsp[-1].num; }
    break;

  case 105:
#line 452 "cfg-grammar.y"
    { yyval.num = lookup_parse_flag(yyvsp[-1].cptr) | yyvsp[0].num; free(yyvsp[-1].cptr); }
    break;

  case 106:
#line 453 "cfg-grammar.y"
    { yyval.num = 0; }
    break;

  case 107:
#line 458 "cfg-grammar.y"
    { log_drv_append(yyvsp[-2].ptr, yyvsp[0].ptr); log_drv_unref(yyvsp[0].ptr); yyval.ptr = yyvsp[-2].ptr; }
    break;

  case 108:
#line 459 "cfg-grammar.y"
    { yyval.ptr = NULL; }
    break;

  case 109:
#line 463 "cfg-grammar.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 110:
#line 464 "cfg-grammar.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 111:
#line 465 "cfg-grammar.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 112:
#line 466 "cfg-grammar.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 113:
#line 467 "cfg-grammar.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 114:
#line 471 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 115:
#line 476 "cfg-grammar.y"
    { 
	    last_driver = affile_dd_new(yyvsp[0].cptr, 0); 
	    free(yyvsp[0].cptr); 
	    last_writer_options = &((AFFileDestDriver *) last_driver)->writer_options;
	  }
    break;

  case 116:
#line 482 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 120:
#line 492 "cfg-grammar.y"
    { last_driver->optional = yyvsp[-1].num; }
    break;

  case 121:
#line 497 "cfg-grammar.y"
    { affile_dd_set_file_uid(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 122:
#line 498 "cfg-grammar.y"
    { affile_dd_set_file_gid(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 123:
#line 499 "cfg-grammar.y"
    { affile_dd_set_file_perm(last_driver, yyvsp[-1].num); }
    break;

  case 124:
#line 500 "cfg-grammar.y"
    { affile_dd_set_dir_uid(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 125:
#line 501 "cfg-grammar.y"
    { affile_dd_set_dir_gid(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 126:
#line 502 "cfg-grammar.y"
    { affile_dd_set_dir_perm(last_driver, yyvsp[-1].num); }
    break;

  case 127:
#line 503 "cfg-grammar.y"
    { affile_dd_set_create_dirs(last_driver, yyvsp[-1].num); }
    break;

  case 128:
#line 504 "cfg-grammar.y"
    { affile_dd_set_remove_if_older(last_driver, yyvsp[-1].num); }
    break;

  case 129:
#line 508 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 130:
#line 513 "cfg-grammar.y"
    { 
	    last_driver = affile_dd_new(yyvsp[0].cptr, AFFILE_NO_EXPAND | AFFILE_PIPE);
	    free(yyvsp[0].cptr); 
	    last_writer_options = &((AFFileDestDriver *) last_driver)->writer_options;
	    last_writer_options->flush_lines = 0;
	  }
    break;

  case 131:
#line 519 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 135:
#line 529 "cfg-grammar.y"
    { affile_dd_set_file_uid(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 136:
#line 530 "cfg-grammar.y"
    { affile_dd_set_file_gid(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 137:
#line 531 "cfg-grammar.y"
    { affile_dd_set_file_perm(last_driver, yyvsp[-1].num); }
    break;

  case 138:
#line 536 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 139:
#line 537 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 140:
#line 538 "cfg-grammar.y"
    { last_addr_family = AF_INET; }
    break;

  case 141:
#line 538 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 142:
#line 539 "cfg-grammar.y"
    { last_addr_family = AF_INET; }
    break;

  case 143:
#line 539 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 144:
#line 540 "cfg-grammar.y"
    { last_addr_family = AF_INET6; }
    break;

  case 145:
#line 540 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 146:
#line 541 "cfg-grammar.y"
    { last_addr_family = AF_INET6; }
    break;

  case 147:
#line 541 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 148:
#line 546 "cfg-grammar.y"
    { 
	    last_driver = afunix_dd_new(yyvsp[0].cptr, AFSOCKET_DGRAM);
	    free(yyvsp[0].cptr);
	    last_writer_options = &((AFSocketDestDriver *) last_driver)->writer_options;
	    last_sock_options = &((AFUnixDestDriver *) last_driver)->sock_options;
	  }
    break;

  case 149:
#line 552 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 150:
#line 557 "cfg-grammar.y"
    { 
	    last_driver = afunix_dd_new(yyvsp[0].cptr, AFSOCKET_STREAM);
	    free(yyvsp[0].cptr);
	    last_writer_options = &((AFSocketDestDriver *) last_driver)->writer_options;
	    last_sock_options = &((AFUnixDestDriver *) last_driver)->sock_options;
	  }
    break;

  case 151:
#line 563 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 156:
#line 578 "cfg-grammar.y"
    { 
	    last_driver = afinet_dd_new(last_addr_family,
			yyvsp[0].cptr, 514,
			AFSOCKET_DGRAM);
	    free(yyvsp[0].cptr);
	    last_writer_options = &((AFSocketDestDriver *) last_driver)->writer_options;
	    last_sock_options = &((AFInetDestDriver *) last_driver)->sock_options.super;
	  }
    break;

  case 157:
#line 586 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 160:
#line 596 "cfg-grammar.y"
    { afinet_dd_set_localip(last_driver, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 161:
#line 597 "cfg-grammar.y"
    { afinet_dd_set_destport(last_driver, yyvsp[-1].num, NULL, NULL); }
    break;

  case 165:
#line 604 "cfg-grammar.y"
    { afinet_dd_set_localport(last_driver, 0, yyvsp[-1].cptr, "udp"); free(yyvsp[-1].cptr); }
    break;

  case 166:
#line 605 "cfg-grammar.y"
    { afinet_dd_set_destport(last_driver, 0, yyvsp[-1].cptr, "udp"); free(yyvsp[-1].cptr); }
    break;

  case 167:
#line 606 "cfg-grammar.y"
    { afinet_dd_set_destport(last_driver, 0, yyvsp[-1].cptr, "udp"); free(yyvsp[-1].cptr); }
    break;

  case 168:
#line 611 "cfg-grammar.y"
    { 
	    last_driver = afinet_dd_new(last_addr_family,
			yyvsp[0].cptr, 514,
			AFSOCKET_STREAM); 
	    free(yyvsp[0].cptr);
	    last_writer_options = &((AFSocketDestDriver *) last_driver)->writer_options;
	    last_sock_options = &((AFInetDestDriver *) last_driver)->sock_options.super;
	  }
    break;

  case 169:
#line 619 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 173:
#line 629 "cfg-grammar.y"
    { afinet_dd_set_localport(last_driver, 0, yyvsp[-1].cptr, "tcp"); free(yyvsp[-1].cptr); }
    break;

  case 174:
#line 630 "cfg-grammar.y"
    { afinet_dd_set_destport(last_driver, 0, yyvsp[-1].cptr, "tcp"); free(yyvsp[-1].cptr); }
    break;

  case 175:
#line 631 "cfg-grammar.y"
    { afinet_dd_set_destport(last_driver, 0, yyvsp[-1].cptr, "tcp"); free(yyvsp[-1].cptr); }
    break;

  case 176:
#line 641 "cfg-grammar.y"
    { yyval.ptr = afuser_dd_new(yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 177:
#line 645 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 178:
#line 650 "cfg-grammar.y"
    { 
	    last_driver = afprogram_dd_new(yyvsp[0].cptr); 
	    free(yyvsp[0].cptr); 
	    last_writer_options = &((AFProgramDestDriver *) last_driver)->writer_options;
	  }
    break;

  case 179:
#line 655 "cfg-grammar.y"
    { yyval.ptr = last_driver; }
    break;

  case 182:
#line 664 "cfg-grammar.y"
    { last_writer_options->options = yyvsp[-1].num; }
    break;

  case 183:
#line 665 "cfg-grammar.y"
    { last_writer_options->fifo_size = yyvsp[-1].num; }
    break;

  case 184:
#line 666 "cfg-grammar.y"
    { last_writer_options->flush_lines = yyvsp[-1].num; }
    break;

  case 185:
#line 667 "cfg-grammar.y"
    { last_writer_options->flush_timeout = yyvsp[-1].num; }
    break;

  case 186:
#line 668 "cfg-grammar.y"
    { last_writer_options->template = cfg_lookup_template(configuration, yyvsp[-1].cptr);
	                                          if (last_writer_options->template == NULL)
	                                            {
	                                              last_writer_options->template = log_template_new(NULL, yyvsp[-1].cptr); 
	                                              last_writer_options->template->def_inline = TRUE;
	                                            }
	                                          else
	                                            log_template_ref(last_writer_options->template);
	                                          free(yyvsp[-1].cptr);
	                                        }
    break;

  case 187:
#line 678 "cfg-grammar.y"
    { log_writer_options_set_template_escape(last_writer_options, yyvsp[-1].num); }
    break;

  case 188:
#line 679 "cfg-grammar.y"
    { msg_error("fsync() does not work yet", NULL); }
    break;

  case 189:
#line 680 "cfg-grammar.y"
    { cfg_timezone_value(yyvsp[-1].cptr, &last_writer_options->zone_offset); free(yyvsp[-1].cptr); }
    break;

  case 190:
#line 681 "cfg-grammar.y"
    { last_writer_options->ts_format = cfg_ts_format_value(yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 191:
#line 682 "cfg-grammar.y"
    { last_writer_options->frac_digits = yyvsp[-1].num; }
    break;

  case 192:
#line 686 "cfg-grammar.y"
    { yyval.num = yyvsp[-1].num | yyvsp[0].num; }
    break;

  case 193:
#line 687 "cfg-grammar.y"
    { yyval.num = 0; }
    break;

  case 194:
#line 691 "cfg-grammar.y"
    { yyval.num = LWO_TMPL_ESCAPE; }
    break;

  case 195:
#line 696 "cfg-grammar.y"
    { log_endpoint_append(yyvsp[-2].ptr, yyvsp[0].ptr); yyval.ptr = yyvsp[-2].ptr; }
    break;

  case 196:
#line 697 "cfg-grammar.y"
    { yyval.ptr = NULL; }
    break;

  case 197:
#line 701 "cfg-grammar.y"
    { yyval.ptr = log_endpoint_new(EP_SOURCE, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 198:
#line 702 "cfg-grammar.y"
    { yyval.ptr = log_endpoint_new(EP_FILTER, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 199:
#line 703 "cfg-grammar.y"
    { yyval.ptr = log_endpoint_new(EP_DESTINATION, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 200:
#line 707 "cfg-grammar.y"
    { yyval.num = yyvsp[-2].num; }
    break;

  case 201:
#line 708 "cfg-grammar.y"
    { yyval.num = 0; }
    break;

  case 202:
#line 713 "cfg-grammar.y"
    { yyval.num |= yyvsp[0].num; }
    break;

  case 203:
#line 714 "cfg-grammar.y"
    { yyval.num = 0; }
    break;

  case 204:
#line 718 "cfg-grammar.y"
    { yyval.num = LC_CATCHALL; }
    break;

  case 205:
#line 719 "cfg-grammar.y"
    { yyval.num = LC_FALLBACK; }
    break;

  case 206:
#line 720 "cfg-grammar.y"
    { yyval.num = LC_FINAL; }
    break;

  case 207:
#line 721 "cfg-grammar.y"
    { yyval.num = LC_FLOW_CONTROL; }
    break;

  case 208:
#line 725 "cfg-grammar.y"
    { yyval.ptr = yyvsp[-2].ptr; }
    break;

  case 209:
#line 726 "cfg-grammar.y"
    { yyval.ptr = NULL; }
    break;

  case 210:
#line 730 "cfg-grammar.y"
    { configuration->mark_freq = yyvsp[-1].num; }
    break;

  case 211:
#line 731 "cfg-grammar.y"
    { configuration->stats_freq = yyvsp[-1].num; }
    break;

  case 212:
#line 732 "cfg-grammar.y"
    { configuration->flush_lines = yyvsp[-1].num; }
    break;

  case 213:
#line 733 "cfg-grammar.y"
    { configuration->flush_timeout = yyvsp[-1].num; }
    break;

  case 214:
#line 734 "cfg-grammar.y"
    { configuration->chain_hostnames = yyvsp[-1].num; }
    break;

  case 215:
#line 735 "cfg-grammar.y"
    { configuration->normalize_hostnames = yyvsp[-1].num; }
    break;

  case 216:
#line 736 "cfg-grammar.y"
    { configuration->keep_hostname = yyvsp[-1].num; }
    break;

  case 217:
#line 737 "cfg-grammar.y"
    { configuration->check_hostname = yyvsp[-1].num; }
    break;

  case 218:
#line 738 "cfg-grammar.y"
    { cfg_bad_hostname_set(configuration, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 219:
#line 739 "cfg-grammar.y"
    { configuration->use_time_recvd = yyvsp[-1].num; }
    break;

  case 220:
#line 740 "cfg-grammar.y"
    { configuration->use_fqdn = yyvsp[-1].num; }
    break;

  case 221:
#line 741 "cfg-grammar.y"
    { configuration->use_dns = yyvsp[-1].num; }
    break;

  case 222:
#line 742 "cfg-grammar.y"
    { configuration->time_reopen = yyvsp[-1].num; }
    break;

  case 223:
#line 743 "cfg-grammar.y"
    { configuration->time_reap = yyvsp[-1].num; }
    break;

  case 224:
#line 745 "cfg-grammar.y"
    { 
		  configuration->time_sleep = yyvsp[-1].num; 
		  if (yyvsp[-1].num > 500) 
		    { 
		      msg_notice("The value specified for time_sleep is too large", evt_tag_int("time_sleep", yyvsp[-1].num), NULL);
		      configuration->time_sleep = 500;
		    }
		}
    break;

  case 225:
#line 753 "cfg-grammar.y"
    { configuration->log_fifo_size = yyvsp[-1].num; }
    break;

  case 226:
#line 754 "cfg-grammar.y"
    { configuration->log_iw_size = yyvsp[-1].num; }
    break;

  case 227:
#line 755 "cfg-grammar.y"
    { configuration->log_fetch_limit = yyvsp[-1].num; }
    break;

  case 228:
#line 756 "cfg-grammar.y"
    { configuration->log_msg_size = yyvsp[-1].num; }
    break;

  case 229:
#line 757 "cfg-grammar.y"
    { configuration->keep_timestamp = yyvsp[-1].num; }
    break;

  case 230:
#line 758 "cfg-grammar.y"
    { configuration->ts_format = cfg_ts_format_value(yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 231:
#line 759 "cfg-grammar.y"
    { configuration->frac_digits = yyvsp[-1].num; }
    break;

  case 232:
#line 760 "cfg-grammar.y"
    { /* ignored */; }
    break;

  case 233:
#line 761 "cfg-grammar.y"
    { /* ignored */; }
    break;

  case 234:
#line 762 "cfg-grammar.y"
    { configuration->create_dirs = yyvsp[-1].num; }
    break;

  case 235:
#line 763 "cfg-grammar.y"
    { cfg_file_owner_set(configuration, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 236:
#line 764 "cfg-grammar.y"
    { cfg_file_group_set(configuration, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 237:
#line 765 "cfg-grammar.y"
    { cfg_file_perm_set(configuration, yyvsp[-1].num); }
    break;

  case 238:
#line 766 "cfg-grammar.y"
    { cfg_dir_owner_set(configuration, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 239:
#line 767 "cfg-grammar.y"
    { cfg_dir_group_set(configuration, yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 240:
#line 768 "cfg-grammar.y"
    { cfg_dir_perm_set(configuration, yyvsp[-1].num); }
    break;

  case 241:
#line 769 "cfg-grammar.y"
    { configuration->use_dns_cache = yyvsp[-1].num; }
    break;

  case 242:
#line 770 "cfg-grammar.y"
    { configuration->dns_cache_size = yyvsp[-1].num; }
    break;

  case 243:
#line 771 "cfg-grammar.y"
    { configuration->dns_cache_expire = yyvsp[-1].num; }
    break;

  case 244:
#line 773 "cfg-grammar.y"
    { configuration->dns_cache_expire_failed = yyvsp[-1].num; }
    break;

  case 245:
#line 774 "cfg-grammar.y"
    { configuration->dns_cache_hosts = yyvsp[-1].cptr; }
    break;

  case 246:
#line 775 "cfg-grammar.y"
    { configuration->file_template_name = yyvsp[-1].cptr; }
    break;

  case 247:
#line 776 "cfg-grammar.y"
    { configuration->proto_template_name = yyvsp[-1].cptr; }
    break;

  case 248:
#line 777 "cfg-grammar.y"
    { cfg_timezone_value(yyvsp[-1].cptr, &configuration->recv_zone_offset); free(yyvsp[-1].cptr); }
    break;

  case 249:
#line 778 "cfg-grammar.y"
    { cfg_timezone_value(yyvsp[-1].cptr, &configuration->send_zone_offset); free(yyvsp[-1].cptr); }
    break;

  case 250:
#line 782 "cfg-grammar.y"
    { yyval.ptr = log_filter_rule_new(yyvsp[-4].cptr, yyvsp[-2].node); free(yyvsp[-4].cptr); }
    break;

  case 251:
#line 786 "cfg-grammar.y"
    { yyval.node = yyvsp[0].node; if (!yyvsp[0].node) return 1; }
    break;

  case 252:
#line 787 "cfg-grammar.y"
    { yyvsp[0].node->comp = !(yyvsp[0].node->comp); yyval.node = yyvsp[0].node; }
    break;

  case 253:
#line 788 "cfg-grammar.y"
    { yyval.node = fop_or_new(yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 254:
#line 789 "cfg-grammar.y"
    { yyval.node = fop_and_new(yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 255:
#line 790 "cfg-grammar.y"
    { yyval.node = yyvsp[-1].node; }
    break;

  case 256:
#line 794 "cfg-grammar.y"
    { yyval.node = filter_facility_new(yyvsp[-1].num);  }
    break;

  case 257:
#line 795 "cfg-grammar.y"
    { yyval.node = filter_facility_new(0x80000000 | yyvsp[-1].num); }
    break;

  case 258:
#line 796 "cfg-grammar.y"
    { yyval.node = filter_level_new(yyvsp[-1].num); }
    break;

  case 259:
#line 797 "cfg-grammar.y"
    { yyval.node = filter_prog_new(yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 260:
#line 798 "cfg-grammar.y"
    { yyval.node = filter_host_new(yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 261:
#line 799 "cfg-grammar.y"
    { yyval.node = filter_match_new(yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 262:
#line 800 "cfg-grammar.y"
    { yyval.node = filter_call_new(yyvsp[-1].cptr, configuration); free(yyvsp[-1].cptr); }
    break;

  case 263:
#line 801 "cfg-grammar.y"
    { yyval.node = filter_netmask_new(yyvsp[-1].cptr); free(yyvsp[-1].cptr); }
    break;

  case 264:
#line 805 "cfg-grammar.y"
    { yyval.num = yyvsp[-1].num + yyvsp[0].num; }
    break;

  case 265:
#line 806 "cfg-grammar.y"
    { yyval.num = yyvsp[0].num; }
    break;

  case 266:
#line 811 "cfg-grammar.y"
    { 
	    int n = syslog_name_lookup_facility_by_name(yyvsp[0].cptr);
	    if (n == -1)
	      {
	        msg_error("Warning: Unknown facility", 
	                  evt_tag_str("facility", yyvsp[0].cptr),
	                  NULL);
	        yyval.num = 0;
	      }
	    else
	      yyval.num = (1 << n); 
	    free(yyvsp[0].cptr); 
	  }
    break;

  case 267:
#line 827 "cfg-grammar.y"
    { yyval.num = yyvsp[-1].num + yyvsp[0].num; }
    break;

  case 268:
#line 828 "cfg-grammar.y"
    { yyval.num = yyvsp[0].num; }
    break;

  case 269:
#line 833 "cfg-grammar.y"
    { 
	    int r1, r2;
	    r1 = syslog_name_lookup_level_by_name(yyvsp[-2].cptr);
	    if (r1 == -1)
	      msg_error("Warning: Unknown priority level",
                        evt_tag_str("priority", yyvsp[-2].cptr),
                        NULL);
	    r2 = syslog_name_lookup_level_by_name(yyvsp[0].cptr);
	    if (r2 == -1)
	      msg_error("Warning: Unknown priority level",
                        evt_tag_str("priority", yyvsp[-2].cptr),
                        NULL);
	    if (r1 != -1 && r2 != -1)
	      yyval.num = syslog_make_range(r1, r2); 
	    else
	      yyval.num = 0;
	    free(yyvsp[-2].cptr); 
	    free(yyvsp[0].cptr); 
	  }
    break;

  case 270:
#line 853 "cfg-grammar.y"
    { 
	    int n = syslog_name_lookup_level_by_name(yyvsp[0].cptr); 
	    if (n == -1)
	      {
	        msg_error("Warning: Unknown priority level",
                          evt_tag_str("priority", yyvsp[0].cptr),
                          NULL);
	        yyval.num = 0;
	      }
	    else
	      yyval.num = 1 << n;
	    free(yyvsp[0].cptr); 
	  }
    break;

  case 271:
#line 869 "cfg-grammar.y"
    { yyval.num = 1; }
    break;

  case 272:
#line 870 "cfg-grammar.y"
    { yyval.num = 0; }
    break;

  case 273:
#line 871 "cfg-grammar.y"
    { yyval.num = yyvsp[0].num; }
    break;

  case 274:
#line 875 "cfg-grammar.y"
    { yyval.num = yyvsp[0].num; }
    break;

  case 275:
#line 876 "cfg-grammar.y"
    { yyval.num = 2; }
    break;


    }

/* Line 1010 of yacc.c.  */
#line 3197 "cfg-grammar.c"

  yyvsp -= yylen;
  yyssp -= yylen;


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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
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

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
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

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 884 "cfg-grammar.y"


extern int linenum;

void 
yyerror(char *msg)
{
  fprintf(stderr, "%s at %d\n", msg, linenum);
}

void
yyparser_reset(void)
{
  last_driver = NULL;
  last_reader_options = NULL;
  last_writer_options = NULL;
  last_template = NULL;
}
