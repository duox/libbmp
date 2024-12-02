/*dump.c*/
#include "pch.h"
#include "main.h"

struct libbmp_bmp_dump_state {
	const libbmp_dump_context *	context;
	BITMAPFILEHEADER			default_file_header;
	const BITMAPFILEHEADER *	file_header;
	const COMMONBITMAPHEADER *	bitmap_header;
	unsigned	width;
	unsigned	height;
	unsigned	pitch;
	unsigned	colors_used;
	unsigned	pal_elem_size;
	const void *color_table;
	const void *data;
};
typedef struct libbmp_bmp_dump_state	libbmp_bmp_dump_state;

f2_status_t _libbmp_ident( libbmp_bmp_dump_state * state, int ident )
{
	return state->context->msgout->ident( state->context->msgout, ident );
}
f2_status_t _libbmp_print_header( libbmp_bmp_dump_state * state, const char * tag )
{
	return state->context->msgout->print_header( state->context->msgout, tag );
}
f2_status_t _libbmp_printf_header( libbmp_bmp_dump_state * state, const char * text, ... )
{
	char buf[256];
	va_list ap;
	va_start( ap, text );
	_vsnprintf( buf, _countof(buf), text, ap );
	va_end( ap );
	return state->context->msgout->print_header( state->context->msgout, buf );
}

f2_status_t _libbmp_print_uint8( libbmp_bmp_dump_state * state, const char * tag, uint8_t value, const char * desc )
{
	char buf[64];
	size_t len = _snprintf( buf, _countof(buf), "%d", value );
	return state->context->msgout->print( state->context->msgout, tag, buf, len, desc );
}
f2_status_t _libbmp_print_uint8_hex( libbmp_bmp_dump_state * state, const char * tag, uint8_t value, const char * desc )
{
	char buf[64];
	size_t len = _snprintf( buf, _countof(buf), "%02X", value );
	return state->context->msgout->print( state->context->msgout, tag, buf, len, desc );
}
f2_status_t _libbmp_print_uint16( libbmp_bmp_dump_state * state, const char * tag, uint16_t value, const char * desc )
{
	char buf[64];
	size_t len = _snprintf( buf, _countof(buf), "%d", value );
	return state->context->msgout->print( state->context->msgout, tag, buf, len, desc );
}
f2_status_t _libbmp_print_int32( libbmp_bmp_dump_state * state, const char * tag, int32_t value, const char * desc )
{
	char buf[64];
	size_t len = _snprintf( buf, _countof(buf), "%d", value );
	return state->context->msgout->print( state->context->msgout, tag, buf, len, desc );
}
f2_status_t _libbmp_print_uint32( libbmp_bmp_dump_state * state, const char * tag, uint32_t value, const char * desc )
{
	char buf[64];
	size_t len = _snprintf( buf, _countof(buf), "%d", value );
	return state->context->msgout->print( state->context->msgout, tag, buf, len, desc );
}
f2_status_t _libbmp_print_uint32_hex( libbmp_bmp_dump_state * state, const char * tag, uint32_t value, const char * desc )
{
	char buf[64];
	size_t len = _snprintf( buf, _countof(buf), "%08X", value );
	return state->context->msgout->print( state->context->msgout, tag, buf, len, desc );
}

f2_status_t _libbmp_print_string( libbmp_bmp_dump_state * state, const char * tag, const char * value, size_t len, const char * desc )
{
	return state->context->msgout->print( state->context->msgout, tag, value, len, desc );
}
f2_status_t _libbmp_printf_string( libbmp_bmp_dump_state * state, const char * tag, const char * text, ... )
{
	char buf[256];
	size_t n;
	va_list ap;
	va_start( ap, text );
	n = _vsnprintf( buf, _countof(buf), text, ap );
	va_end( ap );
	return state->context->msgout->print( state->context->msgout, tag, buf, n, nullptr );
}

