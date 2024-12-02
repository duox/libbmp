/*args.cpp*/
#include "pch.h"
#include "main.h"

command_proc_t	g_command;
switch_desc *	g_switch_array;

status_t process_env( const char * env_var_name )
{
	return process_command_line( getenv( env_var_name ) );
}

status_t process_config_file( const char * file_name, f2_bool_t error_if_not_exists )
{
	status_t	status;
	char *		file;
	size_t		file_size;

	status = _fload( file_name, (void **) &file, &file_size );
	if( STATUS_SUCCESS != status )
	{
		if( error_if_not_exists )
			msg_fatal_error( "cannot open file '%s'", file_name );
		return status;
	}

	/* Process the file */
	status = process_command_line( file );
	free( file );

	/* Exit */
	return status;
}

status_t parse_command_line( const char * command_line, int * argc_ptr, const char *** argv_ptr )
{
	int arg_count;
	size_t buffer_size;
	const char * psz;
	f2_bool_t new_arg = f2_true;
	char quote = 0;
	char * ptr;
	const char ** arg_ptr;
	char * arg;

	*argc_ptr = 0;
	*argv_ptr = nullptr;
	if( NULL == command_line || '\0' == *command_line )
		return STATUS_SUCCESS;

	/* Calculate number of arguments */
	for( psz = command_line; isspace( *psz ); ++ psz )
		;
	for( arg_count = 0, buffer_size = 0; '\0' != *psz; ++ psz )
	{
		/* Process argument separator */
		if( 0 == quote && isspace( *psz ) )
		{
			while( isspace( psz[1] ) )
				++ psz;
			new_arg = f2_true;
			++ buffer_size;
			continue;
		}

		/* Process start of argument */
		if( new_arg )
		{
			++ arg_count;
			new_arg = f2_false;

			if( '\"' == *psz )
			{
				quote = *psz;
				continue;
			}
		}
		else if( quote == *psz )
		{
			quote = '\0';
			continue;
		}

		++ buffer_size;
	}
	++ buffer_size;			/* ending '\0' */

	/* Allocate memory */
	ptr = (char *) malloc(
		( arg_count + 1 ) * sizeof(const char *) +
		buffer_size * sizeof(char)
		);
	if( nullptr == ptr )
		return STATUS_ERROR_INSUFFICIENT_MEMORY;

	arg_ptr = (const char * *) &ptr[0];
	arg = (char *) ( PB(ptr) + ( arg_count + 1 ) * sizeof(const char *) );

	/* Copy arguments */
	new_arg = f2_true;
	arg_count = 0;
	for( psz = command_line; isspace( *psz ); ++ psz )
		;
	for( ; '\0' != *psz; ++ psz )
	{
		/* Process argument separator */
		if( 0 == quote && isspace( *psz ) )
		{
			while( isspace( psz[1] ) )
				++ psz;
			new_arg = f2_true;
			continue;
		}

		/* Process start of argument */
		if( new_arg )
		{
			if( 0 < arg_count )
				*arg ++ = '\0';
			*arg_ptr ++ = (const char *) arg;
			new_arg = f2_false;
			++ arg_count;

			if( '\"' == *psz )
			{
				quote = *psz;
				continue;
			}
		}
		else if( quote == *psz )
		{
			quote = '\0';
			continue;
		}

		*arg ++ = *psz;
	}
	*arg ++ = '\0';

	/* Done */
	*argc_ptr = arg_count;
	*argv_ptr = (const char **) ptr;

	_heapchk();

	/* Exit */
	return STATUS_SUCCESS;
}
status_t process_command_line( const char * command_line )
{
	status_t status;
	int argc;
	const char ** argv;

	/* Parse and process command line */
	status = parse_command_line( command_line, &argc, &argv );
	if( STATUS_SUCCESS != status )
		return status;

	status = process_args( argc, argv );

	/* Done */
	free( (void *) argv );
	_heapchk();

	/* Exit */
	return status;
}

status_t process_args( int arg_count, const char * * args )
{
	int i;

	/* Process all arguments */
	for( i = 1; i < arg_count; ++ i )
	{
		/* Process argument */
		if( '-' == *args[i] /*|| '+' == *args[i] || '/' == *args[i]*/ )
			process_switch( '+'/**args[i]*/, args[i] + 1 );
		else
		{
			if( nullptr == g_command )
				process_command( args[i] );
			else
				process_filename( args[i] );
		}

		/* Check global state */
		if( g_app_flags & (SF_EXIT_REQUIRED|SF_FATAL_ERROR) )
			break;
	}

	/* Exit */
	return STATUS_SUCCESS;
}

