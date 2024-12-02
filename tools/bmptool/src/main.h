/*main.h*/

#ifdef _DEV_PCH
# include <libbmp.h>
#endif // def _DEV_PCH

#define SZ_APP_NAME			"bmptool"
#define SZ_APP_LONG_NAME	"bmptool"
#define SZ_APP_VERSION		"1.00.001.alpha"

typedef enum status_t {
	STATUS_SUCCESS,
	STATUS_ERROR_READING,
	STATUS_ERROR_INSUFFICIENT_MEMORY,
	STATUS_ERROR_INVALID_PARAMETER,
	STATUS_ERROR_NOT_FOUND,
	STATUS_ERROR_NOT_INITIALIZED,
	STATUS_ERROR_INVALID_STATE,
} status_t;

typedef status_t (*switch_proc_t)( char switchar, const char * name, unsigned param, const char * rest );
typedef struct switch_desc
{
	switch_proc_t	pfn;
	unsigned		param;
	const char *	short_name;
	const char *	long_name;
	const char *	desc;
	const char *	format;
} switch_desc;
typedef status_t (*command_proc_t)( const char * file_name );
typedef struct command_desc {
	command_proc_t	cmd_func;
	switch_desc *	switch_array;
	const char *	short_name;
	const char *	long_name;
	const char *	desc;
} command_desc;
typedef struct mask_desc {
	char		name;
	unsigned	def_val;
	unsigned	cur_val;
	const char *desc;
} mask_desc;

status_t process_env( const char * env_var_name );
status_t process_config_file( const char * file_name, f2_bool_t error_if_not_exists );
status_t process_command_line( const char * command_line );
status_t process_args( int arg_count, const char * * args );
status_t process_switch( char switchar, const char * rest );
status_t process_command( const char * name );
status_t process_filename( const char * filename );

extern command_proc_t	g_command;
extern switch_desc *	g_switch_array;
extern switch_desc swt_array_general[];
extern switch_desc swt_array_convert[];
extern switch_desc swt_array_dump[];

extern mask_desc mask_array[];

long _ffilelength( FILE * f );
status_t _fload( const char * fname, void ** data_ptr, size_t * data_size );

#define SF_EXIT_REQUIRED	0x0001
#define SF_FATAL_ERROR		0x0002
#define MF_HEADER_PRINTED	0x0004
#define MF_NO_HELP_NEEDED	0x0008
#define MF_QUIET_MODE		0x0010
#define MF_VERBOSE_MODE		0x0020
#define CF_NO_BFH			0x0040
extern unsigned g_app_flags;
extern unsigned g_files_processed;
extern unsigned g_error_count;
extern unsigned g_warning_count;

extern libbmp_compression_t g_compression_type;
extern libbmp_file_format_t g_format_type;
extern unsigned g_plane_count;

extern unsigned g_dump_flags;	// libbmp LIBBMP_DUMP_* flags

void msg_fatal_error( const char * format, ... );
void msg_error( const char * format, ... );
void msg_warning( const char * format, ... );
void msg_info( const char * format, ... );
void message( const char * format, ... );
void messageAP( const char * format, va_list ap );

void print_usage( void );
void print_compression_list( void );
void print_format_list( void );
void print_mask_list( void );

/*END OF main.h*/
