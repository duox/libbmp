/*bmp_os2.c*/
#include "../core/pch.h"
#include "../core/main.h"
#if !LIBBMP_CFG_NO_OS2_SUPPORT

static const struct {
	libbmp_decompress_func_t	decompress;
	libbmp_compress_func_t		compress;
} bca_to_proc[] = {
	/*[BCA_UNCOMP] =*/		{ _libbmp_decompress_none,		_libbmp_compress_none, },
	/*[BCA_RLE8] =*/		{ _libbmp_decompress_rle8,		_libbmp_compress_rle8 },
	/*[BCA_RLE4] =*/		{ _libbmp_decompress_rle4,		_libbmp_compress_rle4 },
	/*[BCA_HUFFMAN1D] =*/	{ _libbmp_decompress_huffman1d,	_libbmp_compress_huffman1d },
	/*[BCA_RLE24] =*/		{ _libbmp_decompress_rle24,		_libbmp_compress_rle24 },
};

libbmp_decompress_func_t _os2_decompression( DWORD compression )
{
	if( compression >= _countof( bca_to_proc ) )
		return nullptr;
	return bca_to_proc[compression].decompress;
}
libbmp_compress_func_t _os2_compression( DWORD compression )
{
	if( compression >= _countof( bca_to_proc ) )
		return nullptr;
	return bca_to_proc[compression].compress;
}

DWORD _os2_compression_from_id( libbmp_compression_t id, unsigned bits_per_pixel )
{
	debugbreak_if( libbmp_compression_max_count >= (unsigned) id )
		return (DWORD) -1;
	switch( id )
	{
	case libbmp_compression_none:
		return BCA_UNCOMP;
	case libbmp_compression_rle:
		if( 4 == bits_per_pixel )
			return BCA_RLE4;
		if( 8 == bits_per_pixel )
			return BCA_RLE8;
		if( 24 == bits_per_pixel )
			return BCA_RLE24;
		break;
	case libbmp_compression_huffman:
		if( 1 == bits_per_pixel )
			return BCA_HUFFMAN1D;
		break;
	default:
		break;
	}
	return (DWORD) -1;
}

#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
/*END OF bmp_os2.c*/
