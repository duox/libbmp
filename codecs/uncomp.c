/*uncomp.c*/
#include "../core/pch.h"
#include "../core/main.h"

f2_status_t _libbmp_decompress_none( libbmp_bmp_reader_state * state )
{
	libbmp_copy_context	copy_context;

	state->image_desc.compression = libbmp_compression_none;

	// Read the image and exit
	copy_context.in_buf = PCB(state->context->data) + state->file_header->bfOffBits;
	copy_context.in_pitch = state->image_desc.pitch;
	copy_context.out_buf = state->dst_buf;
	copy_context.out_pitch = state->dst_pitch;
	copy_context.row_count = state->image_desc.height;

	return _libbmp_copy_image( &copy_context );
}
f2_status_t _libbmp_compress_none( libbmp_bmp_writer_state * state )
{
	libbmp_copy_context	copy_context;

	// Read the image and exit
	copy_context.in_buf = PB(state->context->image_data);
	copy_context.in_pitch = state->context->image_desc.pitch;
	copy_context.out_buf = state->ptr;
	copy_context.out_pitch = state->pitch;
	copy_context.row_count = state->context->image_desc.height;

	return _libbmp_copy_image( &copy_context );
}

/*END OF uncomp.c*/
