/*read.c*/
#include "pch.h"
#include "main.h"

/**
 * @brief Read file header, parse and check image characteristics.
 */
f2_status_t _libbmp_read_header( libbmp_bmp_reader_state * state )
{
	f2_status_t			status;
	BITMAPFILEHEADER *		file_header;
	COMMONBITMAPHEADER *	bitmap_header;
	const unsigned char *	data = (const unsigned char *) state->ptr;
	size_t					data_size = state->size_left;
	unsigned				planes;
	BITMAPINFOHEADER *		bih;
	int						os2_header = 0;

	// Read file bitmap_header
	if( sizeof(*file_header) >= data_size )
		return F2_STATUS_ERROR_UNKNOWN_FORMAT;
	file_header = (BITMAPFILEHEADER *) data;

	if( LIBBMP_FILETYPE_BITMAP != file_header->bfType )		// try to read raw DIB file (w/o file header)
	{
		file_header = &state->default_file_header;
		file_header->bfType = LIBBMP_FILETYPE_BITMAP;
		file_header->bfSize = (DWORD) data_size;
		file_header->bfReserved1 = 0;
		file_header->bfReserved2 = 0;
		file_header->bfOffBits	 = 0;
	}
	else
	{
		if( file_header->bfSize != state->context->data_size )	// file size should correspond to the one specified in the header
			return F2_STATUS_ERROR_UNKNOWN_FORMAT;
		if( file_header->bfOffBits >= file_header->bfSize )
			return F2_STATUS_ERROR_UNKNOWN_FORMAT;

		data += sizeof(*file_header);
		data_size -= sizeof(*file_header);
	}

	state->file_header = file_header;

	// Read bitmap header
	if( sizeof(*bitmap_header) >= data_size )
		return F2_STATUS_ERROR_UNKNOWN_FORMAT;
	bitmap_header = (COMMONBITMAPHEADER *) data;

	bih = &bitmap_header->WinBitmapHeader;
	if( 0 == bih->biHeight )	// zero height image is pointless
		return 0;
	if( 1 > bih->biWidth )		// width is always positive and non-zero
		return 0;
	planes = bih->biPlanes;
	if( 0 == planes )
		return F2_STATUS_ERROR_INVALID_DATA;
	if( planes > LIBBMP_MAX_PLANES )								// verify plane count cap
		return F2_STATUS_ERROR_INVALID_DATA;
	if( (bih->biBitCount / planes) * planes != bih->biBitCount )	// assume that 24-bit images can be split in 3 planes
		return F2_STATUS_ERROR_INVALID_DATA;
	if( !CHECK_BIT_COUNT( bih->biBitCount ) )
		return F2_STATUS_ERROR_INVALID_DATA;
	if( !in_range( bih->biCompression, BI_RGB, BI_ALPHABITFIELDS ) &&
		!in_range( bih->biCompression, BI_CMYK, BI_CMYKRLE4 ) )
		return F2_STATUS_ERROR_INVALID_DATA;

	state->image_desc.file_format = libbmp_file_format_unspecified;
	state->image_desc.compression = libbmp_compression_unspecified;
	state->image_desc.flags	= LIBBMP_IMAGE_UPSIDEDOWN;
	state->bitmap_header	= bitmap_header;
	data += sizeof(*bitmap_header);
	data_size -= sizeof(*bitmap_header);

	/* Store bitmap information */
	switch( bih->biSize )
	{
	// Windows compatible formats
	case sizeof(BITMAPV5INFOHEADER):
		state->image_desc.file_format = libbmp_file_format_BITMAPV5INFOHEADER;
		state->profile_offset = bitmap_header->WinBitmapV5Header.bV5ProfileData;
		state->profile_size = bitmap_header->WinBitmapV5Header.bV5ProfileSize;
		state->intent = bitmap_header->WinBitmapV5Header.bV5Intent;
		// pass through
	case sizeof(BITMAPV4INFOHEADER):
		state->image_desc.file_format = libbmp_file_format_BITMAPV4INFOHEADER;
		switch( bitmap_header->WinBitmapV4Header.biV4CSType )
		{
		case LCS_CALIBRATED_RGB:
			state->colorspace_filter.cs_type = libbmp_colorspace_filter_calibrated;
			state->colorspace_filter.profile.calibrated.endpoints	= * (icexyz_triple_t *) &bitmap_header->WinBitmapV4Header.biV4Endpoints;
			state->colorspace_filter.profile.calibrated.gamma_red	= FIXED1616_TO_DOUBLE(bitmap_header->WinBitmapV4Header.biV4GammaRed);
			state->colorspace_filter.profile.calibrated.gamma_green	= FIXED1616_TO_DOUBLE(bitmap_header->WinBitmapV4Header.biV4GammaGreen);
			state->colorspace_filter.profile.calibrated.gamma_blue	= FIXED1616_TO_DOUBLE(bitmap_header->WinBitmapV4Header.biV4GammaBlue);
			break;
		case LCS_sRGB:
			state->colorspace_filter.cs_type = libbmp_colorspace_filter_sRGB;
			break;
		case LCS_WINDOWS_COLOR_SPACE:
			state->colorspace_filter.cs_type = libbmp_colorspace_filter_windows;
			break;
		// support for V5 is placed here
		case PROFILE_EMBEDDED:
			// TODO:
			// - check that profile_data and profile_size are valid
			state->colorspace_filter.cs_type = libbmp_colorspace_filter_embedded;
			state->colorspace_filter.profile.embedded.profile_data_offset	= state->profile_offset;
			state->colorspace_filter.profile.embedded.profile_data_size		= state->profile_size;
			break;
		case PROFILE_LINKED:
			// TODO:
			// - check operation results
			// - check that profile_data and profile_size are valid
			state->colorspace_filter.cs_type = libbmp_colorspace_filter_external;
			state->colorspace_filter.profile.embedded.profile_data_offset	= state->profile_offset;
			state->colorspace_filter.profile.embedded.profile_data_size		= state->profile_size;
			break;
		default:
			state->colorspace_filter.cs_type = libbmp_colorspace_filter_none;
			break;
		}
		// pass through
	case sizeof(BITMAPV3INFOHEADER):
		state->image_desc.file_format = libbmp_file_format_BITMAPV3INFOHEADER;
		state->image_desc.pixel_format.a_mask	= bitmap_header->WinBitmapV3Header.biV3AlphaMask;
		// pass through
	case sizeof(BITMAPV2INFOHEADER):
		state->image_desc.file_format = libbmp_file_format_BITMAPV2INFOHEADER;
		if( libbmp_colorspace_filter_none == state->colorspace_filter.cs_type )
		{
			state->image_desc.pixel_format.r_mask	= bitmap_header->WinBitmapV2Header.biV2RedMask;
			state->image_desc.pixel_format.g_mask	= bitmap_header->WinBitmapV2Header.biV2GreenMask;
			state->image_desc.pixel_format.b_mask	= bitmap_header->WinBitmapV2Header.biV2BlueMask;
		}
		// pass through
	case sizeof(BITMAPINFOHEADER):
		state->image_desc.file_format	= libbmp_file_format_BITMAPINFOHEADER;
		state->image_desc.width			= bih->biWidth;
		state->image_desc.height		= f2_abs( bih->biHeight );
		state->image_desc.pitch			= (bih->biWidth * bih->biBitCount + 31) / 8 / bih->biPlanes;	// align to 32 bit
		state->image_desc.pixel_format.bits_per_pixel	= bih->biBitCount;
		state->planes			= bih->biPlanes;
		state->compression		= _win_decompression( bih->biCompression );
		state->data_size		= bih->biSizeImage;
		state->pal_elem_size	= 4;
		state->colors_used		= bih->biClrUsed;

		if( sizeof(BITMAPINFOHEADER) == bih->biSize )	// set default pixel masks
		{
			state->image_desc.pixel_format.x_mask	= 0x0000;
			switch( state->image_desc.pixel_format.bits_per_pixel )
			{
			case 16:
				state->image_desc.pixel_format.r_mask	= 0xf800;
				state->image_desc.pixel_format.g_mask	= 0x07e0;
				state->image_desc.pixel_format.b_mask	= 0x001f;
				break;
			case 32:
				state->image_desc.pixel_format.x_mask	= 0xFF000000;
				// pass through
			case 24:
				state->image_desc.pixel_format.r_mask	= 0x00FF0000;
				state->image_desc.pixel_format.g_mask	= 0x0000FF00;
				state->image_desc.pixel_format.b_mask	= 0x000000FF;
				break;
			default:
				state->image_desc.pixel_format.r_mask	=
				state->image_desc.pixel_format.g_mask	=
				state->image_desc.pixel_format.b_mask	= 0x00000000;
				break;
			}
		}
		break;
	// Other variants
#if !LIBBMP_CFG_NO_LEGACY_SUPPORT
	case sizeof(BITMAPOLDHEADER):	// old bitmap 16-bit format
		if( 2 != bitmap_header->BitmapOldHeader.Version )
			return F2_STATUS_ERROR_INVALID_DATA;
		state->image_desc.width			= bitmap_header->BitmapOldHeader.Width;
		state->image_desc.height		= bitmap_header->BitmapOldHeader.Height;
		state->image_desc.pitch			= bitmap_header->BitmapOldHeader.Pitch;
		state->image_desc.pixel_format.bits_per_pixel	= bitmap_header->BitmapOldHeader.BitCount;
		state->planes			= bitmap_header->BitmapOldHeader.Planes;
		state->compression		= _libbmp_decompress_none;
		state->data_size		= 0;
		state->pal_elem_size	= 3;
		state->colors_used		= bih->biClrUsed;

		break;
	case sizeof(BITMAPCOREHEADER):	// OS/2 and Windows compatible format(s)
		state->image_desc.file_format	= libbmp_file_format_BITMAPCOREHEADER;
		state->image_desc.width		= bitmap_header->BitmapCoreHeader.bcWidth;
		state->image_desc.height	= bitmap_header->BitmapCoreHeader.bcHeight;
		state->image_desc.pitch		= (bitmap_header->BitmapCoreHeader.bcWidth * bitmap_header->BitmapCoreHeader.bcBitCount + 31) / 8 / bitmap_header->BitmapCoreHeader.bcPlanes;	// align to 32 bit
		state->image_desc.pixel_format.bits_per_pixel	= bitmap_header->BitmapCoreHeader.bcBitCount;
		state->planes			= bitmap_header->BitmapCoreHeader.bcPlanes;
		state->compression		= _libbmp_decompress_none;
		state->pal_elem_size	= 3;
		break;
#endif // !LIBBMP_CFG_NO_LEGACY_SUPPORT
	default:
		// OS/2 bitmap format v.2 has variable size, varying from minimum size of v.1 up to size of whole v.2 struct
#if !LIBBMP_CFG_NO_OS2_SUPPORT
		if( sizeof(OS2BMPFILEOLDHEADER) < bitmap_header->Os22BitmapHeader.cbFix && bitmap_header->Os22BitmapHeader.cbFix <= sizeof(OS22XBITMAPHEADER) )
		{
			OS22XBITMAPHEADER os2bih;
			f2_clear_struct( &os2bih );
			f2_copy_mem( &os2bih, &bitmap_header->Os22BitmapHeader, bitmap_header->Os22BitmapHeader.cbFix );

			state->image_desc.file_format	= libbmp_file_format_OS2_BITMAPINFOHEADER2;
			state->image_desc.width		= os2bih.cx;
			state->image_desc.height	= os2bih.cy;
			state->image_desc.pitch		= (os2bih.cx * os2bih.cBitCount + 15) / 8 / os2bih.cPlanes;	// align to 16 bit
			state->image_desc.pixel_format.bits_per_pixel	= os2bih.cBitCount;
			state->planes			= os2bih.cPlanes;
			state->compression		= _os2_decompression( os2bih.ulCompression );
			state->data_size		= os2bih.cbImage;
			state->pal_elem_size	= 4;

			os2_header = 1;

			break;
		}
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
		// Bitmap information is not valid
		status = F2_STATUS_ERROR_INVALID_DATA;
		break;
	}

	// Perform common fixes
	if( file_header == &state->default_file_header )	// a headerless DIB file; fill in bits offset
	{
		file_header->bfOffBits = state->bitmap_header->WinBitmapHeader.biSize;
		if( !os2_header )
		{
			if( BI_BITFIELDS == bitmap_header->WinBitmapHeader.biCompression )
				file_header->bfOffBits += 3 * sizeof(DWORD);
			else if( BI_ALPHABITFIELDS == bitmap_header->WinBitmapHeader.biCompression )
				file_header->bfOffBits += 4 * sizeof(DWORD);
		}
		file_header->bfOffBits += state->colors_used * state->pal_elem_size;
	}

	if( 0 == state->colors_used && state->image_desc.pixel_format.bits_per_pixel <= 8 )
	{
		unsigned entries = (file_header->bfOffBits - sizeof(BITMAPFILEHEADER) - bih->biSize) / state->pal_elem_size;
		unsigned max_size = 1U << state->image_desc.pixel_format.bits_per_pixel;
		if( entries < max_size )
			state->colors_used = entries;
		else
			state->colors_used = max_size;
	}

	if( state->compression == _libbmp_decompress_bitfields && 32 == state->image_desc.pixel_format.bits_per_pixel )
		state->compression = _libbmp_decompress_alpha_bitfields;

	if( 0 == state->data_size )
		state->data_size = state->image_desc.pitch * state->image_desc.height;

	if( 0 > bih->biHeight )
	{
		state->image_desc.flags	&= ~LIBBMP_IMAGE_UPSIDEDOWN;
		state->image_desc.height = (unsigned) - (signed) state->image_desc.height;
	}

	/* Fix some non-critical issues */
	if( bih->biClrUsed > (1U << bih->biBitCount) )
		bih->biClrUsed = (1U << bih->biBitCount);
	if( bih->biClrImportant > bih->biClrUsed )
		bih->biClrImportant = bih->biClrUsed;

	/* Check resulting data */
#ifdef LIBBMP_STRICT
	if( libbmp_succeeded(status) )
	{
		libbmp_verify(
			libbmp_succeeded(status = _check_format_bpp( state )) &&
			libbmp_succeeded(status = _check_format_planes( state )) &&
			libbmp_succeeded(status = _check_image_size( state ))
			);
	}
#endif /* def LIBBMP_STRICT */
//	if( f2_failed(status) )
//		return status;

	// Done
	state->ptr = data;
	state->size_left = data_size;

	// Exit
	return F2_STATUS_SUCCESS;
}

