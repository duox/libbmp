/*write.c*/
#include "pch.h"
#include "main.h"

static f2_status_t _libbmp_write_BITMAPFILEHEADER( libbmp_bmp_writer_state * state, WORD signature )
{
	BITMAPFILEHEADER * bfh = state->file_header;

	bfh->bfType = signature;
	bfh->bfSize = 0;			// will be filled in the end of libbmp_write
	bfh->bfReserved1 = 0;
	bfh->bfReserved2 = 0;
	bfh->bfOffBits = 0;			// will be filled in _libbmp_write_data

	return F2_STATUS_SUCCESS;
}

#if !LIBBMP_CFG_NO_LEGACY_SUPPORT

static f2_status_t _libbmp_write_BITMAPCOREHEADER( libbmp_bmp_writer_state * state )
{
	BITMAPCOREHEADER * bch = &state->bitmap_header->BitmapCoreHeader;

	debugbreak_if( state->context->image_desc.width > 0xFFFF )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( state->context->image_desc.height > 0xFFFF )
		return F2_STATUS_ERROR_INVALID_STATE;

	_libbmp_write_BITMAPFILEHEADER( state, LIBBMP_FILETYPE_BITMAP );

	bch->bcSize = sizeof(BITMAPCOREHEADER);
	bch->bcWidth = (WORD) state->context->image_desc.width;
	bch->bcHeight = (WORD) state->context->image_desc.height;
	bch->bcPlanes = 1;	// TODO: state->context->image_desc.planes;
	bch->bcBitCount = (WORD) state->context->image_desc.pixel_format.bits_per_pixel;

	return F2_STATUS_SUCCESS;
}

#endif // !LIBBMP_CFG_NO_LEGACY_SUPPORT

static f2_status_t _libbmp_write_BITMAPINFOHEADER( libbmp_bmp_writer_state * state )
{
	BITMAPINFOHEADER * bih = &state->bitmap_header->WinBitmapHeader;
	const libbmp_image_descriptor * img = &state->context->image_desc;

	_libbmp_write_BITMAPFILEHEADER( state, LIBBMP_FILETYPE_BITMAP );

	bih->biSize = sizeof(BITMAPINFOHEADER);
	bih->biWidth = img->width;
	bih->biHeight = img->flags & LIBBMP_IMAGE_UPSIDEDOWN ? img->height : - (LONG) img->height;
	bih->biPlanes = 1;	// TODO: img->planes;
	bih->biBitCount = (WORD) img->pixel_format.bits_per_pixel;
	bih->biCompression = state->compression;
	bih->biSizeImage = 0;
	bih->biXPelsPerMeter = 2835;		// 72 DPI (72 DPI x 39.3701 inches per metre yields 2834.6472)
	bih->biYPelsPerMeter = 2835;		// 72 DPI (72 DPI x 39.3701 inches per metre yields 2834.6472)
	bih->biClrUsed = 0;					// will be filled by _libbmp_write_color_table
	bih->biClrImportant = 0;

	return F2_STATUS_SUCCESS;
}
static f2_status_t _libbmp_write_BITMAPV2INFOHEADER( libbmp_bmp_writer_state * state )
{
	f2_status_t status;
	BITMAPV2INFOHEADER * bih = &state->bitmap_header->WinBitmapV2Header;
	const libbmp_image_descriptor * img = &state->context->image_desc;

	f2_clear_mem( bih, sizeof(BITMAPV2INFOHEADER) );
	status = _libbmp_write_BITMAPINFOHEADER( state );
	if( f2_failed( status ) )
		return status;

	bih->biV2Size		= sizeof(BITMAPV2INFOHEADER);
	bih->biV2RedMask	= img->pixel_format.r_mask;
	bih->biV2GreenMask	= img->pixel_format.g_mask;
	bih->biV2BlueMask	= img->pixel_format.b_mask;

	return F2_STATUS_SUCCESS;
}
static f2_status_t _libbmp_write_BITMAPV3INFOHEADER( libbmp_bmp_writer_state * state )
{
	f2_status_t status;
	BITMAPV3INFOHEADER * bih = &state->bitmap_header->WinBitmapV3Header;
	const libbmp_image_descriptor * img = &state->context->image_desc;

	f2_clear_mem( bih, sizeof(BITMAPV3INFOHEADER) );
	status = _libbmp_write_BITMAPV2INFOHEADER( state );
	if( f2_failed( status ) )
		return status;

	bih->biV3Size		= sizeof(BITMAPV3INFOHEADER);
	bih->biV3AlphaMask	= img->pixel_format.a_mask;

	return F2_STATUS_SUCCESS;
}
static f2_status_t _libbmp_write_BITMAPV4INFOHEADER( libbmp_bmp_writer_state * state )
{
	f2_status_t status;
	BITMAPV4INFOHEADER * bih = &state->bitmap_header->WinBitmapV4Header;
	const libbmp_image_descriptor * img = &state->context->image_desc;

	f2_clear_mem( bih, sizeof(BITMAPV3INFOHEADER) );
	status = _libbmp_write_BITMAPV3INFOHEADER( state );
	if( f2_failed( status ) )
		return status;

	bih->biV4Size	= sizeof(BITMAPV4INFOHEADER);

	if( nullptr == state->context->metadata || state->context->metadata->query_metadata )
		bih->biV4CSType = LCS_WINDOWS_COLOR_SPACE;
	else {
		if( libbmp_query_metadata_bool( state->context->metadata, libbmp_metadata_colorspace_sRGB ) )
			bih->biV4CSType = LCS_sRGB;
		else if( libbmp_query_metadata_bool( state->context->metadata, libbmp_metadata_colorspace_calibrated_RGB ) )
			bih->biV4CSType = LCS_CALIBRATED_RGB;
		else if( libbmp_query_metadata_bool( state->context->metadata, libbmp_metadata_colorspace_windows ) )
			bih->biV4CSType = LCS_WINDOWS_COLOR_SPACE;
	}

	f2_unreferenced_local_variable( img );	// TODO
	return F2_STATUS_SUCCESS;
}
static f2_status_t _libbmp_write_BITMAPV5INFOHEADER( libbmp_bmp_writer_state * state )
{
	f2_status_t status;
	BITMAPV5INFOHEADER * bih = &state->bitmap_header->WinBitmapV5Header;
	const libbmp_image_descriptor * img = &state->context->image_desc;

	f2_clear_mem( bih, sizeof(BITMAPV5INFOHEADER) );
	status = _libbmp_write_BITMAPV4INFOHEADER( state );
	if( f2_failed( status ) )
		return status;

	bih->biV5Size		= sizeof(BITMAPV5INFOHEADER);

	f2_unreferenced_local_variable( img );	// TODO
	return F2_STATUS_SUCCESS;
}

