/*bmpdefs.h*/

/*
 *	We are trying to maintain compatibility in definitions of Windows structures, so define Windows native types
 *	in case they are not yet defined.
 */
#ifndef _WINDOWS_
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned short	USHORT;
typedef unsigned long	DWORD;
typedef unsigned long	ULONG;
typedef signed char		SBYTE;
typedef signed short	SHORT;
typedef signed long		LONG;

#pragma pack(push, 1)
struct RGBTRIPLE {
	BYTE	rgbtBlue;
	BYTE	rgbtGreen;
	BYTE	rgbtRed;
};
#pragma pack(pop)
f2_static_assert(sizeof(struct RGBTRIPLE) == 0x03);
typedef struct RGBTRIPLE	RGBTRIPLE;

#pragma pack(push, 1)
struct RGBQUAD {
	BYTE	rgbBlue;		/* Blue component */
	BYTE	rgbGreen;		/* Green component */
	BYTE	rgbRed;			/* Red component */
	BYTE	rgbReserved;	/* Padding (always 0) */
};
#pragma pack(pop)
f2_static_assert(sizeof(struct RGBQUAD) == 0x04);
typedef struct RGBQUAD	RGBQUAD;
#endif /* ndef _WINDOWS_ */

// Bitmap file types
#define LIBBMP_FILETYPE_BITMAP				'MB'	///< 'BM': Windows / OS/2 bitmap
#if !LIBBMP_CFG_NO_OS2_SUPPORT
# define LIBBMP_FILETYPE_BITMAP_ARRAY		'AB'	///< 'BA': OS/2 bitmap array (UNSUPPORTED)
# define LIBBMP_FILETYPE_COLOR_ICON			'IC'	///< 'IC': OS/2 color icon (UNSUPPORTED)
# define LIBBMP_FILETYPE_COLOR_POINTER		'PC'	///< 'CP': OS/2 color pointer (UNSUPPORTED)
# define LIBBMP_FILETYPE_COLOR_STRUCT_ICON	'CI'	///< 'IC': OS/2 struct icon (UNSUPPORTED)
# define LIBBMP_FILETYPE_POINTER			'TP'	///< 'PT': OS/2 pointer (UNSUPPORTED)
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Legacy (old) formats

#if !LIBBMP_CFG_NO_LEGACY_SUPPORT

/* DDB file header (Windows MetaFile Bitmap16), often called BITMAP16 */
#pragma pack(push, 1)
struct tagDDBFILEHEADER			/* sizeof == 0x0A */
{
	WORD	bmType;				/* = 0 */
	WORD	Width;
	WORD	Height;
	WORD	Pitch;
	BYTE	Planes;				/* must be 1 to keep compatibility to WMF */
	BYTE	bmBitCount;
	DWORD	bmBits;
	/* bitmap bits are here (thus no palette information) */
};
#pragma pack(pop)
f2_static_assert(sizeof(struct tagDDBFILEHEADER) == 0x10);
typedef struct tagDDBFILEHEADER		DDBFILEHEADER;

/* Old bitmap header (Windows prior 3.0) */
#pragma pack(push, 1)
struct tagBITMAPOLDHEADER		/* sizeof == 0x10 */
{
	BYTE	Version;			/* = 2 (RT_BITMAP) */
	BYTE	Flags;				/* = 0x80 for discardable bitmaps */
	WORD	Reserved0;			/* = 0 */
	// DDBFILEHEADER starts here
	WORD	Width;
	WORD	Height;
	WORD	Pitch;				/* row size in bytes, must always be even */
	BYTE	Planes;
	BYTE	BitCount;
	// DDBFILEHEADER ends here
	WORD	Reserved1;			/* = 0 */
	WORD	Reserved2;			/* = 0 */
	/* bitmap bits are here (thus no palette information) */
};
#pragma pack(pop)
f2_static_assert(sizeof(struct tagBITMAPOLDHEADER) == 0x10);
typedef struct tagBITMAPOLDHEADER	 BITMAPOLDHEADER;
typedef struct tagBITMAPOLDHEADER *	PBITMAPOLDHEADER;

