# BMP image format library

#### Status
Almost complete, however compression codecs in libmicra are still being implemented (e.g. RLE4 double pixel runs encoding).
Additionally, Huffman1D compression (employed by OS/2 bitmap format) is relying on libhuffman which is still under development.

Testing is performed only during development process on probably copyrighted images, no complete test suite is implemented yet.

#### Supported extensions:
BMP BGA DDB DIB RLE RL4 RL8 VGA

#### Supported formats and modifications:
- DDB (Windows MetaFile Bitmap16), often called BITMAP16 (legacy);
- DDB BMP (Windows prior 3.0) (legacy);
- OS/2 1.x (bitmap core header aka BITMAPCOREHEADER) (legacy);
- Windows bitmap v.1-5;
- OS/2 bitmap file header (compatible with Windows BITMAPFILEHEADER, but uses different namings);
- OS/2 1.x bitmap header (compatible with Windows BITMAPCOREHEADER, but uses different namings);
- OS/2 ancient format (aka OS2BMPFILEOLDHEADER);
- OS/2 2.x bitmap;

#### Recognized compressions:
- uncompressed 1, 2, 4, 8, 15/16, 24, 32 bit pre pixel;
- uncompressed bitfields and alpha bitfields;
- RLE4;
- RLE8;
- RLE24;
- Huffman1D;
- JPEG;
- PNG.

#### Compression default handlers:
- RLE4/RLE8/RLE24: libmicra;
- Huffman: libhuffman;
- PNG: mnglib through libmicra;
- JPEG: jpeglib through libmicra;

Bitmap metric dimensions, color space information, gamma and other megtadata are stored and accessible via dedicated interface.

#### Wishlist:
- array bitmaps (OS/2 bitmap arrays, Windows ICOs/CURs);
- GUI tool;
- cooperation with image libraries such as Freeimage or DevIL.