f2_status_t dump_bytes( libbmp_bmp_dump_state * state, unsigned offset, size_t size )
{
	const uint8_t * data = PCB(state->context->data) + (offset & ~0x0fUL);
	const uint8_t * data_start = PCB(state->context->data) + offset;
	const uint8_t * data_end = PCB(state->context->data) + size;
	char buf[8 + 1 + 2 + 16*3 + 1], * ptr;
	unsigned col;

	while( data < data_end )
	{
		// Dump offset
		ptr = buf;
		ptr += sprintf( ptr, "%08X:\40\40", PCB(state->context->data) - data );

		// Dump bytes
		for( col = 0; col < 16; ++ col, ++ data )
		{
			if( data < data_start )
				ptr += sprintf( ptr, "\40\40\40" );
			else
				ptr += sprintf( ptr, " %02X", *data );
			if( data >= data_end )
				break;
		}
		_libbmp_printf_string( state, nullptr, "%s", buf );
	}
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !LIBBMP_CFG_NO_LEGACY_SUPPORT

f2_status_t _libbmp_dump_header_BITMAPOLDHEADER( libbmp_bmp_dump_state * state, const BITMAPOLDHEADER * bih )
{
	_libbmp_print_uint8 ( state, "Version",		bih->Version,	"Structure version (always 2)" );
	_libbmp_print_uint8 ( state, "Flags",		bih->Flags,		nullptr );
	_libbmp_print_uint16( state, "Reserved0",	bih->Reserved0,	nullptr );
	_libbmp_print_uint16( state, "Width",		bih->Width,		nullptr );
	_libbmp_print_uint16( state, "Height",		bih->Height,	nullptr );
	_libbmp_print_uint16( state, "Pitch",		bih->Pitch,		nullptr );
	_libbmp_print_uint8 ( state, "Planes",		bih->Planes,	nullptr );
	_libbmp_print_uint8 ( state, "BitCount",	bih->BitCount,	nullptr );
	_libbmp_print_uint16( state, "Reserved1",	bih->Reserved1,	nullptr );
	_libbmp_print_uint16( state, "Reserved2",	bih->Reserved2,	nullptr );

	state->width	= bih->Width;
	state->height	= bih->Height;
	state->pitch	= bih->Pitch;

	state->colors_used	= 1 << bih->BitCount;
	state->pal_elem_size= 3;
	state->color_table	= PCB(state->context->data) + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPOLDHEADER);

	state->data = PCB(state->color_table) + state->colors_used*state->pal_elem_size;

	return F2_STATUS_SUCCESS;
}

f2_status_t _libbmp_dump_header_BITMAPCOREHEADER( libbmp_bmp_dump_state * state, const BITMAPCOREHEADER * bih )
{
	_libbmp_print_uint32( state, "bcSize",			bih->bcSize,		nullptr );
	_libbmp_print_uint16( state, "bcWidth",			bih->bcWidth,		nullptr );
	_libbmp_print_uint16( state, "bcHeight",		bih->bcHeight,		nullptr );
	_libbmp_print_uint16( state, "biPlanes",		bih->bcPlanes,		nullptr );
	_libbmp_print_uint16( state, "biBitCount",		bih->bcBitCount,	nullptr );

	state->colors_used	= 1 << bih->bcBitCount;
	state->pal_elem_size= 3;
	state->color_table	= PCB(state->context->data) + sizeof(BITMAPFILEHEADER) + bih->bcSize;

	return F2_STATUS_SUCCESS;
}

#endif // !LIBBMP_CFG_NO_LEGACY_SUPPORT

