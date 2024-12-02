/*main.c*/
#include "pch.h"
#include "main.h"

#ifdef _MSC_VER
# pragma warning(disable:4206)	//: nonstandard extension used : translation unit is empty
#endif // def _MSC_VER

/**
 * @brief Helper function for easying diagnostic output handling.
 */
f2_status_t	libbmp_api libbmp_diagnostics(
	libbmp_diag * diag, f2_status_t status, libbmp_diag_message_t msg, size_t offset, size_t param )
{
	if( nullptr == diag )
		return status;
	debugbreak_if( nullptr == diag->message )
		return status;
	debugbreak_if( (unsigned) msg >= libbmp_diag_message_count )
		return status;
	if( 0 == (diag->reporting_mask & (1 << msg )) )
		return status;

	diag->message( diag, msg, offset, param );

	return status;
}

f2_bool_t libbmp_api libbmp_query_metadata_bool( libbmp_imetadata * thisp, libbmp_metadata_t id )
{
	f2_bool_t buffer = f2_false;
	thisp->query_metadata( thisp, id, libbmp_type_bool, &buffer, sizeof(buffer) );
	return buffer;
}

/*END OF main.c*/