/**
 * @brief Read paleete to internal buffer (for later usage) and send the palette to the palette output interface.
 */
f2_status_t _libbmp_read_color_table( libbmp_bmp_reader_state * state )
{
	f2_status_t status;
	unsigned palette_size;

	// Check current state
	debugbreak_if( 3 != state->pal_elem_size && 4 != state->pal_elem_size )
		return F2_STATUS_ERROR_INVALID_DATA;

	// Get palette data
	palette_size = state->colors_used * state->pal_elem_size;
	if( state->size_left < palette_size )
		return F2_STATUS_ERROR_INVALID_DATA;

	if( state->compression == _libbmp_decompress_bitfields )
		state->ptr += 3*sizeof(uint32_t);	// skip R, G, B masks
	else if( state->compression == _libbmp_decompress_alpha_bitfields )
		state->ptr += 4*sizeof(uint32_t);	// skip R, G, B, A masks

	state->palette = (libbmp_rgbx *) state->ptr;

	// If actual type is RQBTRIPLE, convert palette to the RGBQUAD representation
	if( 3 == state->pal_elem_size )
	{
		RGBTRIPLE * rgb = ((RGBTRIPLE *) state->palette) + state->colors_used - 1;
		libbmp_rgbx * rgbx = state->palette + state->colors_used - 1;
		for( ; rgbx > state->palette; -- rgb, -- rgbx )
		{
			BYTE r = rgb->rgbtRed;
			BYTE g = rgb->rgbtGreen;
			BYTE b = rgb->rgbtBlue;
			rgbx->r	= r;
			rgbx->g	= g;
			rgbx->b	= b;
			rgbx->x	= 255;
		}
	}

	// Notify palette interface
	status = state->context->palette->set_count( state->context->palette, state->colors_used );
	if( f2_failed( status ) )
		return status;

	status = state->context->palette->set_entries( state->context->palette, state->palette, 0, state->colors_used );
	if( f2_failed( status ) )
		return status;

	// Done
	state->ptr += palette_size;
	state->size_left -= palette_size;

	// Exit
	return F2_STATUS_SUCCESS;
}
/**
 * @brief Decompress and read the image.
 */
