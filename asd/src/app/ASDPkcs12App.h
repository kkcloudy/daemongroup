#ifndef ASD_PKCS12_APP_H
#define ASD_PKCS12_APP_H

#include "e_os.h"

#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/lhash.h>
#include <openssl/conf.h>
#include <openssl/txt_db.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
#include <openssl/ossl_typ.h>

int app_RAND_load_file(const char *file, BIO *bio_e, int dont_warn);
int app_RAND_write_file(const char *file, BIO *bio_e);
/* When `file' is NULL, use defaults.
 * `bio_e' is for error messages. */
void app_RAND_allow_write_file(void);
long app_RAND_load_files(char *file); /* `file' is a list of files to read,
                                       * separated by LIST_SEPARATOR_CHAR
                                       * (see e_os.h).  The string is
                                       * destroyed! */

#ifdef OPENSSL_SYS_WIN32
#define rename(from,to) WIN32_rename((from),(to))
int WIN32_rename(const char *oldname,const char *newname);
#endif


extern CONF *config;
extern char *default_config_file;
extern BIO *bio_err;


#ifndef OPENSSL_SYS_NETWARE
#include <signal.h>
#endif

#ifdef SIGPIPE
#define do_pipe_sig()	signal(SIGPIPE,SIG_IGN)
#else
#define do_pipe_sig()
#endif

#if defined(MONOLITH) && !defined(OPENSSL_C)
#  define apps_startup() \
		do_pipe_sig()
#  define apps_shutdown()
#else
#  ifndef OPENSSL_NO_ENGINE
#    if defined(OPENSSL_SYS_MSDOS) || defined(OPENSSL_SYS_WIN16) || \
     defined(OPENSSL_SYS_WIN32)
#      ifdef _O_BINARY
#        define apps_startup() \
			do { _fmode=_O_BINARY; do_pipe_sig(); CRYPTO_malloc_init(); \
			ERR_load_crypto_strings(); OpenSSL_add_all_algorithms(); \
			ENGINE_load_builtin_engines(); setup_ui_method(); } while(0)
#      else
#        define apps_startup() \
			do { _fmode=O_BINARY; do_pipe_sig(); CRYPTO_malloc_init(); \
			ERR_load_crypto_strings(); OpenSSL_add_all_algorithms(); \
			ENGINE_load_builtin_engines(); setup_ui_method(); } while(0)
#      endif
#    else
#      define apps_startup() \
			do { do_pipe_sig(); OpenSSL_add_all_algorithms(); \
			ERR_load_crypto_strings(); ENGINE_load_builtin_engines(); \
			setup_ui_method(); } while(0)
#    endif
#    define apps_shutdown() \
			do { CONF_modules_unload(1); destroy_ui_method(); \
			EVP_cleanup(); ENGINE_cleanup(); \
			CRYPTO_cleanup_all_ex_data(); ERR_remove_state(0); \
			ERR_free_strings(); } while(0)
#  else
#    if defined(OPENSSL_SYS_MSDOS) || defined(OPENSSL_SYS_WIN16) || \
     defined(OPENSSL_SYS_WIN32)
#      ifdef _O_BINARY
#        define apps_startup() \
			do { _fmode=_O_BINARY; do_pipe_sig(); CRYPTO_malloc_init(); \
			ERR_load_crypto_strings(); OpenSSL_add_all_algorithms(); \
			setup_ui_method(); } while(0)
#      else
#        define apps_startup() \
			do { _fmode=O_BINARY; do_pipe_sig(); CRYPTO_malloc_init(); \
			ERR_load_crypto_strings(); OpenSSL_add_all_algorithms(); \
			setup_ui_method(); } while(0)
#      endif
#    else
#      define apps_startup() \
			do { do_pipe_sig(); OpenSSL_add_all_algorithms(); \
			ERR_load_crypto_strings(); \
			setup_ui_method(); } while(0)
#    endif
#    define apps_shutdown() \
			do { CONF_modules_unload(1); destroy_ui_method(); \
			EVP_cleanup(); \
			CRYPTO_cleanup_all_ex_data(); ERR_remove_state(0); \
			ERR_free_strings(); } while(0)
#  endif
#endif

typedef struct args_st
	{
	char **data;
	int count;
	} ARGS;

#define PW_MIN_LENGTH 4
typedef struct pw_cb_data
	{
	const void *password;
	const char *prompt_info;
	} PW_CB_DATA;

int password_callback(char *buf, int bufsiz, int verify,
	PW_CB_DATA *cb_data);