#if !LIBBMP_CFG_NO_OS2_SUPPORT

static f2_status_t _libbmp_write_OS2_BITMAPINFOHEADER1( libbmp_bmp_writer_state * state )
{
	OS21XBITMAPHEADER * bih = &state->bitmap_header->Os21BitmapHeader;

	debugbreak_if( state->context->image_desc.width > 0xFFFF )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( state->context->image_desc.height > 0xFFFF )
		return F2_STATUS_ERROR_INVALID_STATE;

	_libbmp_write_BITMAPFILEHEADER( state, LIBBMP_FILETYPE_BITMAP );

	bih->Size = sizeof(OS21XBITMAPHEADER);
	bih->Width = (WORD) state->context->image_desc.width;
	bih->Height = (WORD) state->context->image_desc.height;
	bih->NumPlanes = 1;	// TODO: state->context->image_desc.planes;
	bih->BitsPerPixel = (WORD) state->context->image_desc.pixel_format.bits_per_pixel;

	return F2_STATUS_SUCCESS;
}
static f2_status_t _libbmp_write_OS2_BITMAPINFOHEADER2( libbmp_bmp_writer_state * state )
{
	OS22XBITMAPHEADER * bih = &state->bitmap_header->Os22BitmapHeader;

	_libbmp_write_BITMAPFILEHEADER( state, LIBBMP_FILETYPE_BITMAP );

	bih->cbFix = sizeof(OS22XBITMAPHEADER);
	bih->cx = state->context->image_desc.width;
	bih->cy = state->context->image_desc.height;
	bih->cPlanes = 1;	// TODO: state->context->image_desc.planes;
	bih->cBitCount = (USHORT) state->context->image_desc.pixel_format.bits_per_pixel;

	bih->ulCompression = state->compression;
	bih->cbImage = 0;
	bih->cxResolution = 2835;		// 72 DPI (72 DPI x 39.3701 inches per metre yields 2834.6472)
	bih->cyResolution = 2835;		// 72 DPI (72 DPI x 39.3701 inches per metre yields 2834.6472)
	bih->cclrUsed = 0;				// will be filled by _libbmp_write_color_table
	bih->cclrImportant = 0;
	bih->usUnits = 0;
	bih->usReserved = 0;
	bih->usRecording = 0;
	bih->usRendering = 0;
	bih->cSize1 = 0;
	bih->cSize2 = 0;
	bih->ulColorEncoding = 0;
	bih->ulIdentifier = 0;

	return F2_STATUS_SUCCESS;
}