status_t cmd_convert( const char * file_name );
status_t cmd_dump( const char * file_name );
static command_desc g_command_array[] = {
	{ cmd_convert,	swt_array_convert,	"c",	"convert",	"convert BMP files" },
	{ cmd_dump,		swt_array_dump,		"d",	"dump",		"dump BMP files"  },
	{ nullptr }
};
status_t process_command( const char * name )
{
	size_t i;
	for( i = 0; i < _countof(g_command_array) - 1; ++ i )
	{
		if( !stricmp( name, g_command_array[i].short_name ) || !stricmp( name, g_command_array[i].long_name ) )
		{
			g_command = g_command_array[i].cmd_func;
			g_switch_array = g_command_array[i].switch_array;
			return STATUS_SUCCESS;
		}
	}
	msg_fatal_error( "unrecognized command '%s'", name );
	return STATUS_ERROR_NOT_FOUND;
}

status_t process_filename( const char * filename )
{
	if( nullptr == g_command )
		return STATUS_ERROR_NOT_INITIALIZED;
	++ g_files_processed;
	return g_command( filename );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char s_help_header[] =
"Usage:\n"
"\x20\x20" SZ_APP_NAME "\x20-?\n"
"\x20\x20" SZ_APP_NAME "\40command [{switches}] {file}\n"
;
static char s_help_footer[] =
"Switchars are '-' (switch off) or '+' (switch on), e.g. +q (quiet mode on).\n"
"Switches marked with '*' are on by default.\n"
"For more information please refer to " SZ_APP_NAME ".md or to online documentation.\n"
;

void print_mask_list( void )
{
	const mask_desc * desc;

	message( "MASKS:\n" );
	for( desc = mask_array; '\0' != desc->name; ++ desc )
		message( "\40\40%c:%08x\t: %s\n", desc->name, desc->def_val, desc->desc );
}
void print_switch_list( const switch_desc * descs, const char * header )
{
	const switch_desc * desc;

	message( "\t[%s]\n", header );
	for( desc = descs; nullptr != desc->pfn; ++ desc )
	{
		message( "%s%-4s\t%s%-10s\t: %s.\n",
			nullptr != desc->short_name ? "-": "",
			nullptr != desc->short_name ? desc->short_name: "",
			nullptr != desc->long_name ?  "--" : "",
			nullptr != desc->long_name ?  desc->long_name : "",
			desc->desc );
		if( nullptr != desc->format )
			message( "\40\40\40\40%s\n", desc->format );
	}
	message( "\n", header );
}
void print_usage( void )
{
	const command_desc * cmd;

	// Write message
	message( "%s", s_help_header );

	// Print all commands
	message( "COMMANDS:\n" );
	for( cmd = g_command_array; nullptr != cmd->cmd_func; ++ cmd )
		message( "%s\t%s\t: %s.\n", cmd->short_name, cmd->long_name, cmd->desc );
	message( "\n" );

	// Print all switches
	message( "SWITCHES:\n" );
	print_switch_list( swt_array_general,	"GENERAL SWITCHES" );
	print_switch_list( swt_array_dump,		"DUMP SWITCHES" );
	print_switch_list( swt_array_convert,	"CONVERSION SWITCHES" );

	// Print additional info
	print_compression_list();
	print_format_list();
	print_mask_list();

	// Done
	message( "%s", s_help_footer );
	g_app_flags |= MF_NO_HELP_NEEDED;
}

status_t switch_help( char switchar, const char * name, unsigned param, const char * rest );
status_t switch_quiet( char switchar, const char * name, unsigned param, const char * rest );
status_t switch_verbose( char switchar, const char * name, unsigned param, const char * rest );
status_t switch_version( char switchar, const char * name, unsigned param, const char * rest );
switch_desc swt_array_general[] = {
	{ switch_help,		0,	"?",	nullptr,	"display command line help" },
	{ switch_help,		0,	"h",	"help",		"display command line help" },
	{ switch_quiet,		0,	"q",	"quiet",	"quite mode (no messages)" },
	{ switch_verbose,	0,	"v",	"verbose",	"operate verbosely" },
	{ switch_version,	0,	nullptr,"ver",		"display program version" },
	{ nullptr }
};
status_t switch_compression( char switchar, const char * name, unsigned param, const char * rest );
status_t switch_format( char switchar, const char * name, unsigned param, const char * rest );
status_t switch_mask( char switchar, const char * name, unsigned param, const char * rest );
status_t switch_no_bfh( char switchar, const char * name, unsigned param, const char * rest );
status_t switch_planes( char switchar, const char * name, unsigned param, const char * rest );
switch_desc swt_array_convert[] = {
	{ switch_compression,	0,	"c",	"convert",	"specify compression type",		"-c:compression-id" },
	{ switch_format,		0,	"f",	"format",	"specify format type",			"-f:format-id" },
	{ switch_mask,			0,	"m",	"mask",		"specify color mask",			"-m:component-id:#" },
	{ switch_no_bfh,		0,	"n",	nullptr,	"do not store file header" },
	{ switch_planes,		0,	"p",	"planes",	"specify plane count",			"-p:#" },
	{ nullptr }
};
status_t switch_dump_flags( char switchar, const char * name, unsigned param, const char * rest );
switch_desc swt_array_dump[] = {
	{ switch_dump_flags,	LIBBMP_DUMP_FILE_HEADER,	"b",	"bitmap",	"* dump bitmap header" },
	{ switch_dump_flags,	LIBBMP_DUMP_BITMAP_HEADER,	"f",	"file",		"* dump file header" },
	{ switch_dump_flags,	LIBBMP_DUMP_GAPS,			"g",	"gaps",		"dump contents of file gaps" },
	{ switch_dump_flags,	LIBBMP_DUMP_IMAGE_BITS,		"i",	"image",	"dump image bits" },
	{ switch_dump_flags,	0,							"m",	"meta",		"dump all meta-data (unsupported)" },
	{ switch_dump_flags,	LIBBMP_DUMP_PREFER_OS2,		nullptr,"os2",		"prefer OS/2 formats" },
	{ switch_dump_flags,	LIBBMP_DUMP_COLOR_TABLE,	"p",	"palette",	"dump palette contents" },
	{ nullptr }
};

int compare_switch_name( const char * str, size_t str_len, const char * switch_str, size_t * name_len )
{
	size_t switch_len;

	switch_len = strlen( switch_str );

	// Compare characters
	if( str_len < switch_len || 0 != strnicmp( str, switch_str, switch_len ) )
		return 0;

	// Check for the real end of name
	if( isalpha( str[switch_len] ) )
		return 0;

	// Exit
	*name_len = switch_len;
	return 1;
}
switch_desc * find_switch( int switchar, switch_desc * desc_array, const char * str, size_t * arg_offset )
{
	size_t i;
	size_t str_len;
	size_t best_len;
	size_t best_switch;
	switch_desc * desc = desc_array;

	if( nullptr == desc_array )
		return nullptr;

	str_len = strlen( str );
	best_len = (size_t) -1;
	best_switch = (size_t) -1;

	for( i = 0; nullptr != desc->pfn; ++ i, ++ desc )
	{
		size_t len = min(str_len, best_len);
		if( '=' == switchar && nullptr != desc->long_name && compare_switch_name( str, len, desc->long_name, &best_len ) )
			best_switch = i;
		else if( '=' != switchar && nullptr != desc->short_name && compare_switch_name( str, len, desc->short_name, &best_len ) )
			best_switch = i;
	}

	if( -1 == best_switch )
		return nullptr;

	if( ':' == str[best_len] || '=' == str[best_len] )
		++ best_len;
	*arg_offset = best_len;

	// Exit
	return desc_array + best_switch;
}
status_t process_switch( char switchar, const char * rest )
{
	status_t status;
	switch_desc * desc;
	size_t arg_offset;

	// Parse additional switchar
	if( '-' == *rest )
	{
		switchar = '=';
		++ rest;
	}

	// Test all switches
	desc = find_switch( switchar, swt_array_general, rest, &arg_offset );
	if( nullptr == desc )
	{
		if( nullptr == g_command )
		{
			msg_fatal_error( "command must precede switches" );
			return STATUS_ERROR_INVALID_STATE;
		}

		desc = find_switch( switchar, g_switch_array, rest, &arg_offset );
		if( nullptr == desc )
		{
			msg_fatal_error( "unrecognized switch \"%s\"", rest );
			return STATUS_ERROR_NOT_FOUND;
		}
	}

	// Get additional off switch
	if( '-' == *rest )
		switchar = '-';

	// Process search results
	status = (*desc->pfn)( switchar,
		nullptr == desc->long_name ? desc->short_name : desc->long_name,
		desc->param, rest + arg_offset );

	// Exit
	return status;
}
status_t warn_switch_tail( const char * name, const char * rest )
{
	if( '\0' != *rest )
		msg_warning( "unexpected characters after switch \"%s\": \"%s\"", name, rest );
	return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

status_t switch_help( char switchar, const char * name, unsigned param, const char * rest )
{
	f2_unreferenced_parameter( switchar );
	f2_unreferenced_parameter( param );
	print_usage();
	return warn_switch_tail( name, rest );
}
status_t switch_quiet( char switchar, const char * name, unsigned param, const char * rest )
{
	f2_unreferenced_parameter( param );
	if( '-' == switchar )
		g_app_flags &= ~MF_QUIET_MODE;
	else
	{
		g_app_flags &= ~MF_VERBOSE_MODE;
		g_app_flags |=	MF_QUIET_MODE;
	}
	return warn_switch_tail( name, rest );
}
status_t switch_verbose( char switchar, const char * name, unsigned param, const char * rest )
{
	f2_unreferenced_parameter( param );
	if( '-' == switchar )
		g_app_flags &= ~MF_VERBOSE_MODE;
	else
	{
		g_app_flags &= ~MF_QUIET_MODE;
		g_app_flags |=	MF_VERBOSE_MODE;
	}
	return warn_switch_tail( name, rest );
}
status_t switch_version( char switchar, const char * name, unsigned param, const char * rest )
{
	f2_unreferenced_parameter( switchar );
	f2_unreferenced_parameter( param );

	g_app_flags |= MF_HEADER_PRINTED;
	message( "%s", SZ_APP_VERSION );

	return warn_switch_tail( name, rest );
}

struct {
	libbmp_compression_t	type;
	const char * name;
	const char * description;
} compression_ids[] = {
	{ libbmp_compression_unspecified,	"auto",		"automatic mode" },
	{ libbmp_compression_none,			"none",		"no compression" },
	{ libbmp_compression_rle,			"rle",		"corresponding RLE compression" },
	{ libbmp_compression_huffman,		"huffman",	"Huffman1d compression (OS/2 2.2 format only)" },
	{ libbmp_compression_jpeg,			"jpeg",		"JPEG compression" },
	{ libbmp_compression_png,			"png",		"PNG compression" },
	{ libbmp_compression_max_count,		nullptr }
};
void print_compression_list( void )
{
	size_t i;
	message( "compression-id :=\n" );
	for( i = 0; i < _countof(compression_ids) - 1; ++ i )
		message( "  %-10s\t: %s\n", compression_ids[i].name, compression_ids[i].description );
}
status_t switch_compression( char switchar, const char * name, unsigned param, const char * rest )
{
	size_t i;

	f2_unreferenced_parameter( switchar );
	f2_unreferenced_parameter( name );
	f2_unreferenced_parameter( param );

	for( i = 0; i < _countof(compression_ids); ++ i )
	{
		if( stricmp( rest, compression_ids[i].name ) )
		{
			g_compression_type = compression_ids[i].type;
			break;
		}
	}
	if( i >= _countof(compression_ids) )
	{
		msg_error( "unrecognized compression type: \"%s\"", rest );
		print_compression_list();
	}

	return STATUS_SUCCESS;
}
struct {
	libbmp_file_format_t	type;
	const char * short_name;
	const char * long_name;
	const char * description;
} format_ids[] = {
	{ libbmp_file_format_unspecified,			"auto",		nullptr,				"automatic mode" },
	{ libbmp_file_format_BITMAPCOREHEADER,		"bch",		"bitmapcoreheader",		"use BITMAPCOREHEADER" },
	{ libbmp_file_format_BITMAPINFOHEADER,		"bih",		"bitmapinfoheader",		"use BITMAPINFOHEADER" },
	{ libbmp_file_format_BITMAPV2INFOHEADER,	"bih2",		"bitmapv2infoheader",	"use BITMAPV2INFOHEADER" },
	{ libbmp_file_format_BITMAPV3INFOHEADER,	"bih3",		"bitmapv3infoheader",	"use BITMAPV3INFOHEADER" },
	{ libbmp_file_format_BITMAPV4INFOHEADER,	"bih4",		"bitmapv4infoheader",	"use BITMAPV4INFOHEADER" },
	{ libbmp_file_format_BITMAPV5INFOHEADER,	"bih5",		"bitmapv5infoheader",	"use BITMAPV5INFOHEADER" },
	{ libbmp_file_format_OS2_BITMAPINFOHEADER1,	"os21bih1",	"os2bitmapinfoheader1",	"use BITMAPINFOHEADER1" },
	{ libbmp_file_format_OS2_BITMAPINFOHEADER2,	"os22bih2",	"os2bitmapinfoheader2",	"use BITMAPINFOHEADER2" },
	{ libbmp_file_format_max_count,				nullptr }
};
void print_format_list( void )
{
	size_t i;
	message( "format-id :=\n" );
	for( i = 0; i < _countof(format_ids) - 1; ++ i )
	{
		if( nullptr != format_ids[i].long_name )
			message( "  %-20s\t: %s\n", format_ids[i].long_name, format_ids[i].description );
		message( "  %-10s\t\t: %s\n", format_ids[i].short_name, format_ids[i].description );
	}
}
status_t switch_format( char switchar, const char * name, unsigned param, const char * rest )
{
	size_t i;

	f2_unreferenced_parameter( switchar );
	f2_unreferenced_parameter( name );
	f2_unreferenced_parameter( param );

	for( i = 0; i < _countof(format_ids); ++ i )
	{
		if( (nullptr != format_ids[i].short_name && stricmp( rest, format_ids[i].short_name )) ||
			(nullptr != format_ids[i].long_name  && stricmp( rest, format_ids[i].long_name )) )
		{
			g_format_type = format_ids[i].type;
			break;
		}
	}
	if( i >= _countof(format_ids) )
	{
		msg_error( "unrecognized format type: \"%s\"", rest );
		print_format_list();
	}

	return STATUS_SUCCESS;
}
status_t switch_mask( char switchar, const char * name, unsigned param, const char * rest )
{
	mask_desc * mask = mask_array;
	char * ep;

	// Check current state
	if( '\0' == *rest )
	{
		msg_error( "expected mask specification" );
		return STATUS_SUCCESS;
	}

	// Find mask by name
	for( ; '\0' != mask->name; ++ mask )
	{
		if( *rest == mask->name )
			break;
	}
	if( '\0' == mask->name )
	{
		msg_error( "unrecognized component type: \"%c\"", *rest );
		print_mask_list();
		return STATUS_SUCCESS;
	}
	++ rest;

	// Parse mask
	if( ':' != *rest || '=' != *rest )
	{
		msg_error( "expected separator betweem component name and mask value" );
		return STATUS_SUCCESS;
	}
	if( !stricmp( rest, "auto" ) )
		mask->cur_val = mask->def_val;
	else
	{
		unsigned long mask_val = strtoul( rest, &ep, 0 );
		if( ep > rest )	// some value was parsed
			mask->cur_val = mask_val;
	}

	// Exit
	f2_unreferenced_parameter( switchar );
	f2_unreferenced_parameter( param );
	return warn_switch_tail( name, rest );
}
status_t switch_no_bfh( char switchar, const char * name, unsigned param, const char * rest )
{
	f2_unreferenced_parameter( name );
	f2_unreferenced_parameter( param );
	if( '-' == switchar )
		g_app_flags &= ~CF_NO_BFH;
	else
		g_app_flags |= CF_NO_BFH;
	return warn_switch_tail( name, rest );
}
status_t switch_planes( char switchar, const char * name, unsigned param, const char * rest )
{
	unsigned long planes;
	char * ep;

	f2_unreferenced_parameter( switchar );
	f2_unreferenced_parameter( param );

	planes = strtoul( rest, &ep, 0 );
	if( ep > rest )	// some value was parsed
		g_plane_count = planes;

	return warn_switch_tail( name, ep );
}

status_t switch_dump_flags( char switchar, const char * name, unsigned param, const char * rest )
{
	if( '-' == switchar )
		g_dump_flags &= ~param;
	else
		g_dump_flags |= param;
	return warn_switch_tail( name, rest );
}

/*END OF args.c*/
