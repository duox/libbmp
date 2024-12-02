/*main.c*/
#include "pch.h"
#include "main.h"

unsigned g_app_flags;
unsigned g_dump_flags = LIBBMP_DUMP_FILE_HEADER | LIBBMP_DUMP_BITMAP_HEADER;
unsigned g_files_processed;
unsigned g_error_count;
unsigned g_warning_count;

libbmp_compression_t g_compression_type = libbmp_compression_unspecified;
libbmp_file_format_t g_format_type = libbmp_file_format_unspecified;
unsigned g_plane_count = 1;

mask_desc mask_array[] = {
	{ 'R',	0x00FF0000,	0x00FF0000, "red component" },
	{ 'G',	0x0000FF00,	0x0000FF00, "green component" },
	{ 'B',	0x000000FF,	0x000000FF, "blue component" },
	{ 'A',	0xFF000000,	0xFF000000, "alpha component" },
	{ 'X',	0xFF000000,	0xFF000000, "unused space" },
	{ '\0' }
};

int main( int argc, const char ** argv )
{
	process_env( "BMPTOOL" );
	process_config_file("bmptool.cfg", f2_false );
	process_args( argc, argv );

	if( 0 == g_files_processed && !(g_app_flags & MF_NO_HELP_NEEDED) )
	{
		msg_fatal_error( "expected filename" );
		print_usage();
	}

#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#endif // def _MSC_VER
	return 0;
}

/*END OF main.c*/