#endif // !LIBBMP_CFG_NO_OS2_SUPPORT

static f2_status_t (*const _libbmp_write_func[])( libbmp_bmp_writer_state * state ) = {
#if !LIBBMP_CFG_NO_LEGACY_SUPPORT
	/*[libbmp_file_format_BITMAPCOREHEADER] = */		_libbmp_write_BITMAPCOREHEADER,
#else
	/*[libbmp_file_format_BITMAPCOREHEADER] = */		nullptr,
#endif // !LIBBMP_CFG_NO_LEGACY_SUPPORT
	/*[libbmp_file_format_BITMAPINFOHEADER] = */		_libbmp_write_BITMAPINFOHEADER,
	/*[libbmp_file_format_BITMAPV2INFOHEADER] = */		_libbmp_write_BITMAPV2INFOHEADER,
	/*[libbmp_file_format_BITMAPV3INFOHEADER] = */		_libbmp_write_BITMAPV3INFOHEADER,
	/*[libbmp_file_format_BITMAPV4INFOHEADER] = */		_libbmp_write_BITMAPV4INFOHEADER,
	/*[libbmp_file_format_BITMAPV5INFOHEADER] = */		_libbmp_write_BITMAPV5INFOHEADER,
#if !LIBBMP_CFG_NO_OS2_SUPPORT
	/*[libbmp_file_format_OS2_BITMAPINFOHEADER1] = */	_libbmp_write_OS2_BITMAPINFOHEADER1,
	/*[libbmp_file_format_OS2_BITMAPINFOHEADER2] = */	_libbmp_write_OS2_BITMAPINFOHEADER2,
#else
	/*[libbmp_file_format_OS2_BITMAPINFOHEADER1] = */	nullptr,
	/*[libbmp_file_format_OS2_BITMAPINFOHEADER2] = */	nullptr,
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
};
f2_status_t _libbmp_write_header( libbmp_bmp_writer_state * state )
{
	f2_status_t		status;
	const libbmp_image_descriptor * img = &state->context->image_desc;
#if !LIBBMP_CFG_NO_OS2_SUPPORT
	f2_bool_t	os2_format = libbmp_file_format_OS2_BITMAPINFOHEADER1 == state->file_format || libbmp_file_format_OS2_BITMAPINFOHEADER1 == state->file_format;
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT

	// Choose appropriate header if the file format was not specified
	state->file_format = img->file_format;
	if( libbmp_file_format_unspecified == img->file_format )
	{
		// Always choose standard Windows variant
		state->file_format = libbmp_file_format_BITMAPINFOHEADER;	// by default, the most common header
		// TODO: should we use more advanced headers?
	}
	debugbreak_if( libbmp_file_format_max_count >= (unsigned) state->file_format )
		return F2_STATUS_ERROR_INVALID_STATE;

	// Choose appropriate compression scheme if the scheme was not specified
	if( libbmp_compression_unspecified == img->compression )
	{
		// We do not use PNG or JPEG compression scheme by default because many importers do not support it
		switch( img->pixel_format.bits_per_pixel )
		{
		case 1:
#if !LIBBMP_CFG_NO_OS2_SUPPORT
			if( os2_format )
				state->compression = BCA_HUFFMAN1D;
			else
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
				state->compression = BI_RGB;
			break;
		case 2:	
			state->compression = BI_RGB;
			break;
		case 4:
			state->compression = BI_RLE4;
			break;
		case 8:
			state->compression = BI_RLE8;
			break;
		case 16:
			state->compression = BI_RGB;
			break;
		case 24:
#if !LIBBMP_CFG_NO_OS2_SUPPORT
			if( os2_format )
				state->compression = BCA_RLE24;
			else
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
				state->compression = BI_BITFIELDS;
			break;
		case 32:
			state->compression = BI_ALPHABITFIELDS;
			break;
		default:
			return F2_STATUS_ERROR_INVALID_STATE;
		}
	}
	else
	{
#if !LIBBMP_CFG_NO_OS2_SUPPORT
		if( os2_format )
			state->compression = _os2_compression_from_id( img->compression, img->pixel_format.bits_per_pixel );
		else
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
			state->compression = _win_compression_from_id( img->compression, img->pixel_format.bits_per_pixel );
	}

#if !LIBBMP_CFG_NO_OS2_SUPPORT
	if( os2_format )
		state->compression_func = _os2_compression( state->compression );
	else
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
		state->compression_func = _win_compression( state->compression );
	debugbreak_if( nullptr == state->compression_func )
		return F2_STATUS_ERROR_INVALID_STATE;

	// Initialize and write headers
	state->file_header = (BITMAPFILEHEADER *) state->ptr;
	state->bitmap_header = (COMMONBITMAPHEADER *) (state->ptr + sizeof(BITMAPFILEHEADER));
	debugbreak_if( _countof(_libbmp_write_func) >= (unsigned) state->file_format )
		return F2_STATUS_ERROR_INVALID_STATE;
	if( nullptr == _libbmp_write_func[state->file_format] )
		return F2_STATUS_ERROR_NOT_SUPPORTED;
	status = (*_libbmp_write_func[state->file_format])( state );
	if( f2_failed( status ) )
		return status;
	state->ptr += sizeof(BITMAPFILEHEADER) + state->bitmap_header->WinBitmapHeader.biSize;

	// Write bitfields if present
#if !LIBBMP_CFG_NO_OS2_SUPPORT
	if( !os2_format )
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
	{
		switch( state->compression )
		{
		case BI_ALPHABITFIELDS:
			PD(state->ptr)[0] = img->pixel_format.r_mask;
			PD(state->ptr)[1] = img->pixel_format.g_mask;
			PD(state->ptr)[2] = img->pixel_format.b_mask;
			PD(state->ptr)[3] = img->pixel_format.a_mask;
			state->ptr += sizeof(DWORD) * 4;
			break;
		case BI_BITFIELDS:
			PD(state->ptr)[0] = img->pixel_format.r_mask;
			PD(state->ptr)[1] = img->pixel_format.g_mask;
			PD(state->ptr)[2] = img->pixel_format.b_mask;
			state->ptr += sizeof(DWORD) * 3;
			break;
		}
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t _libbmp_write_color_table( libbmp_bmp_writer_state * state )
{
	f2_status_t status;
	unsigned i, entry_count;
	const unsigned bpp = state->context->image_desc.pixel_format.bits_per_pixel;
	unsigned max_color_table_size;
	libbmp_rgbx entry[256], * en;
	libbmp_ipalette * const palette = state->context->palette;

	// Check current state
	if( nullptr == state->context->palette )	// if client doen't provide color table
	{
		// Check both for bit count and palette interface to display entire expression in the debug output in case condition fails
		debugbreak_if( 8 >= bpp && nullptr == palette )
			return F2_STATUS_ERROR_INVALID_STATE;	// 1-, 4-, 8-bpp images should have color table
		return F2_STATUS_SUCCESS;
	}
	debugbreak_if( nullptr == palette->get_count || nullptr == palette->get_entries )
		return F2_STATUS_ERROR_INVALID_STATE;

	max_color_table_size = 8 >= bpp ? 1 << bpp : 256;	// for non-palette mode, maximum size of color table is 256

	// Get palette information
	entry_count = 0;
	status = palette->get_count( palette, &entry_count );
	if( f2_failed( status ) )
		return status;
	if( 8 < bpp && 0 == entry_count )	// non-paletted images are allowed to not have color table
		return F2_STATUS_SUCCESS;

	debugbreak_if( !in_range_exclusive( entry_count, 0, max_color_table_size ) )
		return F2_STATUS_ERROR_INVALID_STATE;

	// Get palette
	f2_clear_mem( entry, sizeof(entry) );
	status = palette->get_entries( palette, entry, 0, entry_count );
	if( f2_failed( status ) )
		return status;

	// Write palette
	en = entry;
#if !LIBBMP_CFG_NO_LEGACY_SUPPORT
	if( sizeof(BITMAPCOREHEADER) == state->bitmap_header->BitmapCoreHeader.bcSize )
	{
		RGBTRIPLE * rgb = (RGBTRIPLE *) state->ptr;
		for( i = 0; i < entry_count; ++ i, ++ rgb, ++ en )
		{
			rgb->rgbtBlue	= en->b;
			rgb->rgbtGreen	= en->g;
			rgb->rgbtRed	= en->r;
		}
		state->ptr += sizeof(RGBTRIPLE) * entry_count;
	}
	else
#endif // !LIBBMP_CFG_NO_LEGACY_SUPPORT
	{
		RGBQUAD * rgb = (RGBQUAD *) state->ptr;
		for( i = 0; i < entry_count; ++ i, ++ rgb, ++ en )
		{
			rgb->rgbBlue	= en->b;
			rgb->rgbGreen	= en->g;
			rgb->rgbRed		= en->r;
			rgb->rgbReserved= en->x;
		}
		state->ptr += sizeof(RGBQUAD) * entry_count;
	}

	// Done
	if( state->bitmap_header->WinBitmapHeader.biSize >= sizeof(BITMAPINFOHEADER) )
		state->bitmap_header->WinBitmapHeader.biClrUsed = entry_count;

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t _libbmp_write_data( libbmp_bmp_writer_state * state )
{
	f2_status_t	status;

	// Check current state
	debugbreak_if( nullptr == state->compression_func )
		return F2_STATUS_ERROR_UNKNOWN_COMPRESSION;

	state->file_header->bfOffBits = (DWORD) (state->ptr - PB(state->buffer));

	// Decompress data to the buffer
	status = state->compression_func( state );
	if( f2_failed( status ) )
		return status;

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t libbmp_write_bmp( const libbmp_bmp_writer_context * context )
{
	f2_status_t		status;
	libbmp_bmp_writer_state	state;
	size_t file_size;
	const libbmp_image_descriptor * img;
	const libbmp_pixel_format *		pf;

	// Check current state
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == context->image_data )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( 0 == context->image_data_size )
		return F2_STATUS_ERROR_INVALID_STATE;

	debugbreak_if( nullptr == context->bitmap )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( nullptr == context->bitmap->get_buffer || nullptr == context->bitmap->set_buffer_size )
		return F2_STATUS_ERROR_INVALID_STATE;

	memset( &state, 0, sizeof(state) );
	state.context = context;

	// Check image characteristics
	img = &context->image_desc;
	debugbreak_if( libbmp_compression_max_count >= (unsigned) img->compression )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( libbmp_file_format_max_count >= (unsigned) img->file_format )
		return F2_STATUS_ERROR_INVALID_STATE;

	pf = &img->pixel_format;
	debugbreak_if( !CHECK_BIT_COUNT( pf->bits_per_pixel ) )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( !CHECK_BIT_COUNT( pf->bits_per_pixel ) )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( 0 != (pf->r_mask & pf->g_mask & pf->b_mask & pf->a_mask & pf->x_mask) )
		return F2_STATUS_ERROR_INVALID_STATE;

	debugbreak_if( 0 == img->width )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( 0 > (signed) img->width )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( 0 == img->height )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( 0 > (signed) img->pitch )
		return F2_STATUS_ERROR_INVALID_STATE;

	state.pitch = img->pitch;
	if( 0 == state.pitch )
		state.pitch = (img->width * pf->bits_per_pixel + 31) / 4;

	// Allocate buffer (actual memory used can be less due to compression)
	state.buffer_size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV5INFOHEADER) + 4*sizeof(RGBQUAD) + 256*sizeof(RGBQUAD) + (state.pitch + 2)*img->height;
	status = context->bitmap->get_buffer( context->bitmap, &state.buffer, state.buffer_size );
	if( f2_failed( status ) )
		return status;
	state.ptr = state.buffer;

	// Write file header
	status = _libbmp_write_header( &state );
	if( f2_failed( status ) )
		return status;

	// Write palette, if present
	if( nullptr != context->palette )
	{
		status = _libbmp_write_color_table( &state );
		if( f2_failed(status) )
			return status;
	}

	// Write file data
	status = _libbmp_write_data( &state );
	if( f2_failed(status) )
		return status;

	// Finish operation
	file_size = state.ptr - PCB(state.buffer);
	state.file_header->bfSize = (DWORD) file_size;

	status = context->bitmap->set_buffer_size( context->bitmap, file_size );
	if( f2_failed(status) )
		return status;

	// Exit
	return F2_STATUS_SUCCESS;
}

/*END OF write.c*/
