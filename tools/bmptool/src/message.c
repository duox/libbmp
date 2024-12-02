/*message.c*/
#include "pch.h"
#include "main.h"


static void print_utility_header()
{
	if( g_app_flags & MF_HEADER_PRINTED )
		return;
	g_app_flags |= MF_HEADER_PRINTED;

	printf( "%s", SZ_APP_LONG_NAME " version " SZ_APP_VERSION ". Copyright (c) 2012 StoneGlass.\n" );

	return;
}

static void print_message( const char * headers, const char * format, va_list ap )
{
	print_utility_header();

	if( nullptr != headers && '\0' != *headers )
		printf( "%s: ", headers );

	vprintf( format, ap );
	printf( ".\n" );

	return;
}

/****************************************************************************************************************************************************/

void msg_fatal_error( const char * format, ... )
{
	va_list ap;
	va_start( ap, format );
	print_message( "*** Fatal error", format, ap );
	va_end( ap );

	g_app_flags |= SF_FATAL_ERROR|MF_NO_HELP_NEEDED;

	return;
}

void msg_error( const char * format, ... )
{
	va_list ap;
	va_start( ap, format );
	print_message( "** Error", format, ap );
	va_end( ap );

	++ g_error_count;

	g_app_flags |= MF_NO_HELP_NEEDED;

	return;
}

void msg_warning( const char * format, ... )
{
	va_list ap;
	va_start( ap, format );
	print_message( "* Warning", format, ap );
	va_end( ap );

	++ g_warning_count;

	g_app_flags |= MF_NO_HELP_NEEDED;

	return;
}

void msg_info( const char * format, ... )
{
	va_list ap;

	va_start( ap, format );
	print_message( "Info", format, ap );
	va_end( ap );

	return;
}

void message( const char * format, ... )
{
	va_list ap;

	print_utility_header();

	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );

	return;
}
void messageAP( const char * format, va_list ap )
{
	print_utility_header();
	vprintf( format, ap );
	return;
}


/*END OF message.c*/