f2_status_t _libbmp_dump_header_BITMAPINFOHEADER( libbmp_bmp_dump_state * state, const BITMAPINFOHEADER * bih )
{
	unsigned planes = bih->biPlanes;
	if( 0 == planes ) {
		libbmp_diagnostics( state->context->error_handler, F2_STATUS_SUCCESS, libbmp_diag_message_null_plane_count, 0, 0 );
		planes = 1;
	}
	else if( (bih->biBitCount / planes) * planes != bih->biBitCount )
		return libbmp_diagnostics( state->context->error_handler, F2_STATUS_ERROR_INVALID_DATA, libbmp_diag_message_invalid_plane_count, 0, planes );

	_libbmp_print_uint32( state, "biSize",			bih->biSize,			nullptr );
	_libbmp_print_int32 ( state, "biWidth",			bih->biWidth,			nullptr );
	_libbmp_print_int32 ( state, "biHeight",		bih->biHeight,			nullptr );
	_libbmp_print_uint16( state, "biPlanes",		bih->biPlanes,			nullptr );
	_libbmp_print_uint16( state, "biBitCount",		bih->biBitCount,		nullptr );
	_libbmp_print_uint32( state, "biCompression",	bih->biCompression,		nullptr );
	_libbmp_print_uint32( state, "biSizeImage",		bih->biSizeImage,		nullptr );
	_libbmp_print_uint32( state, "biXPelsPerMeter",	bih->biXPelsPerMeter,	nullptr );
	_libbmp_print_uint32( state, "biYPelsPerMeter",	bih->biYPelsPerMeter,	nullptr );
	_libbmp_print_uint32( state, "biClrUsed",		bih->biClrUsed,			nullptr );
	_libbmp_print_uint32( state, "biClrImportant",	bih->biClrImportant,	nullptr );

	state->width	= bih->biWidth;
	state->height	= bih->biHeight;
	state->pitch	= (bih->biWidth * bih->biBitCount + 31) / 8 / planes;	// align to 32 bit

	state->colors_used	= bih->biClrUsed;
	state->pal_elem_size= 4;
	state->color_table	= PCB(state->context->data) + sizeof(BITMAPFILEHEADER) + bih->biSize;
	if( BI_BITFIELDS == bih->biCompression )
	{
		DWORD expected_image_offset = sizeof(BITMAPFILEHEADER) + bih->biSize + bih->biClrUsed*4;
		if( expected_image_offset + 12 == ((BITMAPFILEHEADER *) state->context->data)->bfOffBits )
		{
			_libbmp_print_header( state, "BITFIELDS[3]" );
			_libbmp_ident( state, +1 );
				_libbmp_print_uint32_hex( state, "RMask",	((DWORD *) state->color_table)[0],	nullptr );
				_libbmp_print_uint32_hex( state, "GMask",	((DWORD *) state->color_table)[1],	nullptr );
				_libbmp_print_uint32_hex( state, "BMask",	((DWORD *) state->color_table)[2],	nullptr );
			_libbmp_ident( state, -1 );
			state->color_table = PCB(state->color_table) + 3 * sizeof(DWORD);
		}
	}
	else if( BI_ALPHABITFIELDS == bih->biCompression )
	{
		DWORD expected_image_offset = sizeof(BITMAPFILEHEADER) + bih->biSize + bih->biClrUsed*4;
		if( expected_image_offset + 16 == ((BITMAPFILEHEADER *) state->context->data)->bfOffBits )
		{
			_libbmp_print_header( state, "BITFIELDS[4]" );
			_libbmp_ident( state, +1 );
				_libbmp_print_uint32_hex( state, "RMask",	((DWORD *) state->color_table)[0],	nullptr );
				_libbmp_print_uint32_hex( state, "GMask",	((DWORD *) state->color_table)[1],	nullptr );
				_libbmp_print_uint32_hex( state, "BMask",	((DWORD *) state->color_table)[2],	nullptr );
				_libbmp_print_uint32_hex( state, "AMask",	((DWORD *) state->color_table)[3],	nullptr );
			_libbmp_ident( state, -1 );
			state->color_table = PCB(state->color_table) + 4 * sizeof(DWORD);
		}
	}

	state->data = PCB(state->color_table) + state->colors_used*state->pal_elem_size;

	return F2_STATUS_SUCCESS;
}
f2_status_t _libbmp_dump_header_BITMAPV2INFOHEADER( libbmp_bmp_dump_state * state, const BITMAPV2INFOHEADER * bih )
{
	_libbmp_dump_header_BITMAPINFOHEADER( state, (const BITMAPINFOHEADER *) bih );

	_libbmp_print_uint32_hex( state, "biV2RedMask",		bih->biV2RedMask,	nullptr );
	_libbmp_print_uint32_hex( state, "biV2GreenMask",	bih->biV2GreenMask,	nullptr );
	_libbmp_print_uint32_hex( state, "biV2BlueMask",	bih->biV2BlueMask,	nullptr );

	return F2_STATUS_SUCCESS;
}
f2_status_t _libbmp_dump_header_BITMAPV3INFOHEADER( libbmp_bmp_dump_state * state, const BITMAPV3INFOHEADER * bih )
{
	_libbmp_dump_header_BITMAPV2INFOHEADER( state, (const BITMAPV2INFOHEADER *) bih );

	_libbmp_print_uint32_hex( state, "biV3AlphaMask",	bih->biV3AlphaMask,	nullptr );

	return F2_STATUS_SUCCESS;
}
f2_status_t _libbmp_dump_header_BITMAPV4INFOHEADER( libbmp_bmp_dump_state * state, const BITMAPV4INFOHEADER * bih )
{
	_libbmp_dump_header_BITMAPV3INFOHEADER( state, (const BITMAPV3INFOHEADER *) bih );

	_libbmp_print_uint32( state, "biV4CSType",	bih->biV4CSType,	nullptr );

	_libbmp_print_header( state, "biV4Endpoints" );
	_libbmp_ident( state, +1 );
		_libbmp_print_header( state, "ciexyzRed" );
		_libbmp_ident( state, +1 );
			_libbmp_print_uint32( state, "ciexyzX",	bih->biV4Endpoints.ciexyzRed.ciexyzX, nullptr );
			_libbmp_print_uint32( state, "ciexyzX",	bih->biV4Endpoints.ciexyzRed.ciexyzY, nullptr );
			_libbmp_print_uint32( state, "ciexyzX",	bih->biV4Endpoints.ciexyzRed.ciexyzZ, nullptr );
		_libbmp_ident( state, -1 );

		_libbmp_print_header( state, "ciexyzGreen" );
		_libbmp_ident( state, +1 );
			_libbmp_print_uint32( state, "ciexyzX",	bih->biV4Endpoints.ciexyzGreen.ciexyzX, nullptr );
			_libbmp_print_uint32( state, "ciexyzX",	bih->biV4Endpoints.ciexyzGreen.ciexyzY, nullptr );
			_libbmp_print_uint32( state, "ciexyzX",	bih->biV4Endpoints.ciexyzGreen.ciexyzZ, nullptr );
		_libbmp_ident( state, -1 );

		_libbmp_print_header( state, "ciexyzBlue" );
		_libbmp_ident( state, +1 );
			_libbmp_print_uint32( state, "ciexyzX",	bih->biV4Endpoints.ciexyzBlue.ciexyzX, nullptr );
			_libbmp_print_uint32( state, "ciexyzX",	bih->biV4Endpoints.ciexyzBlue.ciexyzY, nullptr );
			_libbmp_print_uint32( state, "ciexyzX",	bih->biV4Endpoints.ciexyzBlue.ciexyzZ, nullptr );
		_libbmp_ident( state, -1 );
	_libbmp_ident( state, -1 );

	_libbmp_print_uint32( state, "biV4GammaRed",	bih->biV4GammaRed,		nullptr );
	_libbmp_print_uint32( state, "biV4GammaGreen",	bih->biV4GammaGreen,	nullptr );
	_libbmp_print_uint32( state, "biV4GammaBlue",	bih->biV4GammaBlue,		nullptr );

	// Exit
	return F2_STATUS_SUCCESS;
}
f2_status_t _libbmp_dump_header_BITMAPV5INFOHEADER( libbmp_bmp_dump_state * state, const BITMAPV5INFOHEADER * bih )
{
	_libbmp_dump_header_BITMAPV4INFOHEADER( state, (const BITMAPV4INFOHEADER *) bih );

	_libbmp_print_uint32( state, "bV5Intent",		bih->bV5Intent,			nullptr );
	_libbmp_print_uint32( state, "bV5ProfileData",	bih->bV5ProfileData,	nullptr );
	_libbmp_print_uint32( state, "bV5ProfileSize",	bih->bV5ProfileSize,	nullptr );
	_libbmp_print_uint32( state, "bV5Reserved",		bih->bV5Reserved,		nullptr );

	return F2_STATUS_SUCCESS;
}

