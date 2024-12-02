/*bmp_win.c*/
#include "../core/pch.h"
#include "../core/main.h"

static const struct {
	libbmp_decompress_func_t	decompress;
	libbmp_compress_func_t		compress;
} bi_to_proc[] = {
	/*[BI_RGB] =*/			{ _libbmp_decompress_none,				_libbmp_compress_none },
	/*[BCA_RLE8] =*/		{ _libbmp_decompress_rle8,				_libbmp_compress_rle8 },
	/*[BCA_RLE4] =*/		{ _libbmp_decompress_rle4,				_libbmp_compress_rle4 },
	/*[BI_BITFIELDS] =*/	{ _libbmp_decompress_bitfields,			_libbmp_compress_bitfields },
	/*[BI_JPEG] =*/			{ _libbmp_decompress_jpeg,				_libbmp_compress_jpeg },
	/*[BI_PNG] =*/			{ _libbmp_decompress_png,				_libbmp_compress_png },
	/*[BI_ALPHABITFIELDS]=*/{ _libbmp_decompress_alpha_bitfields,	_libbmp_compress_alpha_bitfields },
};

libbmp_decompress_func_t _win_decompression( DWORD compression )
{
	if( compression >= _countof( bi_to_proc ) )
		return nullptr;
	return bi_to_proc[compression].decompress;
}
libbmp_compress_func_t _win_compression( DWORD compression )
{
	if( compression >= _countof( bi_to_proc ) )
		return nullptr;
	return bi_to_proc[compression].compress;
}

DWORD _win_compression_from_id( libbmp_compression_t id, unsigned bits_per_pixel )
{
	debugbreak_if( libbmp_compression_max_count >= (unsigned) id )
		return (DWORD) -1;
	switch( id )
	{
	case libbmp_compression_none:
		return BI_RGB;
	case libbmp_compression_rle:
		if( 4 == bits_per_pixel )
			return BI_RLE4;
		if( 8 == bits_per_pixel )
			return BI_RLE8;
		break;
	case libbmp_compression_jpeg:
		if( 24 == bits_per_pixel )
			return BI_JPEG;
		break;
	case libbmp_compression_png:
		if( 24 == bits_per_pixel || 32 == bits_per_pixel )
			return BI_PNG;
		break;
	default:
		break;
	}
	return (DWORD) -1;
}

/*END OF bmp_win.c*/