/* Bitmap core header (compatible with OS/2 1.x) */
#ifndef _WINDOWS_
#pragma pack(push, 1)
struct tagBITMAPCOREHEADER		/* sizeof == 0xC */
{
	DWORD	bcSize;
	WORD	bcWidth;
	WORD	bcHeight;
	WORD	bcPlanes;
	WORD	bcBitCount;
	//if( bcBitCount <= 8 )	RGBTRIPLE	rgbPalette[1 << bcBitCount];
};
#pragma pack(pop)
f2_static_assert(sizeof(struct tagBITMAPCOREHEADER) == 0x0C);
typedef struct tagBITMAPCOREHEADER		 BITMAPCOREHEADER;
typedef struct tagBITMAPCOREHEADER *	PBITMAPCOREHEADER;
#endif /* ndef _WINDOWS_ */

#endif // !LIBBMP_CFG_NO_LEGACY_SUPPORT

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Windows common formats

/*
 *	Color space definitions.
 */
#if !defined _WINDOWS_ || WINVER >= 0x0400
typedef long					FXPT2DOT30;
typedef struct tagCIEXYZ		CIEXYZ;
typedef struct tagICEXYZTRIPLE	CIEXYZTRIPLE;
struct tagCIEXYZ
{
	FXPT2DOT30	ciexyzX;
	FXPT2DOT30	ciexyzY;
	FXPT2DOT30	ciexyzZ;
};
struct tagICEXYZTRIPLE
{
	CIEXYZ	ciexyzRed;
	CIEXYZ	ciexyzGreen;
	CIEXYZ	ciexyzBlue;
};

// Values for bV5CSType
#ifndef PROFILE_LINKED
# define LCS_CALIBRATED_RGB			0x00000000L
# define LCS_sRGB					0x73524742	// 'sRGB'
# define LCS_WINDOWS_COLOR_SPACE	0x57696E20	// 'Win ' Windows default color space
# define PROFILE_LINKED				0x4C494E4B	// 'LINK'
# define PROFILE_EMBEDDED			0x4D424544	// 'MBED'
#endif /* ndef PROFILE_LINKED */

#ifndef LCS_GM_BUSINESS
# define LCS_GM_BUSINESS			0x00000001L
# define LCS_GM_GRAPHICS			0x00000002L
# define LCS_GM_IMAGES				0x00000004L
# define LCS_GM_ABS_COLORIMETRIC	0x00000008L
#endif /* ndef LCS_GM_BUSINESS */

#endif /* !defined _WINDOWS_ || WINVER >= 0x0400 */

/*
 *	Windows bitmap compression types.
 */
#ifndef BI_RGB
# define BI_RGB				0L
#endif /* ndef BI_RGB */
#ifndef BI_RLE8
# define BI_RLE8			1L
#endif /* ndef BI_RLE8 */
#ifndef BI_RLE4
# define BI_RLE4			2L
#endif /* ndef BI_RLE4 */
#ifndef BI_BITFIELDS
# define BI_BITFIELDS		3L
#endif /* ndef BI_BITFIELDS */
#ifndef BI_JPEG
# define BI_JPEG			4L
#endif /* ndef BI_JPEG */
#ifndef BI_PNG
# define BI_PNG				5L
#endif /* ndef BI_PNG */
#ifndef BI_ALPHABITFIELDS
# define BI_ALPHABITFIELDS	6L
#endif /* ndef BI_ALPHABITFIELDS */
#define BI_CMYK				11L		// TODO: uncompressed, CMYK data
#define BI_CMYKRLE8			12L		// TODO: RLE8 compressed, CMYK data
#define BI_CMYKRLE4			13L		// TODO: RLE4 compressed, CMYK data

/* Bitmap file header */
#pragma pack(push, 1)
struct tagBITMAPFILEHEADER
{
	WORD	bfType;
	DWORD	bfSize;				///< size of the image file, in bytes
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
};
#pragma pack(pop)
f2_static_assert(sizeof(struct tagBITMAPFILEHEADER) == 0x0E);
typedef struct tagBITMAPFILEHEADER		 BITMAPFILEHEADER;
typedef struct tagBITMAPFILEHEADER *	PBITMAPFILEHEADER;

/* Bitmap information header (initial version) */
#ifndef _WINDOWS_
#pragma pack(push, 1)
struct tagBITMAPINFOHEADER		/* sizeof == 0x28 */
{
	DWORD	biSize;
	LONG	biWidth;
	LONG	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
	DWORD	biCompression;
	DWORD	biSizeImage;
	DWORD	biXPelsPerMeter;
	DWORD	biYPelsPerMeter;
	DWORD	biClrUsed;
	DWORD	biClrImportant;
};
#pragma pack(pop)
f2_static_assert(sizeof(struct tagBITMAPINFOHEADER) == 0x28);
typedef struct tagBITMAPINFOHEADER		 BITMAPINFOHEADER;
typedef struct tagBITMAPINFOHEADER *	PBITMAPINFOHEADER;
#endif /* ndef _WINDOWS_ */

