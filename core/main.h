/*main.h*/

#include <stdio.h>	// for NULL

#include "../include/libbmp.h"
#include "./bmpdefs.h"

#define CHECK_BIT_COUNT(bc)		( 1 == (bc) || 2 == (bc) || 4 == (bc) || 8 == (bc) || 15 == (bc) || 16 == (bc) || 24 == (bc) || 32 == (bc) || 48 == (bc) || 64 == (bc) )
#define LIBBMP_MAX_PLANES		64

/* Color space filter */
typedef long					fixed_2_30_t;
typedef struct ciexyz_t			ciexyz_t;
typedef struct icexyz_triple_t	icexyz_triple_t;
struct ciexyz_t
{
	fixed_2_30_t	ciexyz_x;
	fixed_2_30_t	ciexyz_y;
	fixed_2_30_t	ciexyz_z;
};
struct icexyz_triple_t
{
	ciexyz_t	ciexyz_red;
	ciexyz_t	ciexyz_green;
	ciexyz_t	ciexyz_blue;
};
enum libbmp_colorspace_filter_type_t
{
	libbmp_colorspace_filter_none,
	libbmp_colorspace_filter_calibrated,
	libbmp_colorspace_filter_external,
	libbmp_colorspace_filter_embedded,
	libbmp_colorspace_filter_sRGB,
	libbmp_colorspace_filter_windows,
	libbmp_colorspace_filter_unknown	= -1
};
typedef enum libbmp_colorspace_filter_type_t	libbmp_colorspace_filter_type_t;
struct libbmp_colorspace_filter {
	libbmp_colorspace_filter_type_t	cs_type;
	union {
		struct {
			icexyz_triple_t endpoints;
			double	gamma_red;
			double	gamma_green;
			double	gamma_blue;
		} calibrated;
		struct {
			unsigned	profile_data_offset;
			unsigned	profile_data_size;
		} embedded;
		struct {
			const char*	profile_file_name;
			unsigned	profile_file_name_size;
		} external;
	} profile;
};
typedef struct libbmp_colorspace_filter	libbmp_colorspace_filter;

#define FIXED1616_TO_DOUBLE(fx32)		( (fx32) * 1.0/65536.0 )


typedef struct libbmp_bmp_reader_state libbmp_bmp_reader_state;
typedef struct libbmp_bmp_writer_state libbmp_bmp_writer_state;

typedef struct libbmp_buffer {
	const void *			in_data;		// initial pointer to data
	size_t					in_data_size;	// initial size
	const unsigned char *	data;			// pointer to current location
	size_t					size_left;		// size left
} libbmp_buffer;

typedef f2_status_t (*libbmp_decompress_func_t)( libbmp_bmp_reader_state * state );
typedef f2_status_t (*libbmp_compress_func_t)( libbmp_bmp_writer_state * state );

struct libbmp_bmp_reader_state {
	const libbmp_bmp_reader_context * context;

	const uint8_t *	ptr;
	size_t			size_left;

	void *			dst_buf;
	unsigned		dst_pitch;

	libbmp_image_descriptor image_desc;

	BITMAPFILEHEADER	default_file_header;	// used for headerless DIB files
	BITMAPFILEHEADER *	file_header;
	COMMONBITMAPHEADER *bitmap_header;

	DWORD data_offset;
	libbmp_decompress_func_t	compression;
	DWORD planes;
	DWORD data_size;
	DWORD pal_elem_size;

	DWORD colors_used;
	libbmp_rgbx	* palette;

	DWORD intent;
	DWORD profile_offset;
	DWORD profile_size;

	libbmp_colorspace_filter	colorspace_filter;
};

f2_status_t _libbmp_read_header( libbmp_bmp_reader_state * state );

typedef struct libbmp_bmp_writer_state {
	const libbmp_bmp_writer_context * context;

	void *		buffer;			// output buffer
	size_t		buffer_size;	// size of the memory requested for output buffer
	uint8_t *	ptr;			// output cursor
	unsigned	pitch;

	libbmp_file_format_t	file_format;		// evaluated file format
	DWORD					compression;		// evaluated compression type
	libbmp_compress_func_t	compression_func;

	BITMAPFILEHEADER *	file_header;
	COMMONBITMAPHEADER *bitmap_header;
} libbmp_bmp_writer_state;

libbmp_decompress_func_t _os2_decompression( DWORD compression );
libbmp_decompress_func_t _win_decompression( DWORD compression );
libbmp_compress_func_t _os2_compression( DWORD compression );
libbmp_compress_func_t _win_compression( DWORD compression );
DWORD _os2_compression_from_id( libbmp_compression_t id, unsigned bits_per_pixel );
DWORD _win_compression_from_id( libbmp_compression_t id, unsigned bits_per_pixel );

f2_status_t _libbmp_decompress_none( libbmp_bmp_reader_state * state );
f2_status_t _libbmp_decompress_rle4( libbmp_bmp_reader_state * state );
f2_status_t _libbmp_decompress_rle8( libbmp_bmp_reader_state * state );
f2_status_t _libbmp_decompress_rle24( libbmp_bmp_reader_state * state );
f2_status_t _libbmp_decompress_bitfields( libbmp_bmp_reader_state * state );
f2_status_t _libbmp_decompress_alpha_bitfields( libbmp_bmp_reader_state * state );
f2_status_t _libbmp_decompress_jpeg( libbmp_bmp_reader_state * state );
f2_status_t _libbmp_decompress_png( libbmp_bmp_reader_state * state );
f2_status_t _libbmp_decompress_huffman1d( libbmp_bmp_reader_state * state );

f2_status_t _libbmp_compress_none( libbmp_bmp_writer_state * state );
f2_status_t _libbmp_compress_rle4( libbmp_bmp_writer_state * state );
f2_status_t _libbmp_compress_rle8( libbmp_bmp_writer_state * state );
f2_status_t _libbmp_compress_rle24( libbmp_bmp_writer_state * state );
f2_status_t _libbmp_compress_bitfields( libbmp_bmp_writer_state * state );
f2_status_t _libbmp_compress_alpha_bitfields( libbmp_bmp_writer_state * state );
f2_status_t _libbmp_compress_jpeg( libbmp_bmp_writer_state * state );
f2_status_t _libbmp_compress_png( libbmp_bmp_writer_state * state );
f2_status_t _libbmp_compress_huffman1d( libbmp_bmp_writer_state * state );

typedef struct libbmp_copy_context {
	const void *	in_buf;
	unsigned		in_pitch;

	void *			out_buf;
	unsigned		out_pitch;

	unsigned		row_count;
} libbmp_copy_context;
f2_status_t _libbmp_copy_image( const libbmp_copy_context * copy_context );

f2_status_t _libbmp_process_decoder( libbmp_encoder * encoder, libbmp_bmp_reader_state * state );
f2_status_t _libbmp_process_encoder( libbmp_encoder * encoder, libbmp_bmp_writer_state * state );

/*END OF main.h*/
