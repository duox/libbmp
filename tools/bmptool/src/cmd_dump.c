/*cmd_dump.c*/
#include "pch.h"
#include "main.h"

static libbmp_dump_context	g_dump_context;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct dumpapp_msgout {
	f2_msgout		msgout;
	unsigned	ident;
} dumpapp_msgout;
static dumpapp_msgout		g_msgout;

static f2_status_t msgout_ident( f2_msgout * thisp, int ident )
{
	dumpapp_msgout * ctx = (dumpapp_msgout *) thisp;

	signed new_ident = (signed) ctx->ident + ident;
	f2_assert( new_ident >= 0 );
	if( new_ident < 0 )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	ctx->ident = (unsigned) new_ident;

	return F2_STATUS_SUCCESS;
}
static f2_status_t msgout_print_header( f2_msgout * thisp, const char * text )
{
	dumpapp_msgout * ctx = (dumpapp_msgout *) thisp;

	printf( "%*c", ctx->ident*2, 0x20 );
	puts( text );

	return F2_STATUS_SUCCESS;
}
static f2_status_t msgout_print( f2_msgout * thisp, const char * tag, const char * value, size_t len, const char * desc )
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
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

status_t cmd_dump( const char * file_name )
{
	void * data;
	size_t data_size;
	f2_status_t status;

	if( 0 != _fload( file_name, &data, &data_size ) )
		return STATUS_ERROR_NOT_FOUND;

	g_msgout.msgout.ident = msgout_ident;
	g_msgout.msgout.print_header = msgout_print_header;
	g_msgout.msgout.print = msgout_print;
	g_dump_context.msgout = &g_msgout.msgout;
	g_dump_context.flags = g_dump_flags;
	g_dump_context.data = data;
	g_dump_context.data_size = data_size;
	status = libbmp_dump_bmp( &g_dump_context );
	if( f2_failed( status ) )
		message( "libbmp error: %s\n", f2_get_error_string( status ) );

	free( data );

	return STATUS_SUCCESS;
}

/*END OF cmd_dump.c*/
