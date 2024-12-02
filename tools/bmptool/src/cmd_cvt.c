/*cmd_cvt.c*/
#include "pch.h"
#include "main.h"

typedef struct image_data {
	libbmp_rgbx	palette[256];
	void *	image_data;
	size_t	image_data_size;
} image_data;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct bitmap_object
{
	void *	data;
	libbmp_image_descriptor desc;
} bitmap_object;

typedef struct test_obitmap
{
	libbmp_obitmap	obitmap;
	bitmap_object *	bitmap;
} test_obitmap;
f2_status_t _obitmap_set_desc( libbmp_obitmap * thisp, libbmp_image_descriptor * desc )
{
	test_obitmap * obitmap = (test_obitmap *) thisp;
	obitmap->bitmap->desc = *desc;
	return F2_STATUS_SUCCESS;
}
f2_status_t _obitmap_get_buffer( libbmp_obitmap * thisp, const void ** data, unsigned * pitch )
{
	test_obitmap * obitmap = (test_obitmap *) thisp;

	size_t size = obitmap->bitmap->desc.pitch * obitmap->bitmap->desc.height;
	*data = obitmap->bitmap->data = malloc( size );
	*pitch = obitmap->bitmap->desc.pitch;
	return F2_STATUS_SUCCESS;
}

typedef struct test_ibitmap
{
	libbmp_ibitmap	ibitmap;
	bitmap_object *	bitmap;
} test_ibitmap;
f2_status_t _ibitmap_get_buffer( libbmp_ibitmap * thisp, const void ** data, size_t size )
{
	return F2_STATUS_SUCCESS;
}
f2_status_t _ibitmap_set_buffer_size( libbmp_ibitmap * thisp, size_t size )
{
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct palette_object
{
	libbmp_rgbx *	palette;
	unsigned		count;
} palette_object;
f2_status_t _palette_set_count( palette_object * thisp, unsigned count )
{
	thisp->palette = (libbmp_rgbx *) malloc( count * sizeof(libbmp_rgbx) );
	thisp->count = count;
	return F2_STATUS_SUCCESS;
}
f2_status_t _palette_set_entries( palette_object * thisp, const libbmp_rgbx * color, unsigned start_index, unsigned count )
{
	memcpy( &thisp->palette + start_index, color, count*sizeof(libbmp_rgbx) );
	return F2_STATUS_SUCCESS;
}
f2_status_t _palette_get_entries( palette_object * thisp, libbmp_rgbx * color, unsigned start_index, unsigned count )
{
	memcpy( color, &thisp->palette + start_index, count*sizeof(libbmp_rgbx) );
	return F2_STATUS_SUCCESS;
}


typedef struct test_opalette
{
	libbmp_opalette opalette;
	palette_object *	palette;
} test_opalette;
f2_status_t _opalette_set_count( libbmp_opalette * thisp, unsigned count )
{
	test_opalette * opalette = (test_opalette *) thisp;
	return _palette_set_count( opalette->palette, count );
}
f2_status_t _opalette_set_entries( libbmp_opalette * thisp, const libbmp_rgbx * color, unsigned start_index, unsigned count )
{
	test_opalette * opalette = (test_opalette *) thisp;
	return _palette_set_entries( opalette->palette, color, start_index, count );
}

typedef struct test_ipalette
{
	libbmp_ipalette		ipalette;
	palette_object *	palette;
} test_ipalette;
f2_status_t _ipalette_get_count( libbmp_ipalette * thisp, unsigned * count )
{
	test_ipalette * ipalette = (test_ipalette *) thisp;
	*count = ipalette->palette->count;
	return F2_STATUS_SUCCESS;
}
f2_status_t _ipalette_get_entries( libbmp_ipalette * thisp, libbmp_rgbx * color, unsigned start_index, unsigned count )
{
	test_ipalette * ipalette = (test_ipalette *) thisp;
	return _palette_get_entries( ipalette->palette, color, start_index, count );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

status_t cmd_convert( const char * file_name )
{
	f2_status_t	status;
	bitmap_object	bitmap = { nullptr };
	palette_object	palette = { nullptr, 0 };
	libbmp_bmp_reader_context	reader_context;
	test_obitmap	obitmap;
	test_opalette	opalette;
	libbmp_bmp_writer_context	writer_context;
	test_ibitmap	ibitmap;
	test_ipalette	ipalette;

	// Load image
	memset( &obitmap, 0, sizeof(obitmap) );
	obitmap.bitmap = &bitmap;
	obitmap.obitmap.set_desc = _obitmap_set_desc;
	obitmap.obitmap.get_buffer = _obitmap_get_buffer;

	memset( &opalette, 0, sizeof(opalette) );
	opalette.palette = &palette;
	opalette.opalette.set_count = _opalette_set_count;
	opalette.opalette.set_entries = _opalette_set_entries;

	memset( &reader_context, 0, sizeof(reader_context) );
	reader_context.bitmap = &obitmap.obitmap;
	reader_context.palette = &opalette.opalette;

	if( 0 != _fload( file_name, (void *) &reader_context.data, &reader_context.data_size ) )
	{
		printf( "*** error: cannot load: %s\n", file_name );
		return STATUS_ERROR_NOT_FOUND;
	}

	status = libbmp_read_bmp( &reader_context );
	if( f2_failed(status) )
	{
		printf( "*** error: failed reading: %d\n", status );
		return STATUS_SUCCESS;
	}

	// Write itmap
	memset( &ibitmap, 0, sizeof(ibitmap) );
	ibitmap.bitmap = &bitmap;
	ibitmap.ibitmap.get_buffer = _ibitmap_get_buffer;
	ibitmap.ibitmap.set_buffer_size = _ibitmap_set_buffer_size;

	memset( &ipalette, 0, sizeof(ipalette) );
	ipalette.palette = &palette;
	ipalette.ipalette.get_count = _ipalette_get_count;
	ipalette.ipalette.get_entries = _ipalette_get_entries;

	memset( &writer_context, 0, sizeof(writer_context) );
	writer_context.bitmap = &ibitmap.ibitmap;
	writer_context.palette = &ipalette.ipalette;
	writer_context.image_data = reader_context.data;
	writer_context.image_data_size = reader_context.data_size;

	status = libbmp_write_bmp( &writer_context );

	// Clean up
	free( (void *) reader_context.data );reader_context.data = NULL;
	free( obitmap.bitmap->data );		obitmap.bitmap->data  = NULL;
	free( opalette.palette->palette );	opalette.palette->palette = NULL;

	// Exit
	return STATUS_SUCCESS;
}

/*END OF cmd_cvt.c*/
