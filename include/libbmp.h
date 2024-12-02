/*libbmp.h*/
/** @file
 *
 * @brief Lightweight BMP image file format library.
 */
#ifndef __libbmp__66E23F6E86534A7D9A559060C0F93F13__
#define __libbmp__66E23F6E86534A7D9A559060C0F93F13__

#include <libf2.h> 

#define libbmp_api

#include "libbmpcf.h" 

#pragma pack(push, 1)

typedef struct libbmp_rgb {
	uint8_t r, g, b;
} libbmp_rgb;
f2_static_assert(sizeof(libbmp_rgb) == 0x03);

typedef struct libbmp_rgbx {
	uint8_t r, g, b, x;
} libbmp_rgbx;
f2_static_assert(sizeof(libbmp_rgb) == 0x04);

// Image information
typedef enum libbmp_file_format_t {
	libbmp_file_format_BITMAPCOREHEADER,
	libbmp_file_format_BITMAPINFOHEADER,
	libbmp_file_format_BITMAPV2INFOHEADER,
	libbmp_file_format_BITMAPV3INFOHEADER,
	libbmp_file_format_BITMAPV4INFOHEADER,
	libbmp_file_format_BITMAPV5INFOHEADER,

	libbmp_file_format_OS2_BITMAPINFOHEADER1,
	libbmp_file_format_OS2_BITMAPINFOHEADER2,

	libbmp_file_format_max_count,
	libbmp_file_format_unspecified
} libbmp_file_format_t;
typedef enum libbmp_compression_t {
	libbmp_compression_none,
	libbmp_compression_rle,
	libbmp_compression_huffman,
	libbmp_compression_jpeg,
	libbmp_compression_png,

	libbmp_compression_max_count,
	libbmp_compression_unspecified
} libbmp_compression_t;

typedef struct libbmp_pixel_format {
	uint32_t	r_mask, g_mask, b_mask, a_mask, x_mask;
	uint16_t	bits_per_pixel;
	uint16_t	flags;				// currently unused; possibly future use is specifying colorspace type (e.g CMYK)
} libbmp_pixel_format;
f2_static_assert(sizeof(libbmp_pixel_format) == 0x18);

typedef struct libbmp_image_descriptor	libbmp_image_descriptor;
struct libbmp_image_descriptor {
	const char *format_string;		// format identifier string (statically allocated)

	#define LIBBMP_IMAGE_UPSIDEDOWN		0x0001
	#define LIBBMP_IMAGE_PALETTED		0x0002	///< image is paletted; a palette interface is required
	uint32_t	flags;

	uint32_t	width;
	uint32_t	height;
	uint32_t	pitch;

	libbmp_file_format_t	file_format;
	libbmp_compression_t	compression;
	libbmp_pixel_format		pixel_format;
};

// Error handler interface
typedef enum libbmp_diag_message_t {
	libbmp_diag_message_invalid_file_size,		///< file size is invalid as specified in the file header
	libbmp_diag_message_null_plane_count,		///< plane count is 0
	libbmp_diag_message_invalid_plane_count,	///< plane count is neither 0 nor power of two; param contains this invalid value
	libbmp_diag_message_count
} libbmp_diag_message_t;
typedef struct libbmp_diag		libbmp_diag;
typedef f2_status_t (*libbmp_diag_message)( libbmp_diag * thisp, libbmp_diag_message_t msg, size_t offset, size_t param );
struct libbmp_diag {
	unsigned	reporting_mask;
	libbmp_diag_message		message;
};

f2_status_t	libbmp_api libbmp_diagnostics( libbmp_diag * diag, f2_status_t status, libbmp_diag_message_t msg, size_t offset, size_t param );


// Input stream interfaces
/*typedef struct libbmp_istream libbmp_istream;
typedef f2_status_t (*libbmp_istream_read)( libbmp_istream *thisp, void * buffer, size_t size );
typedef f2_status_t (*libbmp_istream_seek)( libbmp_istream *thisp, unsigned offset );
typedef f2_status_t (*libbmp_istream_tell)( libbmp_istream *thisp, unsigned * offset );
struct libbmp_istream {
	libbmp_istream_read	read;
	libbmp_istream_seek	seek;
	libbmp_istream_tell	tell;
};*/

