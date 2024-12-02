/*diag.c*/
#include "pch.h"
#include "main.h"

const char * libbmp_api libbmp_get_error_string( f2_status_t status )
{
	static const char * g_msg_array[] = {
		"no error",
		"invalid parameter given",
		"client structures are in invalid state",
		"image file format is unknown",
		"image compression is unknown",
		"format recognized, but data is invalid",
		"mark code to implement",
		"one of the client's interfaces behaves incorrectly",
		"function not supported",
		"insufficient space in the buffer",
	};
	if( (unsigned) status >= _countof(g_msg_array) )
		return "<invalid status code>";
	return g_msg_array[status];
}

/*END OF diag.c*/