/* Bitmap information header v.2 */
/* (undocumented, so we don't use #ifdef/#endif pair since header is not defined by Windows headers) */
#pragma pack(push, 1)
struct tagBITMAPV2INFOHEADER	/* sizeof == 0x34 */
{
	DWORD	biV2Size;
	LONG	biV2Width;
	LONG	biV2Height;
	WORD	biV2Planes;
	WORD	biV2BitCount;
	DWORD	biV2Compression;
	DWORD	biV2SizeImage;
	DWORD	biV2XPelsPerMeter;
	DWORD	biV2YPelsPerMeter;
	DWORD	biV2ClrUsed;
	DWORD	biV2ClrImportant;
	/* new for V2 */
	DWORD	biV2RedMask;
	DWORD	biV2GreenMask;
	DWORD	biV2BlueMask;
};
#pragma pack(pop)
f2_static_assert(sizeof(struct tagBITMAPV2INFOHEADER) == 0x34);
typedef struct tagBITMAPV2INFOHEADER	 BITMAPV2INFOHEADER;
typedef struct tagBITMAPV2INFOHEADER *	PBITMAPV2INFOHEADER;

/* Bitmap information header v.3 */
/* (undocumented, so we don't use #ifdef/#endif pair since header is not defined by Windows headers) */
#pragma pack(push, 1)
struct tagBITMAPV3INFOHEADER	/* sizeof == 0x38 */
{
	DWORD	biV3Size;
	LONG	biV3Width;
	LONG	biV3Height;
	WORD	biV3Planes;
	WORD	biV3BitCount;
	DWORD	biV3Compression;
	DWORD	biV3SizeImage;
	DWORD	biV3XPelsPerMeter;
	DWORD	biV3YPelsPerMeter;
	DWORD	biV3ClrUsed;
	DWORD	biV3ClrImportant;
	/* new for V2 */
	DWORD	biV3RedMask;
	DWORD	biV3GreenMask;
	DWORD	biV3BlueMask;
	/* new for V3 */
	DWORD	biV3AlphaMask;
};
#pragma pack(pop)
f2_static_assert(sizeof(struct tagBITMAPV3INFOHEADER) == 0x38);
typedef struct tagBITMAPV3INFOHEADER	 BITMAPV3INFOHEADER;
typedef struct tagBITMAPV3INFOHEADER *	PBITMAPV3INFOHEADER;

/* Bitmap information header v.4 (introduced first in Windows NT 4) */
#if !defined _WINDOWS_ || WINVER < 0x0400
#pragma pack(push, 1)
struct tagBITMAPV4INFOHEADER	/* sizeof == 0x6C */
{
	DWORD	biV4Size;
	LONG	biV4Width;
	LONG	biV4Height;
	WORD	biV4Planes;
	WORD	biV4BitCount;
	DWORD	biV4Compression;
	DWORD	biV4SizeImage;
	DWORD	biV4XPelsPerMeter;
	DWORD	biV4YPelsPerMeter;
	DWORD	biV4ClrUsed;
	DWORD	biV4ClrImportant;
	/* new for V2 */
	DWORD	biV4RedMask;
	DWORD	biV4GreenMask;
	DWORD	biV4BlueMask;
	/* new for V3 */
	DWORD	biV4AlphaMask;
	/* new for V4 */
	DWORD	biV4CSType;
	CIEXYZTRIPLE biV4Endpoints;
	DWORD	biV4GammaRed;
	DWORD	biV4GammaGreen;
	DWORD	biV4GammaBlue;
};
#pragma pack(pop)
f2_static_assert(sizeof(struct tagBITMAPV4INFOHEADER) == 0x6C);
typedef struct tagBITMAPV4INFOHEADER	 BITMAPV4INFOHEADER;
typedef struct tagBITMAPV4INFOHEADER *	PBITMAPV4INFOHEADER;
#endif /* !defined _WINDOWS_ || WINVER < 0x0400 */

