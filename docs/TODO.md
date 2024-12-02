TODO list
==============================================

+ attach metadata interfaces to the codec
- try to support extra functionality marked UNSUPPORTED
- support CMYK source (note that color profile information is needed to convert from CMYK to RGB so transfer data as is)
- consider using output streams instead of output buffers
- generate test data, including fuzzied

ARRAY SUPPORT
- ICO/CUR support (they share similar format)
- OS/2 bitmap arrays:
	BITMAPARRAYFILEHEADER2 (for bitmap #1)
		BITMAPFILEHEADER2
			BITMAPINFOHEADER2
		Color table
	BITMAPARRAYFILEHEADER2 (for bitmap #2)
		BITMAPFILEHEADER2
			BITMAPINFOHEADER2
		Color table
		Pel Data (for bitmap #1)
		Pel Data (for bitmap #2)


* Masks for 16 bit (5-5-5): ALPHA = 0x00000000, RED = 0x00007C00, GREEN = 0x000003E0, BLUE = 0x0000001F
* Masks for 16 bit (5-6-5): ALPHA = 0x00000000, RED = 0x0000F800, GREEN = 0x000007E0, BLUE = 0x0000001F
* Masks for 24 bit: ALPHA = 0x00000000, RED = 0x00FF0000, GREEN = 0x0000FF00, BLUE = 0x000000FF
* Masks for 32 bit: ALPHA = 0xFF000000, RED = 0x00FF0000, GREEN = 0x0000FF00, BLUE = 0x000000FF

