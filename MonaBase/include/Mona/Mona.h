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

#include <stdio.h>
#include <string>

/////  Usefull macros and patchs   //////

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define HMAC_KEY_SIZE	0x20

#if defined(_WIN32)
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#define sprintf sprintf_s
#define timegm _mkgmtime
#define GMTIME(VALUE,RESULT) gmtime_s(&RESULT,&VALUE);
#define LOCALTIME(VALUE,RESULT) localtime_s(&RESULT,&VALUE);
#define STRERROR(CODE,ERROR) {ERROR.resize(100);strerror_s(&ERROR[0],(std::size_t)100,CODE);}
#elif
#define STRERROR(CODE,ERROR) {error.resize(100);strerror_r(CODE,&ERROR[0],100);}
#define GMTIME(VALUE,RESULT) gmtime_r(&VALUE,&RESULT)
#define LOCALTIME(VALUE,RESULT) localtime_r(&VALUE,&RESULT)
#endif

///// Disable some annoying warnings /////
#if defined(_MSC_VER)
#pragma warning(disable:4018) // signed/unsigned comparison
#endif

//
// Automatically link Base library.
//
#if defined(_MSC_VER)
	#if defined(_DEBUG)
		#pragma comment(lib, "libeay32MTd.lib")
		#pragma comment(lib, "ssleay32MTd.lib")
	#else
		#pragma comment(lib, "libeay32MT.lib")
		#pragma comment(lib, "ssleay32MT.lib")
	#endif
#endif

//
// Memory Leak
//

#if defined(_WIN32) && defined(_DEBUG)
	#include <map> // A cause d'un pb avec le nouveau new debug! TODO enlever?
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif



//
// Platform Identification
//
#define _OS_FREE_BSD      0x0001
#define _OS_AIX           0x0002
#define _OS_TRU64         0x0003
#define _OS_LINUX         0x0004
#define _OS_MAC_OS_X      0x0005
#define _OS_NET_BSD       0x0006
#define _OS_OPEN_BSD      0x0007
#define _OS_IRIX          0x0008
#define _OS_SOLARIS       0x0009
#define _OS_QNX           0x000a
#define _OS_CYGWIN        0x000b
#define _OS_UNKNOWN_UNIX  0x00ff
#define _OS_WINDOWS_NT    0x1001
#define _OS_WINDOWS_CE    0x1011


#if defined(__FreeBSD__)
#define _OS_UNIX 1
#define _OS_BSD 1
#define _OS _OS_FREE_BSD
#elif defined(_AIX) || defined(__TOS_AIX__)
#define _OS_UNIX 1
#define _OS _OS_AIX
#elif defined(__digital__) || defined(__osf__)
#define _OS_UNIX 1
#define _OS _OS_TRU64
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__TOS_LINUX__)
#define _OS_UNIX 1
#define _OS _OS_LINUX
#elif defined(__APPLE__) || defined(__TOS_MACOS__)
#define _OS_UNIX 1
#define _OS_BSD 1
#define _OS _OS_MAC_OS_X
#elif defined(__NetBSD__)
#define _OS_UNIX 1
#define _OS_BSD 1
#define _OS _OS_NET_BSD
#elif defined(__OpenBSD__)
#define _OS_UNIX 1
#define _OS_BSD 1
#define _OS _OS_OPEN_BSD
#elif defined(sgi) || defined(__sgi)
#define _OS_UNIX 1
#define _OS _OS_IRIX
#elif defined(sun) || defined(__sun)
#define _OS_UNIX 1
#define _OS _OS_SOLARIS
#elif defined(__QNX__)
#define _OS_UNIX 1
#define _OS _OS_QNX
#elif defined(unix) || defined(__unix) || defined(__unix__)
#define _OS_UNIX 1
#define _OS _OS_UNKNOWN_UNIX
#elif defined(_WIN32_WCE)
#define _OS_WINDOWS 1
#define _OS _OS_WINDOWS_CE
#elif defined(_WIN32) || defined(_WIN64)
#define _OS_WINDOWS 1
#define _OS _OS_WINDOWS_NT
#elif defined(__CYGWIN__)
#define _OS_UNIX 1
#define _OS _OS_CYGWIN
#endif


#if !defined(_OS)
#error "Unknown Platform."
#endif


//
// Hardware Architecture and Byte Order
//

