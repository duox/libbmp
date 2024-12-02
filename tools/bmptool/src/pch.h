/*pch.h*/

#ifdef _MSC_VER
# pragma warning(disable:4201)	//: nonstandard extension used : nameless struct/union
# pragma warning(disable:4702)	//: unreachable code
# pragma warning(disable:4127)	//: conditional expression is constant
#endif // def _MSC_VER


#ifdef _WIN32
# define STRICT
# include <windows.h>
# include <shlwapi.h>
#endif // def _WIN32


#include <io.h>
#include <malloc.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _MSC_VER
# include <crtdbg.h>
#endif

#ifndef DEV_PCH
# include <libbmp.h>
#endif // ndef _DEV_PCH

/*END OF pch.h*/
