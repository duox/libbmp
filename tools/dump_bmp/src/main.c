/*main.c*/
#include "pch.h"
#include "main.h"

static libbmp_dump_context	g_dump_context;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct dumpapp_msgout {
	libbmp_msgout		msgout;
	unsigned	ident;
} dumpapp_msgout;
static dumpapp_msgout		g_msgout;

static libbmp_status_t msgout_ident( libbmp_msgout * thisp, int ident )
{
	dumpapp_msgout * ctx = (dumpapp_msgout *) thisp;

	signed new_ident = (signed) ctx->ident + ident;
	libbmp_assert( new_ident >= 0 );
	if( new_ident < 0 )
		return LIBBMP_STATUS_ERROR_INVALID_PARAMETER;
	ctx->ident = (unsigned) new_ident;

	return LIBBMP_STATUS_SUCCESS;
}
static libbmp_status_t msgout_print_header( libbmp_msgout * thisp, const char * text )
{
	dumpapp_msgout * ctx = (dumpapp_msgout *) thisp;

	printf( "%*c", ctx->ident*2, 0x20 );
	puts( text );

	return LIBBMP_STATUS_SUCCESS;
}
static libbmp_status_t msgout_print( libbmp_msgout * thisp, const char * tag, const char * value, size_t len, const char * desc )
{
	dumpapp_msgout * ctx = (dumpapp_msgout *) thisp;
	char buf[1024] = "";

	// Generate string
	if( NULL != tag )
	{
		strcat( buf, tag );
		strcat( buf, " : " );
	}

	if( NULL != value && 0 != len )
		strncat( buf, value, len );

	if( NULL != desc )
	{
		strcat( buf, " : " );
		strcat( buf, desc );
	}

	// Display string
	printf( "%*c", ctx->ident*2, 0x20 );
	puts( buf );

	// Exit
	return LIBBMP_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int load( const char * name, void ** buf_ptr, size_t * size_ptr )
{
	int h;
	long size;
	void * buf;
	int nread;

	h = _open( name, _O_RDONLY|_O_BINARY );
	if( -1 == h )
	{
		printf( "*** error: file was not found: %s\n", name );
		return -1;
	}

	size = _filelength( h );
	if( -1 == size )
	{
		printf( "*** error: error querying file size: %s\n", name );
		_close( h );
		return -1;
	}

	buf = malloc( size + 1 );
	if( NULL == buf )
	{
		printf( "*** error: cannot allocate %d bytes: %s\n", size, name );
		_close( h );
		return -1;
	}

	nread = read( h, buf, size );
	_close( h );
	if( size != nread )
	{
		printf( "*** error: error reading: %s\n", name );
		return -1;
	}

	*buf_ptr = buf;
	*size_ptr = size;

	return 0;
}

void dump_file( char * fname )
{
	void * data;
	size_t data_size;
	libbmp_status_t status;

	if( 0 != load( fname, &data, &data_size ) )
		return;

	g_dump_context.data = data;
	g_dump_context.data_size = data_size;
	status = libbmp_dump_bmp( &g_dump_context );

	if( libbmp_failed( status ) )
		printf( "*** libbmp error: %s\n", libbmp_get_error_string( status ) );

	free( data );

	_CrtDumpMemoryLeaks();

	return;
}

static const char * help_string =
"Usage: dump_bmp { [{-switch}] bmp_file }\n"
"       dump_bmp -?\n"
"Switches:\n"
" ?\t: display command line help.\n"
" -\t: all next arguments are file names.\n"
" data\t: display bitmap bits as raw data (implies -pixels-).\n"
" header\t: *display file header.\n"
" os2\t: prefer OS/2 naming if possible.\n"
" pal\t: display color table.\n"
" pixels\t: display color table as pixels (implies -data-).\n"
"Switches marked with '*' are on by default.\n"
"To invert switch action, use '-' after switch name e.g. dump_bmp -header- file.bmp"
;
int main( int argc, char * argv[] )
{
	int i;
	int filenames_only = 0;

	puts( "dump_bmp v.1.0.01\n");
	if( 1 >= argc )
	{
		puts( help_string );
		return 1;
	}

	g_msgout.msgout.ident = msgout_ident;
	g_msgout.msgout.print_header = msgout_print_header;
	g_msgout.msgout.print = msgout_print;
	g_dump_context.msgout = &g_msgout.msgout;
	g_dump_context.flags = LIBBMP_DUMP_FILE_HEADER | LIBBMP_DUMP_BITMAP_HEADER;

	for( i = 1; i < argc; ++ i )
	{
		if( !filenames_only && '-' == *argv[i] )
		{
			if( !strcmp( argv[i] + 1, "?" ) )
				puts( help_string );
			else if( !strcmp( argv[i] + 1, "-" ) )
				filenames_only = 1;

			else if( !strcmp( argv[i] + 1, "header" ) )
				g_dump_context.flags |= LIBBMP_DUMP_FILE_HEADER | LIBBMP_DUMP_BITMAP_HEADER;
			else if( !strcmp( argv[i] + 1, "header-" ) )
				g_dump_context.flags &= ~(LIBBMP_DUMP_FILE_HEADER | LIBBMP_DUMP_BITMAP_HEADER);

			else if( !strcmp( argv[i] + 1, "pal" ) )
				g_dump_context.flags |= LIBBMP_DUMP_COLOR_TABLE;
			else if( !strcmp( argv[i] + 1, "pal-" ) )
				g_dump_context.flags &= ~LIBBMP_DUMP_COLOR_TABLE;

			else if( !strcmp( argv[i] + 1, "data" ) )
				g_dump_context.flags = (g_dump_context.flags | LIBBMP_DUMP_IMAGE_BITS) & ~LIBBMP_DUMP_PIXELS;
			else if( !strcmp( argv[i] + 1, "data-" ) )
				g_dump_context.flags &= ~(LIBBMP_DUMP_IMAGE_BITS | LIBBMP_DUMP_PIXELS);

			else if( !strcmp( argv[i] + 1, "pixels" ) )
				g_dump_context.flags |= LIBBMP_DUMP_IMAGE_BITS | LIBBMP_DUMP_PIXELS;
			else if( !strcmp( argv[i] + 1, "pixels-" ) )
				g_dump_context.flags &= ~(LIBBMP_DUMP_IMAGE_BITS | LIBBMP_DUMP_PIXELS);

			else if( !strcmp( argv[i] + 1, "os2" ) )
				g_dump_context.flags |= LIBBMP_DUMP_PREFER_OS2;
			else if( !strcmp( argv[i] + 1, "os2-" ) )
				g_dump_context.flags &= ~LIBBMP_DUMP_PREFER_OS2;

			else if( '\0' == argv[i][1] )
				printf( "arg #%d: expected switch name", i - 1 );
			else
				printf( "unrecognized switch: %s", argv[i] );
		}
		else
		{
			printf( "=== %s\n", argv[i] );
			dump_file( argv[i] );
		}
	}

	return 0;
}

/*END OF bitmap.c*/