#if !LIBBMP_CFG_NO_OS2_SUPPORT

f2_status_t _libbmp_dump_header_OS21XBITMAPHEADER( libbmp_bmp_dump_state * state, const OS21XBITMAPHEADER * bih )
{
	_libbmp_print_uint32( state, "Size",			bih->Size,			nullptr );
	_libbmp_print_uint16( state, "Width",			bih->Width,			nullptr );
	_libbmp_print_uint16( state, "Height",			bih->Height,		nullptr );
	_libbmp_print_uint16( state, "NumPlanes",		bih->NumPlanes,		nullptr );
	_libbmp_print_uint16( state, "BitsPerPixel",	bih->BitsPerPixel,	nullptr );

	state->width	= bih->Width;
	state->height	= bih->Height;
	state->pitch	= (bih->Width * bih->BitsPerPixel + 15) / 8 / bih->NumPlanes;	// align to 16 bit

	state->colors_used	= 1 << bih->BitsPerPixel;
	state->pal_elem_size= 3;
	state->color_table	= PCB(state->context->data) + sizeof(BITMAPFILEHEADER) + bih->Size;

	return F2_STATUS_SUCCESS;
}
f2_status_t _libbmp_dump_header_OS22XBITMAPHEADER( libbmp_bmp_dump_state * state, const OS22XBITMAPHEADER * bih )
{
	_libbmp_print_uint32( state, "Size",			bih->cbFix,			"Size of bitmap header, in bytes" );
	_libbmp_print_uint32( state, "Width",			bih->cx,			"Bitmap width, in pixels" );
	_libbmp_print_uint32( state, "Height",			bih->cy,			"Bitmap height, in pixels" );
	_libbmp_print_uint16( state, "NumPlanes",		bih->cPlanes,		"Number of color planes" );
	_libbmp_print_uint16( state, "BitsPerPixel",	bih->cBitCount,		"Number of bits per pixel" );

	_libbmp_print_uint32( state, "Compression",		bih->ulCompression,		"Bitmap compression scheme" );
	_libbmp_print_uint32( state, "ImageDataSize",	bih->cbImage,			"Size of bitmap data in bytes" );
	_libbmp_print_uint32( state, "XResolution",		bih->cxResolution,		"X resolution of display device" );
	_libbmp_print_uint32( state, "YResolution",		bih->cyResolution,		"Y resolution of display device" );
	_libbmp_print_uint32( state, "ColorsUsed",		bih->cclrUsed,			"Number of color table indices used" );
	_libbmp_print_uint32( state, "ColorsImportant",	bih->cclrImportant,		"Number of important color indices" );
	_libbmp_print_uint16( state, "Units",			bih->usUnits,			"Type of units used to measure resolution" );
	_libbmp_print_uint16( state, "Reserved",		bih->usReserved,		"Pad structure to 4-byte boundary" );
	_libbmp_print_uint16( state, "Recording",		bih->usRecording,		"Recording algorithm" );
	_libbmp_print_uint16( state, "Rendering",		bih->usRendering,		"Halftoning algorithm used" );
	_libbmp_print_uint32( state, "Size1",			bih->cSize1,			"Reserved for halftoning algorithm use" );
	_libbmp_print_uint32( state, "Size2",			bih->cSize2,			"Reserved for halftoning algorithm use" );
	_libbmp_print_uint32( state, "ColorEncoding",	bih->ulColorEncoding,	"Color model used in bitmap" );
	_libbmp_print_uint32( state, "Identifier",		bih->ulIdentifier,		"Reserved for application use" );

	state->width	= bih->cx;
	state->height	= bih->cy;
	state->pitch	= (bih->cy * bih->cBitCount + 15) / 8 / bih->cPlanes;	// align to 16 bit

	state->colors_used	= bih->cclrUsed;
	state->pal_elem_size= 3;
	state->color_table	= PCB(state->context->data) + sizeof(BITMAPFILEHEADER) + bih->cbFix;

	return F2_STATUS_SUCCESS;
}

