/*detect.c*/
#include "pch.h"
#include "main.h"

f2_status_t libbmp_detect( const void * data, size_t data_size, libbmp_image_descriptor * desc )
{
	f2_status_t		status;
	libbmp_bmp_reader_context	context;
	libbmp_bmp_reader_state		state;

	memset( &context, 0, sizeof(context) );
	context.data = data;
	context.data_size = data_size;

	memset( &state, 0, sizeof(state) );
	state.context = &context;

	status = _libbmp_read_header( &state );
	if( f2_failed(status) )
		return status;

	if( nullptr != desc )
		*desc = state.image_desc;

	return F2_STATUS_SUCCESS;
}

/*END OF detect.c*/