#define _ARCH_IA32    0x01
#define _ARCH_IA64    0x02
#define _ARCH_MIPS    0x03
#define _ARCH_PPC     0x04
#define _ARCH_POWER   0x05
#define _ARCH_SPARC   0x06
#define _ARCH_AMD64   0x07
#define _ARCH_ARM     0x08
#define _ARCH_SH      0x09
#define _ARCH_NIOS2   0x0a


#if defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define _ARCH _ARCH_IA32
#define _ARCH_LITTLE_ENDIAN 1
#elif defined(_IA64) || defined(__IA64__) || defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
#define _ARCH _ARCH_IA64
#define _ARCH_LITTLE_ENDIAN 1
#elif defined(__x86_64__) || defined(_M_X64)
#define _ARCH _ARCH_AMD64
#define _ARCH_LITTLE_ENDIAN 1
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__) || defined(_M_MRX000)
#define _ARCH _ARCH_MIPS
#define _ARCH_BIG_ENDIAN 1
#elif defined(__hppa) || defined(__hppa__)
#define _ARCH _ARCH_HPPA
#define _ARCH_BIG_ENDIAN 1
#elif defined(__PPC) || defined(__POWERPC__) || defined(__powerpc) || defined(__PPC__) || defined(__powerpc__) || defined(__ppc__) || defined(__ppc) || defined(_ARCH_PPC) || defined(_M_PPC)
#define _ARCH _ARCH_PPC
#define _ARCH_BIG_ENDIAN 1
#elif defined(_POWER) || defined(_ARCH_PWR) || defined(_ARCH_PWR2) || defined(_ARCH_PWR3) || defined(_ARCH_PWR4) || defined(__THW_RS6000)
#define _ARCH _ARCH_POWER
#define _ARCH_BIG_ENDIAN 1
#elif defined(__sparc__) || defined(__sparc) || defined(sparc)
#define _ARCH _ARCH_SPARC
#define _ARCH_BIG_ENDIAN 1
#elif defined(__arm__) || defined(__arm) || defined(ARM) || defined(_ARM_) || defined(__ARM__) || defined(_M_ARM)
#define _ARCH _ARCH_ARM
#if defined(__ARMEB__)
#define _ARCH_BIG_ENDIAN 1
#else
#define _ARCH_LITTLE_ENDIAN 1
#endif
#elif defined(__sh__) || defined(__sh) || defined(SHx) || defined(_SHX_)
#define _ARCH _ARCH_SH
#if defined(__LITTLE_ENDIAN__) || (_OS == _OS_WINDOWS_CE)
#define _ARCH_LITTLE_ENDIAN 1
#else
#define _ARCH_BIG_ENDIAN 1
#endif
#elif defined (nios2) || defined(__nios2) || defined(__nios2__)
#define _ARCH _ARCH_NIOS2
#if defined(__nios2_little_endian) || defined(nios2_little_endian) || defined(__nios2_little_endian__)
#define _ARCH_LITTLE_ENDIAN 1
#else
#define _ARCH_BIG_ENDIAN 1
#endif

#endif


#if !defined(_ARCH)
#error "Unknown Hardware Architecture."
#endif



namespace Mona {

void DetectMemoryLeak();


///// TYPES /////

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
#if defined(__LP64__)
	typedef signed long        Int64;
	typedef unsigned long      UInt64;
#else
	typedef signed long long   Int64;
	typedef unsigned long long UInt64;
#endif

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
#if defined(__64BIT__)
	typedef signed long        Int64;
	typedef unsigned long      UInt64;
#else
	typedef signed long long   Int64;
	typedef unsigned long long UInt64;
#endif
#endif



//////  No copy, no move, objet nullable  //////


class Static {
	Static(const Static& other) = delete;
	Static& operator=(const Static& other) = delete;
	Static(Static&& other) = delete;
	Static& operator=(Static&& other) = delete;
};
 
class Object {
	Object(const Object& other) = delete;
	Object& operator=(const Object& other) = delete;
	Object(Object&& other) = delete;
	Object operator=(Object&& other) = delete;
public:
	Object() = default;
	virtual ~Object() = default;
};

class ObjectNullable : virtual Object {
	bool _isNull;
public:
	ObjectNullable(bool isNull = false) : _isNull(isNull) {}

	operator bool() const { return !_isNull; }
};



} // namespace Mona