/* Bitmap information header v.5 (introduced first in Windows 2000) */
#if !defined _WINDOWS_ || WINVER < 0x0500
#pragma pack(push, 1)
struct tagBITMAPV5INFOHEADER	/* sizeof == 0x7C */
{
	DWORD	biV5Size;
	LONG	biV5Width;
	LONG	biV5Height;
	WORD	biV5Planes;
	WORD	biV5BitCount;
	DWORD	biV5Compression;
	DWORD	biV5SizeImage;
	DWORD	biV5XPelsPerMeter;
	DWORD	biV5YPelsPerMeter;
	DWORD	biV5ClrUsed;
	DWORD	biV5ClrImportant;
	/* new for V2 */
	DWORD	biV5RedMask;
	DWORD	biV5GreenMask;
	DWORD	biV5BlueMask;
	/* new for V3 */
	DWORD	biV5AlphaMask;
	/* new for V4 */
	DWORD	biV5CSType;
	CIEXYZTRIPLE biV5Endpoints;
	DWORD	biV5GammaRed;
	DWORD	biV5GammaGreen;
	DWORD	biV5GammaBlue;
	/* new for V5 */
	DWORD	bV5Intent;
	DWORD	bV5ProfileData;
	DWORD	bV5ProfileSize;
	DWORD	bV5Reserved;
};
#pragma pack(pop)
f2_static_assert(sizeof(struct tagBITMAPV5INFOHEADER) == 0x7C);
typedef struct tagBITMAPV5INFOHEADER	 BITMAPV5INFOHEADER;
typedef struct tagBITMAPV5INFOHEADER *	PBITMAPV5INFOHEADER;

#endif /* !defined _WINDOWS_ || WINVER < 0x0400 */

/****************************************************************************************************************************************************/
/* OS/2 bitmap definitions */

#if !LIBBMP_CFG_NO_OS2_SUPPORT

#pragma pack(push, 1)
typedef struct _RGB
{
	BYTE	bBlue;		/* Blue component of the color definition. */
	BYTE	bGreen;		/* Green component of the color definition. */
	BYTE	bRed;		/* Red component of the color definition. */
} RGB, *PRGB;
f2_static_assert(sizeof(struct _RGB) == 0x03);
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _RGB2 {
  BYTE     bBlue;      /*  Blue component of the color definition. */
  BYTE     bGreen;     /*  Green component of the color definition. */
  BYTE     bRed;       /*  Red component of the color definition. */
  BYTE     fcOptions;  /*  Entry options. */
} RGB2, * PRGB2;
f2_static_assert(sizeof(struct _RGB2) == 0x04);
#pragma pack(pop)

#define PC_RESERVED		0x0001	// The color entry is reserved for animating color with the palette manager.
#define PC_EXPLICIT		0x0002	// The low-order word of the color table entry designates a physical palette slot.
								// This allows an application to show the actual contents of the device palette as
								// realized for other logical palettes. This does not prevent the color
								// in the slot from being changed for any reason.


/* OS/2 bitmap file header (compatible with Windows BITMAPFILEHEADER, but uses different namings) */
#pragma pack(push, 1)
struct tagOS2BMPFILEHEADER
{
	WORD	FileType;		/* File type identifier */
	DWORD	FileSize;		/* Size of the file in bytes */
	WORD	XHotSpot;		/* X coordinate of hotspot */
	WORD	YHotSpot;		/* Y coordinate of hotspot */
	DWORD	BitmapOffset;	/* Starting position of image data in bytes */
};
#pragma pack(pop)
typedef struct tagOS2BMPFILEHEADER		 OS2BMPFILEHEADER;
typedef struct tagOS2BMPFILEHEADER *	POS2BMPFILEHEADER;

#define OS2BITMAPTYPE_BITMAP_ARRAY		0x4142	// 'BA'
#define OS2BITMAPTYPE_BITMAP			0x4D42	// 'BM'
#define OS2BITMAPTYPE_COLOR_ICON		0x4943	// `CI'
#define OS2BITMAPTYPE_COLOR_POINTER		0x5043	// `CP'
#define OS2BITMAPTYPE_ICON				0x4349	// `IC'
#define OS2BITMAPTYPE_POINTER			0x5450	// `PT'

/* OS/2 ancient format */
#pragma pack(push, 1)
struct tagOS2BMPFILEOLDHEADER
{
	WORD	FileType;		/* File type identifier */
	DWORD	HeaderSize;		/* Header size (instead of file size) */
	WORD	XHotSpot;		/* X coordinate of hotspot */
	WORD	YHotSpot;		/* Y coordinate of hotspot */
	DWORD	BitmapOffset;	/* Starting position of image data in bytes */
};
#pragma pack(pop)
typedef struct tagOS2BMPFILEOLDHEADER	 OS2BMPFILEOLDHEADER;
typedef struct tagOS2BMPFILEOLDHEADER *	POS2BMPFILEOLDHEADER;

