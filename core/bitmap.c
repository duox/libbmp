/*bitmap.c*/
#include "pch.h"
#include "main.h"

f2_status_t _libbmp_copy_image( const libbmp_copy_context * context )
{
	const unsigned char * src = context->in_buf;
	unsigned char * dst = ((unsigned char *) context->out_buf) + context->out_pitch * context->row_count;
	unsigned y;	// one-base index of rows

	// Read data line by line, from bottom to top
	for( y = context->row_count; y > 0; -- y )
	{
		dst -= context->out_pitch;
		memcpy( dst, src, context->in_pitch );
		src += context->in_pitch;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t _libbmp_process_decoder( libbmp_encoder * encoder, libbmp_bmp_reader_state * state )
{
	f2_status_t status;
	size_t nbytes = 0;
	size_t dst_buf_size;

	// Check current state
	debugbreak_if( nullptr == encoder )
		return F2_STATUS_ERROR_NOT_SUPPORTED;

	// Process input buffer
	dst_buf_size = state->dst_pitch * state->image_desc.height;
	status = encoder->process( encoder,
		state->dst_buf, dst_buf_size,
		state->ptr, state->size_left,
		&state->image_desc,
		&nbytes
		);
	if( nbytes != dst_buf_size )	// exactly spedifid number of bytes must be decoded
		return F2_STATUS_ERROR_INVALID_DATA;

	// Exit
	return status;
}
f2_status_t _libbmp_process_encoder( libbmp_encoder * encoder, libbmp_bmp_writer_state * state )
{
	f2_status_t status;
	size_t nbytes = 0;

	// Check current state
	debugbreak_if( nullptr == encoder )
		return F2_STATUS_ERROR_NOT_SUPPORTED;

	// Process input buffer
	status = encoder->process( encoder,
		state->buffer, state->ptr - PB(state->buffer),
		state->context->image_data, state->context->image_data_size,
		&state->context->image_desc,
		&nbytes
		);
	state->ptr += nbytes;

	// Exit
	return status;
}

/*END OF bitmap.c*/
