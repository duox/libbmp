/*alpha_bitfields.c*/
#include "../core/pch.h"
#include "../core/main.h"

f2_status_t _libbmp_decompress_alpha_bitfields( libbmp_bmp_reader_state * state )
{
	libbmp_copy_context	copy_context;
	size_t offset;
	const uint32_t * masks;
	libbmp_pixel_format * pf;
	DWORD expected_image_offset;

	state->image_desc.compression = libbmp_compression_none;

	// Check whether bitfields are present
	offset =
		sizeof(BITMAPFILEHEADER) +								// file header
		state->bitmap_header->WinBitmapHeader.biSize +			// bitmap header
		state->bitmap_header->WinBitmapHeader.biClrImportant*4;	// color table
	expected_image_offset = offset +
		4*sizeof(uint32_t);										// 4 color masks

	if( expected_image_offset != ((BITMAPFILEHEADER *) state->context->data)->bfOffBits )
		return F2_STATUS_ERROR_INVALID_DATA;

	// Read bit masks
	masks = (uint32_t *) (PB(state->ptr) + offset);

	pf = (libbmp_pixel_format *) &state->image_desc.pixel_format;
	f2_assert( 0 == pf->r_mask || pf->r_mask = masks[0] );
	f2_assert( 0 == pf->g_mask || pf->g_mask = masks[1] );
	f2_assert( 0 == pf->b_mask || pf->b_mask = masks[2] );
	f2_assert( 0 == pf->a_mask || pf->a_mask = masks[3] );
	pf->r_mask = masks[0];
	pf->g_mask = masks[1];
	pf->b_mask = masks[2];
	pf->a_mask = masks[3];

	// Now read the image and exit
	copy_context.in_buf = PB(state->ptr) + state->file_header->bfOffBits;
	copy_context.in_pitch = state->image_desc.pitch;
	copy_context.out_buf = state->dst_buf;
	copy_context.out_pitch = state->dst_pitch;
	copy_context.row_count = state->image_desc.height;

	return _libbmp_copy_image( &copy_context );
}
f2_status_t _libbmp_compress_alpha_bitfields( libbmp_bmp_writer_state * state )
{
	// Check current state
	const libbmp_pixel_format * pf = &state->context->image_desc.pixel_format;
	debugbreak_if( 16 != pf->bits_per_pixel && 24 != pf->bits_per_pixel && 32 != pf->bits_per_pixel )
		return F2_STATUS_ERROR_INVALID_STATE;	// only 32 bit images are supported

	// Copy uncompressed image
	return _libbmp_compress_none( state );
}

/*END OF alpha_bitfields.c*/
