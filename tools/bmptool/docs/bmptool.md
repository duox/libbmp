BMP tool utility
================================

bmptool -?
bmptool command [{switches}] {file}

COMMANDS:
c	convert		: convert file(s)
d	dump		: dump file(s)

SWITHES:

		[GENERAL SWITCHES]
-?					: display command line help
-h	--help			: display extended help information
-q	--quiet			: quiet mode (no messages)
-v	--verbose		: verbose mode (more messages)
	--ver			: display version information

		[DUMP SWITCHES]
-b	--bitmap		: * dump bitmap header
-f	--file			: * dump file header
-g	--gaps			: dump contents of file gaps
-i	--image			: dump image bits.
-m	--meta			: dump all meta-data
-p	--palette		: dump palette contents

		[CONVERSION SWITCHES]
-c	--compression	: specify compression
	-c:compression-id
-f	--format		: specify file format
	-f:format-id
-m	--mask			: specify color mask
	-m:component-id:#
-n					: no file header (DIB file)
-p	--planes		: specify number of planes
	-p:#

compression-id := none|rle|huffman|jpeg|rle
format-id := bmpcoreheader|bih|bih2|bih3|bih4|bih5|os21|os22
component-id ::= R|G|B|A|X