#endif // !LIBBMP_CFG_NO_OS2_SUPPORT

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

f2_status_t _libbmp_dump_file_header( libbmp_bmp_dump_state * state )
{
	// Dump file header
	const BITMAPFILEHEADER * file_header = state->file_header;
	_libbmp_print_header( state, "BITMAPFILEHEADER" );
	_libbmp_ident( state, +1 );
		_libbmp_print_string( state, "bfType",		(const char *)&file_header->bfType, 2, nullptr );
		_libbmp_print_uint32( state, "bfSize",		file_header->bfSize, nullptr );
		_libbmp_print_uint16( state, "bfReserved1",	file_header->bfReserved1, nullptr );
		_libbmp_print_uint16( state, "bfReserved2",	file_header->bfReserved2, nullptr );
		_libbmp_print_uint32( state, "bfOffBits",	file_header->bfOffBits, nullptr );
	_libbmp_ident( state, -1 );

	// Exit
	return F2_STATUS_SUCCESS;
}
f2_status_t _libbmp_dump_bitmap_header( libbmp_bmp_dump_state * state )
{
	const unsigned char *	data = (const unsigned char *) state->context->data;
//	size_t					data_size = state->context->data_size;
	const COMMONBITMAPHEADER * bitmap_header;

	// Offset to the bitmap header
	if( state->file_header == &state->default_file_header )
		bitmap_header = (const COMMONBITMAPHEADER *) data;
	else
		bitmap_header = (const COMMONBITMAPHEADER *) (data + sizeof(BITMAPFILEHEADER));

	// Dump bitmap header
	switch( bitmap_header->WinBitmapHeader.biSize ) {
	// Windows common formats
	case sizeof(BITMAPINFOHEADER):
		_libbmp_print_header( state, "BITMAPINFOHEADER" );
		_libbmp_ident( state, +1 );
			_libbmp_dump_header_BITMAPINFOHEADER( state, (BITMAPINFOHEADER *) bitmap_header );
		_libbmp_ident( state, -1 );
		break;
	case sizeof(BITMAPV2INFOHEADER):
		_libbmp_print_header( state, "BITMAPV2INFOHEADER" );
		_libbmp_ident( state, +1 );
			_libbmp_dump_header_BITMAPV2INFOHEADER( state, (BITMAPV2INFOHEADER *) bitmap_header );
		_libbmp_ident( state, -1 );
		break;
	case sizeof(BITMAPV3INFOHEADER):
		_libbmp_print_header( state, "BITMAPV3INFOHEADER" );
		_libbmp_ident( state, +1 );
			_libbmp_dump_header_BITMAPV3INFOHEADER( state, (BITMAPV3INFOHEADER *) bitmap_header );
		_libbmp_ident( state, -1 );
		break;
	case sizeof(BITMAPV4INFOHEADER):
		_libbmp_print_header( state, "BITMAPV4INFOHEADER" );
		_libbmp_ident( state, +1 );
			_libbmp_dump_header_BITMAPV4INFOHEADER( state, (BITMAPV4INFOHEADER *) bitmap_header );
		_libbmp_ident( state, -1 );
		break;
	case sizeof(BITMAPV5INFOHEADER):
		_libbmp_print_header( state, "BITMAPV5INFOHEADER" );
		_libbmp_ident( state, +1 );
			_libbmp_dump_header_BITMAPV5INFOHEADER( state, (BITMAPV5INFOHEADER *) bitmap_header );
		_libbmp_ident( state, -1 );
		break;
	// OS/2 compatible formats
#if !LIBBMP_CFG_NO_OS2_SUPPORT
//	case sizeof(OS21XBITMAPHEADER):
//		_libbmp_print_header( state, "OS21XBITMAPHEADER" );
//		_libbmp_ident( state, +1 );
//			_libbmp_dump_header_OS21XBITMAPHEADER( state, (OS21XBITMAPHEADER *) bitmap_header );
//		_libbmp_ident( state, -1 );
//		break;
	case sizeof(OS22XBITMAPHEADER):
		_libbmp_print_header( state, "OS22XBITMAPHEADER" );
		_libbmp_ident( state, +1 );
			_libbmp_dump_header_OS22XBITMAPHEADER( state, (OS22XBITMAPHEADER *) bitmap_header );
		_libbmp_ident( state, -1 );
		break;
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
	// Other variants
#if !LIBBMP_CFG_NO_LEGACY_SUPPORT
	case sizeof(BITMAPOLDHEADER):	// old bitmap 16-bit format
		_libbmp_print_header( state, "BITMAPOLDHEADER" );
		_libbmp_ident( state, +1 );
			_libbmp_dump_header_BITMAPOLDHEADER( state, (const BITMAPOLDHEADER *) bitmap_header );
		_libbmp_ident( state, -1 );
		break;
	case sizeof(BITMAPCOREHEADER):	// OS/2 and Windows compatible format(s)
		if( state->context->flags & LIBBMP_DUMP_PREFER_OS2 )
		{
			_libbmp_print_header( state, "OS21XBITMAPHEADER" );
			_libbmp_ident( state, +1 );
				_libbmp_dump_header_OS21XBITMAPHEADER( state, (OS21XBITMAPHEADER *) bitmap_header );
			_libbmp_ident( state, -1 );
		}
		else
		{
			_libbmp_print_header( state, "BITMAPCOREHEADER" );
			_libbmp_ident( state, +1 );
				_libbmp_dump_header_BITMAPCOREHEADER( state, (BITMAPCOREHEADER *) bitmap_header );
			_libbmp_ident( state, -1 );
		}
		break;
#endif // !LIBBMP_CFG_NO_LEGACY_SUPPORT
	default:
		break;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t _libbmp_dump_color_table( libbmp_bmp_dump_state * state )
{
	unsigned index;
	char buf[32];

	// Check current state
	if( nullptr == state->color_table || 0 == state->colors_used )
		return F2_STATUS_SUCCESS;

	// Dump palette
	if( 3 == state->pal_elem_size )
	{
		const RGBTRIPLE * rgbt = (const RGBTRIPLE *) state->color_table;

		_libbmp_printf_header( state, "COLOR TABLE: RGBTRIPLE[%d]", state->colors_used );
		_libbmp_ident( state, +1 );
		for( index = 0; index < state->colors_used; ++ index, ++ rgbt )
		{
			_snprintf( buf, _countof(buf), "%3d:", index );
			_libbmp_printf_string( state, buf, "R:%02X G:%02X B:%02X",
				rgbt->rgbtRed, rgbt->rgbtGreen, rgbt->rgbtBlue );
		}
		_libbmp_ident( state, -1 );
	}
	else if( 4 == state->pal_elem_size )
	{
		const RGBQUAD * rgbq = (const RGBQUAD *) state->color_table;

		_libbmp_printf_header( state, "COLOR TABLE: RGBQUAD[%d]", state->colors_used );
		_libbmp_ident( state, +1 );
		for( index = 0; index < state->colors_used; ++ index, ++ rgbq )
		{
			_snprintf( buf, _countof(buf), "%3d:", index );
			_libbmp_printf_string( state, buf, "R:%02X G:%02X B:%02X Q:%02X",
				rgbq->rgbRed, rgbq->rgbGreen, rgbq->rgbBlue, rgbq->rgbReserved );
		}
		_libbmp_ident( state, -1 );
	}
	else
	{
		f2_assert( !"state->pal_elem_size != [3; 4]" );
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t _libbmp_dump_data( libbmp_bmp_dump_state * state )
{
	// Dump pixels
	_libbmp_printf_header( state, "PIXEL DATA: width:%d height:%d pitch:%d", state->width, state->height, state->pitch );
	_libbmp_ident( state, +1 );
	if( state->context->flags & LIBBMP_DUMP_PIXELS )
	{
		unsigned x, y;
		//const uint8_t * row = (const uint8_t *) state->data;

		for( y = 0; y < state->height; ++ y )
		{
			for( x = 0; x < state->width; ++ x )
			{
				// TODO
			}
		}
	}
	else
	{
		unsigned y, pos, col;
		char buf[128];
		const uint8_t * row = (const uint8_t *) state->data;

		for( y = 0; y < state->height; ++ y )
		{
			for( pos = 0; pos < state->pitch; pos += 8 )
			{
				size_t len = 0;
				const unsigned col_size = min(24
					, state->pitch - pos);
				const uint8_t * ptr = row;
				for( col = 0; col < col_size; ++ col, ++ ptr )
					len += _snprintf( buf + len, _countof(buf), "%02X ", *ptr );
				_libbmp_print_string( state, nullptr, buf, len, nullptr );
			}
			row += state->pitch;
		}
	}
	_libbmp_ident( state, -1 );

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t _libbmp_dump_gaps( libbmp_bmp_dump_state * state )
{
	unsigned gap0_offset = 0;
	unsigned gap1_offset = 0;

	// Dump gap between headers/palette and data
	if( nullptr != state->file_header )
		gap0_offset += sizeof(BITMAPFILEHEADER);
	gap0_offset += state->bitmap_header->WinBitmapHeader.biSize;

	if( BI_BITFIELDS == state->bitmap_header->WinBitmapHeader.biCompression )
		gap0_offset += 3 * sizeof(DWORD);
	else if( BI_ALPHABITFIELDS == state->bitmap_header->WinBitmapHeader.biCompression )
		gap0_offset += 4 * sizeof(DWORD);

	gap0_offset += state->pal_elem_size * state->colors_used;

	if( gap0_offset < state->file_header->bfOffBits )
	{
		_libbmp_printf_header( state, "GAP0:" );
		dump_bytes( state, gap0_offset, state->file_header->bfOffBits - gap0_offset );
	}

	// Dump data after file
	gap1_offset = state->file_header->bfOffBits + state->pitch*state->height;
	if( state->context->data_size > gap1_offset )
	{
		_libbmp_printf_header( state, "GAP1:" );
		dump_bytes( state, gap1_offset, state->context->data_size - gap1_offset );
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

f2_status_t libbmp_dump_bmp( const libbmp_dump_context * context )
{
	f2_status_t			status;
	libbmp_bmp_dump_state	state;

	// Check current state
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == context->data )
		return F2_STATUS_ERROR_INVALID_STATE;

	memset( &state, 0, sizeof(state) );
	state.context = context;

	// Pre-parse file
	state.file_header = (const BITMAPFILEHEADER *) context->data;
	if( LIBBMP_FILETYPE_BITMAP != state.file_header->bfType )
	{
		state.file_header = &state.default_file_header;
		state.default_file_header.bfType = LIBBMP_FILETYPE_BITMAP;
		state.default_file_header.bfSize = (DWORD) context->data_size;
		state.default_file_header.bfReserved1 = 0;
		state.default_file_header.bfReserved2 = 0;
		state.default_file_header.bfOffBits = 0;
	}
	state.bitmap_header = (const COMMONBITMAPHEADER *) &state.file_header[1];

	// Dump bitmap_header
	if( context->flags & LIBBMP_DUMP_FILE_HEADER && state.file_header != &state.default_file_header ) {
		status = _libbmp_dump_file_header( &state );
		if( f2_failed(status) )
			return status;
	}
	if( context->flags & LIBBMP_DUMP_BITMAP_HEADER ) {
		status = _libbmp_dump_bitmap_header( &state );
		if( f2_failed(status) )
			return status;
	}

	// Dump palette
	if( context->flags & LIBBMP_DUMP_COLOR_TABLE ) {
		if( 0 != state.colors_used )
		{
			status = _libbmp_dump_color_table( &state );
			if( f2_failed(status) )
				return status;
		}
	}

	// Dump data
	if( context->flags & LIBBMP_DUMP_IMAGE_BITS ) {
		status = _libbmp_dump_data( &state );
		if( f2_failed(status) )
			return status;
	}

	// Dump gaps
	if( context->flags & LIBBMP_DUMP_GAPS ) {
		status = _libbmp_dump_gaps( &state );
		if( f2_failed(status) )
			return status;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

/*END OF dump.c*/
