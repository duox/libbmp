/*huffman1d.c*/
#include "../core/pch.h"
#include "../core/main.h"

f2_status_t _libbmp_decompress_huffman1d( libbmp_bmp_reader_state * state )
{
	state->image_desc.compression = libbmp_compression_huffman;
	return _libbmp_process_decoder( state->context->huffman1d_reader, state );
}
f2_status_t _libbmp_compress_huffman1d( libbmp_bmp_writer_state * state )
{
	return _libbmp_process_encoder( state->context->huffman1d_writer, state );
}

/*END OF huffman1d.c*/
