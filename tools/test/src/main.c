/*main.c*/
#include "pch.h"
#include "main.h"

int load( const char * name, void ** buf_ptr, size_t * size_ptr )
{
	int h;
	long size;
	void * buf;
	int nread;

	h = _open( name, _O_RDONLY|_O_BINARY );
	if( -1 == h )
	{
		printf( "*** error: file was not found: %s\n", name );
		return -1;
	}

	size = _filelength( h );
	if( -1 == size )
	{
		printf( "*** error: error querying file size: %s\n", name );
		_close( h );
		return -1;
	}

	buf = malloc( size + 1 );
	if( NULL == buf )
	{
		printf( "*** error: cannot allocate %d bytes: %s\n", size, name );
		_close( h );
		return -1;
	}

	nread = read( h, buf, size );
	_close( h );
	if( size != nread )
	{
		printf( "*** error: error reading: %s\n", name );
		return -1;
	}

	*buf_ptr = buf;
	*size_ptr = size;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct test_obitmap
{
	libbmp_obitmap obitmap;
	libbmp_image_descriptor desc;
	void *	data;
};
typedef struct test_obitmap	test_obitmap;

libbmp_status_t _obitmap_set_desc( libbmp_obitmap * thisp, libbmp_image_descriptor * desc )
{
	test_obitmap * obitmap = (test_obitmap *) thisp;
	obitmap->desc = *desc;
	return LIBBMP_STATUS_SUCCESS;
}
libbmp_status_t _obitmap_get_buffer( libbmp_obitmap * thisp, const void ** data, unsigned * pitch )
{
	test_obitmap * obitmap = (test_obitmap *) thisp;

	size_t size = obitmap->desc.pitch * obitmap->desc.height;
	*data = obitmap->data = malloc( size );
	*pitch = obitmap->desc.pitch;
	return LIBBMP_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct test_opalette
{
	libbmp_opalette opalette;
	libbmp_rgbx *	palette;
	unsigned		count;
};
typedef struct test_opalette	test_opalette;

libbmp_status_t _opalette_set_count( libbmp_opalette * thisp, unsigned count )
{
	test_opalette * opalette = (test_opalette *) thisp;
	opalette->palette = (libbmp_rgbx *) malloc( count * sizeof(libbmp_rgbx) );
	opalette->count = count;
	return LIBBMP_STATUS_SUCCESS;
}
libbmp_status_t _opalette_set_entries( libbmp_opalette * thisp, const libbmp_rgbx * color, unsigned start_index, unsigned count )
{
	test_opalette * opalette = (test_opalette *) thisp;
	memcpy( &opalette->palette + start_index, color, count*sizeof(libbmp_rgbx) );
	return LIBBMP_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const struct test_file_t {
	const char * name;
} test_file[] = {
	"d:\\Projects\\freetools\\third-party\\libbmp\\archives\\libbmp\\data\\g\\rgb24.bmp"
};

int main( void )
{
	size_t i;
	libbmp_status_t	status;
	libbmp_bmp_reader_context	reader_context;
	test_obitmap	obitmap;
	test_opalette	opalette;

	memset( &obitmap, 0, sizeof(obitmap) );
	obitmap.obitmap.set_desc = _obitmap_set_desc;
	obitmap.obitmap.get_buffer = _obitmap_get_buffer;

	memset( &opalette, 0, sizeof(opalette) );
	opalette.opalette.set_count = _opalette_set_count;
	opalette.opalette.set_entries = _opalette_set_entries;

	reader_context.bitmap = &obitmap.obitmap;
	reader_context.palette = &opalette.opalette;

	for( i = 0; i < sizeof(test_file) / sizeof(*test_file); ++ i )
	{
		if( 0 != load( test_file[i].name, (void *) &reader_context.data, &reader_context.data_size ) )
			continue;

		// Read bitmap
		status = libbmp_read_bmp( &reader_context );
		if( libbmp_failed(status) )
			printf( "*** error: libbmp_read failed: %d\n", status );
		else
		{
			// Write bitmap
			// Compare buffers
		}

		// Clean up
		free( (void *) reader_context.data );reader_context.data = NULL;
		free( obitmap.data );		obitmap.data  = NULL;
		free( opalette.palette );	opalette.palette = NULL;
	}

#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#endif // def _MSC_VER
	return 0;
}

/*END OF main.c*/