// Image data interfaces
typedef struct libbmp_obitmap libbmp_obitmap;
typedef f2_status_t (*libbmp_obitmap_set_desc)( libbmp_obitmap * thisp, libbmp_image_descriptor * desc );
typedef f2_status_t (*libbmp_obitmap_get_buffer)( libbmp_obitmap * thisp, const void ** data, unsigned * pitch );
struct libbmp_obitmap {
	libbmp_obitmap_set_desc		set_desc;
	libbmp_obitmap_get_buffer	get_buffer;
};

typedef struct libbmp_ibitmap libbmp_ibitmap;
typedef f2_status_t (*libbmp_ibitmap_get_buffer)( libbmp_ibitmap * thisp, const void ** data, size_t size );
typedef f2_status_t (*libbmp_ibitmap_set_buffer_size)( libbmp_ibitmap * thisp, size_t size );
struct libbmp_ibitmap {
	libbmp_ibitmap_get_buffer		get_buffer;			// get working buffer
	libbmp_ibitmap_set_buffer_size	set_buffer_size;	// set used buffer size
};

// Palette interfaces
typedef struct libbmp_opalette libbmp_opalette;
typedef f2_status_t (*libbmp_opalette_set_count)( libbmp_opalette * thisp, unsigned count );
typedef f2_status_t (*libbmp_opalette_set_entries)( libbmp_opalette * thisp, const libbmp_rgbx * color, unsigned start_index, unsigned count );
struct libbmp_opalette {
	libbmp_opalette_set_count	set_count;
	libbmp_opalette_set_entries	set_entries;
};

typedef struct libbmp_ipalette libbmp_ipalette;
typedef f2_status_t (*libbmp_ipalette_get_count)( libbmp_ipalette * thisp, unsigned * count );
typedef f2_status_t (*libbmp_ipalette_get_entries)( libbmp_ipalette * thisp, libbmp_rgbx * color, unsigned start_index, unsigned count );
struct libbmp_ipalette {
	libbmp_ipalette_get_count	get_count;
	libbmp_ipalette_get_entries	get_entries;
};

// Meta-data interfaces
typedef enum libbmp_metadata_t {
	// pels per meter (uint32)
	libbmp_metadata_xpels_per_meter,
	libbmp_metadata_ypels_per_meter,
	// colorspace type (bool)
	libbmp_metadata_colorspace_sRGB,
	libbmp_metadata_colorspace_calibrated_RGB,
	libbmp_metadata_colorspace_windows,
	// CIE endpoints (float64)
	libbmp_metadata_ciexyz_red_x,	libbmp_metadata_ciexyz_red_y,	libbmp_metadata_ciexyz_red_z,
	libbmp_metadata_ciexyz_green_x,	libbmp_metadata_ciexyz_green_y,	libbmp_metadata_ciexyz_green_z,
	libbmp_metadata_ciexyz_blue_x,	libbmp_metadata_ciexyz_blue_y,	libbmp_metadata_ciexyz_blue_z,
	// gamma values (float64)
	libbmp_metadata_gamma_red,
	libbmp_metadata_gamma_green,
	libbmp_metadata_gamma_blue,
	// bitmap usage intent (bool)
	libbmp_metadata_intent_abs_colometric,
	libbmp_metadata_intent_charts,
	libbmp_metadata_intent_graphics,
	libbmp_metadata_intent_images,
	// profile data (blob)
	libbmp_metadata_icm_profile_data,
	libbmp_metadata_icm_profile_file,
	// gap between header/palette and image data (blob)
	libbmp_metadata_header_image_gap,
	// gap between image data and end of file (blob)
	libbmp_metadata_image_eof_gap,
	// special values
	libbmp_metadata_max_value
} libbmp_metadata_t;
typedef enum libbmp_type_t {
	libbmp_type_null,
	libbmp_type_int8,	libbmp_type_int16,	libbmp_type_int32,
	libbmp_type_uint8,	libbmp_type_uint16,	libbmp_type_uint32,
	libbmp_type_float32,libbmp_type_float64,
	libbmp_type_bool,
	libbmp_type_blob,
	libbmp_type_max_value
} libbmp_type_t;

typedef struct libbmp_imetadata	libbmp_imetadata;
typedef f2_status_t (*libbmp_imetadata_query_metadata)( libbmp_imetadata * thisp,
	libbmp_metadata_t id, libbmp_type_t type, void * buffer, size_t buffer_size );