/* OS/2 1.x bitmap header (compatible with Windows BITMAPCOREHEADER, but uses different namings) */
#pragma pack(push, 1)
struct tagOS21XBITMAPHEADER
{
	DWORD	Size;			/* Size of this header in bytes */
	WORD	Width;			/* Image width in pixels */
	WORD	Height;			/* Image height in pixels */
	WORD	NumPlanes;		/* Number of color planes */
	WORD	BitsPerPixel;	/* Number of bits per pixel */
};
#pragma pack(pop)
typedef struct tagOS21XBITMAPHEADER		 OS21XBITMAPHEADER;
typedef struct tagOS21XBITMAPHEADER *	POS21XBITMAPHEADER;

/* OS/2 2.x bitmap header */
#pragma pack(push, 1)
typedef struct _BITMAPINFOHEADER2
{
	ULONG	cbFix;				/* Size of this structure in bytes */
	ULONG	cx;					/* Bitmap width in pixels */
	ULONG	cy;					/* Bitmap height in pixel */
	USHORT	cPlanes;			/* Number of bit planes (color depth) */
	USHORT	cBitCount;			/* Number of bits per pixel per plane */
	/* new for OS/2 2.x */
	ULONG	ulCompression;		/* Bitmap compression scheme */
	ULONG	cbImage;			/* Size of bitmap data in bytes */
	ULONG	cxResolution;		/* X resolution of display device */
	ULONG	cyResolution;		/* Y resolution of display device */
	ULONG	cclrUsed;			/* Number of color table indices used */
	ULONG	cclrImportant;		/* Number of important color indices */
	/* end of BITMAPINFOHEADER */
	USHORT	usUnits;			/* Type of units used to measure resolution */
	USHORT	usReserved;			/* Pad structure to 4-byte boundary */
	USHORT	usRecording;		/* Recording algorithm */
	USHORT	usRendering;		/* Halftoning algorithm used */
	ULONG	cSize1;				/* Reserved for halftoning algorithm use */
	ULONG	cSize2;				/* Reserved for halftoning algorithm use */
	ULONG	ulColorEncoding;	/* Color model used in bitmap */
	ULONG	ulIdentifier;		/* Reserved for application use */
} BITMAPINFOHEADER2, * PBITMAPINFOHEADER2;
#pragma pack(pop)

typedef BITMAPINFOHEADER2	OS22XBITMAPHEADER;

/*
 *	OS/2 bitmap compression types.
 */
#define BCA_UNCOMP          BI_RGB
#define BCA_RLE4			BI_RLE4
#define BCA_RLE8			BI_RLE8
#define BCA_HUFFMAN1D       3L
#define BCA_RLE24			4L

#define BRU_METRIC		0	// default value for the Units field

#endif // !LIBBMP_CFG_NO_OS2_SUPPORT

/****************************************************************************************************************************************************/
/* Common definitions */

union tagCOMMONBITMAPHEADER
{
#if !LIBBMP_CFG_NO_LEGACY_SUPPORT
	BITMAPOLDHEADER			BitmapOldHeader;

	BITMAPCOREHEADER		BitmapCoreHeader;
#endif // !LIBBMP_CFG_NO_LEGACY_SUPPORT

	BITMAPINFOHEADER		WinBitmapHeader;
	BITMAPV2INFOHEADER		WinBitmapV2Header;
	BITMAPV3INFOHEADER		WinBitmapV3Header;
	BITMAPV4INFOHEADER		WinBitmapV4Header;
	BITMAPV5INFOHEADER		WinBitmapV5Header;

#if !LIBBMP_CFG_NO_OS2_SUPPORT
	OS21XBITMAPHEADER		Os21BitmapHeader;
	OS22XBITMAPHEADER		Os22BitmapHeader;
#endif // !LIBBMP_CFG_NO_OS2_SUPPORT
};
typedef union tagCOMMONBITMAPHEADER		 COMMONBITMAPHEADER;
typedef union tagCOMMONBITMAPHEADER *	PCOMMONBITMAPHEADER;

/*END OF bmpdefs.h*/
