/*jpeg.c*/
#include "../core/pch.h"
#include "../core/main.h"

f2_status_t _libbmp_decompress_jpeg( libbmp_bmp_reader_state * state )
{
	state->image_desc.compression = libbmp_compression_jpeg;
	return _libbmp_process_decoder( state->context->jpeg_reader, state );
}
f2_status_t _libbmp_compress_jpeg( libbmp_bmp_writer_state * state )
{
	return _libbmp_process_encoder( state->context->jpeg_writer, state );
}

/*END OF jpeg.c*/
