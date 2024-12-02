/*png.c*/
#include "../core/pch.h"
#include "../core/main.h"

f2_status_t _libbmp_decompress_png( libbmp_bmp_reader_state * state )
{
	state->image_desc.compression = libbmp_compression_png;
	return _libbmp_process_decoder( state->context->png_reader, state );
}
f2_status_t _libbmp_compress_png( libbmp_bmp_writer_state * state )
{
	return _libbmp_process_encoder( state->context->png_writer, state );
}

/*END OF png.c*/