struct libbmp_imetadata {
	libbmp_imetadata_query_metadata	query_metadata;
};
f2_bool_t libbmp_api libbmp_query_metadata_bool( libbmp_imetadata * thisp, libbmp_metadata_t id );

typedef struct libbmp_ometadata	libbmp_ometadata;
typedef f2_status_t (*libbmp_ometadata_process_metadata)( libbmp_ometadata * thisp,
	libbmp_metadata_t id, libbmp_type_t type, const void * data, size_t data_size );
struct libbmp_ometadata {
	libbmp_ometadata_process_metadata	process_metadata;
};

// Additional format readers/writers
// Used for lnking in foreign formats encoders/decoders for handling jpeg and png compression schemes
typedef struct libbmp_encoder	libbmp_encoder;
typedef f2_status_t (*libbmp_encoder_process)( libbmp_encoder * thisp,
	void * dst_buffer, size_t dst_buffer_size,
	const void * src_buffer, size_t src_buffer_size,
	const libbmp_image_descriptor * expecetd_format,
	size_t * bytes_written );
struct libbmp_encoder
{
	libbmp_encoder_process	process;
};

// Public interface
f2_status_t libbmp_api libbmp_detect( const void * data, size_t data_size, libbmp_image_descriptor * desc );

struct libbmp_bmp_reader_context {
	const void *			data;
	size_t					data_size;
	libbmp_pixel_format		pixel_format;		///< output pixel format (UNSUPPORTED)

	libbmp_obitmap *		bitmap;
	libbmp_opalette *		palette;
	libbmp_diag *			error_handler;
	libbmp_ometadata *		metadata;

	libbmp_encoder *		rle4_reader;
	libbmp_encoder *		rle8_reader;
	libbmp_encoder *		rle24_reader;
	libbmp_encoder *		huffman1d_reader;
	libbmp_encoder *		jpeg_reader;
	libbmp_encoder *		png_reader;
};
typedef struct libbmp_bmp_reader_context	libbmp_bmp_reader_context;
f2_status_t libbmp_api libbmp_read_bmp( const libbmp_bmp_reader_context * context );

struct libbmp_bmp_writer_context {
	const void *			image_data;
	size_t					image_data_size;
	libbmp_image_descriptor image_desc;

	f2_ostream *			stream;
	libbmp_ibitmap *		bitmap;
	libbmp_ipalette *		palette;
	libbmp_diag *			error_handler;
	libbmp_imetadata *		metadata;

	libbmp_encoder *		rle4_writer;
	libbmp_encoder *		rle8_writer;
	libbmp_encoder *		rle24_writer;
	libbmp_encoder *		huffman1d_writer;
	libbmp_encoder *		jpeg_writer;
	libbmp_encoder *		png_writer;
};
typedef struct libbmp_bmp_writer_context	libbmp_bmp_writer_context;
f2_status_t libbmp_api libbmp_write_bmp( const libbmp_bmp_writer_context * context );

typedef struct libbmp_dump_context	libbmp_dump_context;
struct libbmp_dump_context {
	const void *			data;
	size_t					data_size;

	f2_msgout *			msgout;
	libbmp_diag *			error_handler;

	#define LIBBMP_DUMP_FILE_HEADER		0x0001
	#define LIBBMP_DUMP_BITMAP_HEADER	0x0002
	#define LIBBMP_DUMP_COLOR_TABLE		0x0004
	#define LIBBMP_DUMP_IMAGE_BITS		0x0008	/// < dump image data as bytes
	#define LIBBMP_DUMP_PIXELS			0x0010	/// < dump image data as pixels
	#define LIBBMP_DUMP_GAPS			0x0020
	#define LIBBMP_DUMP_PREFER_OS2		0x0100	///< prefer OS/2 namings
	#define LIBBMP_DUMP_FORCE			0x0200	///< force dump, fail only at non-recoverable errors
	unsigned				flags;
};
f2_status_t libbmp_api libbmp_dump_bmp( const libbmp_dump_context * context );


#pragma pack(pop)

#endif // ndef __libbmp__66E23F6E86534A7D9A559060C0F93F13__
/*END OF libbmp.h*/