int setup_ui_method(void);
void destroy_ui_method(void);

int should_retry(int i);
int args_from_file(char *file, int *argc, char **argv[]);
int str2fmt(char *s);
void program_name(char *in,char *out,int size);
int chopup_args(ARGS *arg,char *buf, int *argc, char **argv[]);
#ifdef HEADER_X509_H
int dump_cert_text(BIO *out, X509 *x);
void print_name(BIO *out, const char *title, X509_NAME *nm, unsigned long lflags);
#endif
int set_cert_ex(unsigned long *flags, const char *arg);
int set_name_ex(unsigned long *flags, const char *arg);
int set_ext_copy(int *copy_type, const char *arg);
int copy_extensions(X509 *x, X509_REQ *req, int copy_type);
int app_passwd(BIO *err, char *arg1, char *arg2, char **pass1, char **pass2);
int add_oid_section(BIO *err, CONF *conf);
X509 *load_cert(BIO *err, const char *file, int format,
	const char *pass, ENGINE *e, const char *cert_descrip);
EVP_PKEY *load_key(BIO *err, const char *file, int format, int maybe_stdin,
	const char *pass, ENGINE *e, const char *key_descrip);
EVP_PKEY *load_pubkey(BIO *err, const char *file, int format, int maybe_stdin,
	const char *pass, ENGINE *e, const char *key_descrip);
STACK_OF(X509) *load_certs(BIO *err, const char *file, int format,
	const char *pass, ENGINE *e, const char *cert_descrip);
X509_STORE *setup_verify(BIO *bp, char *CAfile, char *CApath);
#ifndef OPENSSL_NO_ENGINE
ENGINE *setup_engine(BIO *err, const char *engine, int debug);
#endif

int load_config(BIO *err, CONF *cnf);
char *make_config_name(void);

/* Functions defined in ca.c and also used in ocsp.c */
int unpack_revinfo(ASN1_TIME **prevtm, int *preason, ASN1_OBJECT **phold,
			ASN1_GENERALIZEDTIME **pinvtm, const char *str);

#define DB_type         0
#define DB_exp_date     1
#define DB_rev_date     2
#define DB_serial       3       /* index - unique */
#define DB_file         4       
#define DB_name         5       /* index - unique when active and not disabled */
#define DB_NUMBER       6

#define DB_TYPE_REV	'R'
#define DB_TYPE_EXP	'E'
#define DB_TYPE_VAL	'V'

typedef struct db_attr_st
	{
	int unique_subject;
	} DB_ATTR;
typedef struct ca_db_st
	{
	DB_ATTR attributes;
	TXT_DB *db;
	} CA_DB;

BIGNUM *load_serial(char *serialfile, int create, ASN1_INTEGER **retai);
int save_serial(char *serialfile, char *suffix, BIGNUM *serial, ASN1_INTEGER **retai);
int rotate_serial(char *serialfile, char *new_suffix, char *old_suffix);
int rand_serial(BIGNUM *b, ASN1_INTEGER *ai);
CA_DB *load_index(char *dbfile, DB_ATTR *dbattr);
int index_index(CA_DB *db);
int save_index(const char *dbfile, const char *suffix, CA_DB *db);
int rotate_index(const char *dbfile, const char *new_suffix, const char *old_suffix);
void free_index(CA_DB *db);
int index_name_cmp(const char **a, const char **b);
int parse_yesno(const char *str, int def);

X509_NAME *parse_name(char *str, long chtype, int multirdn);
int args_verify(char ***pargs, int *pargc,
			int *badarg, BIO *err, X509_VERIFY_PARAM **pm);
void policies_print(BIO *out, X509_STORE_CTX *ctx);

#define FORMAT_UNDEF    0
#define FORMAT_ASN1     1
#define FORMAT_TEXT     2
#define FORMAT_PEM      3
#define FORMAT_NETSCAPE 4
#define FORMAT_PKCS12   5
#define FORMAT_SMIME    6
#define FORMAT_ENGINE   7
#define FORMAT_IISSGC	8	/* XXX this stupid macro helps us to avoid
				 * adding yet another param to load_*key() */

#define EXT_COPY_NONE	0
#define EXT_COPY_ADD	1
#define EXT_COPY_ALL	2

#define NETSCAPE_CERT_HDR	"certificate"

#define APP_PASS_LEN	1024

#define SERIAL_RAND_BITS	64

extern EVP_CIPHER const*enc;


#endif/*ASD_PKCS12_APP_H*/
