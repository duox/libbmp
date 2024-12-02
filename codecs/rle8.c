/*rle8.c*/
#include "../core/pch.h"
#include "../core/main.h"

f2_status_t _libbmp_decompress_rle8( libbmp_bmp_reader_state * state )
{
	state->image_desc.compression = libbmp_compression_rle;
	return _libbmp_process_decoder( state->context->rle8_reader, state );
}
f2_status_t _libbmp_compress_rle8( libbmp_bmp_writer_state * state )
{
	return _libbmp_process_encoder( state->context->rle8_writer, state );
}

/*END OF rle8.c*/
