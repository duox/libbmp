/*fileio.c*/
#include "pch.h"
#include "main.h"

long _ffilelength( FILE * f )
{
	long old_pos;
	long len;

	old_pos = ftell( f );
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	fseek( f, old_pos, SEEK_SET );

	return len;
}

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

status_t _fload( const char * fname, void ** data_ptr, size_t * data_size )
{
	FILE * f;
	size_t flen;
	void * buf;
	size_t nread;

	if( nullptr == data_ptr )
		return STATUS_ERROR_INVALID_PARAMETER;

	errno = 0;
	f = fopen( fname, "r" );
	if( nullptr == f )
		return STATUS_ERROR_NOT_FOUND;
	flen = _ffilelength( f );

	buf = malloc( flen + 2 );
	if( nullptr == buf )
	{
		fclose( f);
		return STATUS_ERROR_INSUFFICIENT_MEMORY;
	}

	nread = fread( buf, 1, flen, f );
	fclose( f );
	if( 0 == nread && !feof( f ) )
	{
		free( buf );
		return STATUS_ERROR_READING;
	}
	if( 0 != nread )
		flen = (long) nread;
	/*if( nread != (size_t) flen )
	{
		free( buf );
		return STATUS_ERROR_READING;
	}*/

	((char *) buf)[flen + 0] = 0;
	((char *) buf)[flen + 1] = 0;

	*data_ptr = buf;
	if( nullptr != data_size )
		*data_size = flen;

	return STATUS_SUCCESS;
}

/*END OF fileio.c*/