f2_status_t _libbmp_read_data( libbmp_bmp_reader_state * state )
{
	f2_status_t	status;

	// Check current state
	debugbreak_if( nullptr == state->compression )
		return F2_STATUS_ERROR_UNKNOWN_COMPRESSION;

	// Offset to image data
	debugbreak_if( state->file_header->bfOffBits > state->context->data_size )
		return F2_STATUS_ERROR_INVALID_DATA;

	state->ptr = ((const unsigned char *) state->ptr) + state->file_header->bfOffBits;
	state->size_left = state->context->data_size - state->file_header->bfOffBits;

	// Acquire destination buffer
	status = state->context->bitmap->set_desc( state->context->bitmap, &state->image_desc );
	debugbreak_if( f2_failed( status ) )
		return status;

	state->dst_buf = nullptr;
	state->dst_pitch = 0;
	status = state->context->bitmap->get_buffer( state->context->bitmap, &state->dst_buf, &state->dst_pitch );
	debugbreak_if( f2_failed( status ) )
		return status;
	debugbreak_if( nullptr == state->dst_buf || state->image_desc.pitch > state->dst_pitch )
		return F2_STATUS_ERROR_INVALID_INTERFACE;

	// Decompress data to the buffer
	status = state->compression( state );
	if( f2_failed( status ) )
		return status;

	// Exit
	return F2_STATUS_SUCCESS;
}

/**
 * @brief Read bitmap image.
 */
f2_status_t libbmp_read_bmp( const libbmp_bmp_reader_context * context )
{
	f2_status_t		status;
	libbmp_bmp_reader_state	state;

	// Check current state
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == context->data )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( nullptr != context->bitmap && (nullptr == context->bitmap->set_desc || nullptr == context->bitmap->get_buffer) )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( nullptr != context->palette && (nullptr == context->palette->set_count || nullptr == context->palette->set_entries) )
		return F2_STATUS_ERROR_INVALID_STATE;

	memset( &state, 0, sizeof(state) );
	state.context = context;
	state.ptr = context->data;
	state.size_left = context->data_size;

	// Read bitmap_header
	status = _libbmp_read_header( &state );
	if( f2_failed(status) )
		return status;

	// Read palette
	if( nullptr != context->palette  && 0 != state.colors_used )
	{
		status = _libbmp_read_color_table( &state );
		if( f2_failed(status) )
			return status;
	}

	// Read data
	if( nullptr != context->bitmap )
	{
		status = _libbmp_read_data( &state );
		if( f2_failed(status) )
			return status;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

/*END OF read.c*/
