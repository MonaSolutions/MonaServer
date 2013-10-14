/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License received along this program for more
	details (or else see http://www.gnu.org/licenses/).

	This file is a part of Mona.
*/

#pragma once

#if defined(STATIC) && !defined(POCO_STATIC)
	#define POCO_STATIC
#endif


#include "Poco/Foundation.h"
#include <stdio.h>


#if defined(POCO_OS_FAMILY_WINDOWS)
	#define snprintf _snprintf
	#define timegm _mkgmtime
#endif

#define HMAC_KEY_SIZE	0x20

//
// Automatically link Base library.
//
#if defined(_MSC_VER)
	#if !defined(POCO_NO_AUTOMATIC_LIBS)
		#if defined(_DEBUG)
			#pragma comment(lib, "libeay32MTd.lib")
			#pragma comment(lib, "ssleay32MTd.lib")
		#else
			#pragma comment(lib, "libeay32MT.lib")
			#pragma comment(lib, "ssleay32MT.lib")
		#endif
	#endif
#endif

// Fonctions round

#define ROUND(val) floor( val + 0.5 )

//
// Memory Leak
//

#if defined(POCO_OS_FAMILY_WINDOWS) && defined(_DEBUG)
	#include <map> // A cause d'un pb avec le nouveau new debug!
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

void DetectMemoryLeak();

namespace Mona {

#if defined(_MSC_VER)
	//
	// Windows/Visual C++
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed __int64         Int64;
	typedef unsigned __int64       UInt64;
	#if defined(_WIN64)
		#define POCO_PTR_IS_64_BIT 1
		typedef signed __int64     IntPtr;
		typedef unsigned __int64   UIntPtr;
	#else
		typedef signed long        IntPtr;
		typedef unsigned long      UIntPtr;
	#endif
	#define POCO_HAVE_INT64 1
#elif defined(__GNUC__) || defined(__clang__)
	//
	// Unix/GCC
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed long            IntPtr;
	typedef unsigned long          UIntPtr;
	#if defined(__LP64__)
		#define POCO_PTR_IS_64_BIT 1
		#define POCO_LONG_IS_64_BIT 1
		typedef signed long        Int64;
		typedef unsigned long      UInt64;
	#else
		typedef signed long long   Int64;
		typedef unsigned long long UInt64;
	#endif
	#define POCO_HAVE_INT64 1
#elif defined(__DECCXX)
	//
	// Compaq C++
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed __int64         Int64;
	typedef unsigned __int64       UInt64;
	#if defined(__VMS)
		#if defined(__32BITS)
			typedef signed long    IntPtr;
			typedef unsigned long  UIntPtr;
		#else
			typedef Int64          IntPtr;
			typedef UInt64         UIntPtr;
			#define POCO_PTR_IS_64_BIT 1
		#endif
	#else
		typedef signed long        IntPtr;
		typedef unsigned long      UIntPtr;
		#define POCO_PTR_IS_64_BIT 1
		#define POCO_LONG_IS_64_BIT 1
	#endif
	#define POCO_HAVE_INT64 1
#elif defined(__HP_aCC)
	//
	// HP Ansi C++
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed long            IntPtr;
	typedef unsigned long          UIntPtr;
	#if defined(__LP64__)
		#define POCO_PTR_IS_64_BIT 1
		#define POCO_LONG_IS_64_BIT 1
		typedef signed long        Int64;
		typedef unsigned long      UInt64;
	#else
		typedef signed long long   Int64;
		typedef unsigned long long UInt64;
	#endif
	#define POCO_HAVE_INT64 1
#elif defined(__SUNPRO_CC)
	//
	// SUN Forte C++
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed long            IntPtr;
	typedef unsigned long          UIntPtr;
	#if defined(__sparcv9)
		#define POCO_PTR_IS_64_BIT 1
		#define POCO_LONG_IS_64_BIT 1
		typedef signed long        Int64;
		typedef unsigned long      UInt64;
	#else
		typedef signed long long   Int64;
		typedef unsigned long long UInt64;
	#endif
	#define POCO_HAVE_INT64 1
#elif defined(__IBMCPP__) 
	//
	// IBM XL C++
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed long            IntPtr;
	typedef unsigned long          UIntPtr;
	#if defined(__64BIT__)
		#define POCO_PTR_IS_64_BIT 1
		#define POCO_LONG_IS_64_BIT 1
		typedef signed long        Int64;
		typedef unsigned long      UInt64;
	#else
		typedef signed long long   Int64;
		typedef unsigned long long UInt64;
	#endif
	#define POCO_HAVE_INT64 1
#elif defined(__sgi) 
	//
	// MIPSpro C++
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed long            IntPtr;
	typedef unsigned long          UIntPtr;
	#if _MIPS_SZLONG == 64
		#define POCO_PTR_IS_64_BIT 1
		#define POCO_LONG_IS_64_BIT 1
		typedef signed long        Int64;
		typedef unsigned long      UInt64;
	#else
		typedef signed long long   Int64;
		typedef unsigned long long UInt64;
	#endif
	#define POCO_HAVE_INT64 1
#elif defined(_DIAB_TOOL)
	typedef signed char        Int8;
	typedef unsigned char      UInt8;
	typedef signed short       Int16;
	typedef unsigned short     UInt16;
	typedef signed int         Int32;
	typedef unsigned int       UInt32;
	typedef signed long        IntPtr;
	typedef unsigned long      UIntPtr;
	typedef signed long long   Int64;
	typedef unsigned long long UInt64;
	#define POCO_HAVE_INT64 1
#endif


} // namespace Mona
