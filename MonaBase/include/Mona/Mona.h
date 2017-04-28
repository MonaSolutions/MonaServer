/*
Copyright 2014 Mona
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

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
#include <cstdlib>
#include <cstdint>
#include <string>
#include <cstring>
#include <complex>

// disable the "throw noexception" warning because Mona has its own exception and can use everywhere std throw on FATAL problem (unexpected behavior)
#pragma warning(disable: 4297)

/////  Usefull macros and patchs   //////

#define BIN		(Mona::UInt8*)
#define STR		(char*)

// BEWARE, be sure that DATA[SIZE] exists! (if DATA must be a allocation of >= (SIZE+1))
#define SCOPED_STRINGIFY(DATA,SIZE,INNER_CODE)	{ char& __end((char&)(DATA)[SIZE]); char __prev(__end); __end = 0; INNER_CODE; __end = __prev; }

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#define MAP_FIND_OR_EMPLACE(MAP,IT,KEY,...) auto IT = MAP.lower_bound(KEY); if (IT == MAP.end() || IT->first != KEY) {IT = MAP.emplace_hint(IT,std::piecewise_construct,std::forward_as_tuple(KEY),std::forward_as_tuple(__VA_ARGS__));}

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
#define _ARCH_POWERPC 0x04
#define _ARCH_POWER   0x05
#define _ARCH_SPARC   0x06
#define _ARCH_AMD64   0x07
#define _ARCH_ARM     0x08
#define _ARCH_SH      0x09
#define _ARCH_NIOS2   0x0a
#define _ARCH_ARM64   0x0b
#define _ARCH_AARCH64 0x0c


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
#define _ARCH _ARCH_POWERPC
#define _ARCH_BIG_ENDIAN 1
#elif defined(_POWER) || defined(_ARCH_PWR) || defined(_ARCH_PWR2) || defined(_ARCH_PWR3) || defined(_ARCH_PWR4) || defined(__THW_RS6000)
#define _ARCH _ARCH_POWER
#define _ARCH_BIG_ENDIAN 1
#elif defined(__sparc__) || defined(__sparc) || defined(sparc)
#define _ARCH _ARCH_SPARC
#define _ARCH_BIG_ENDIAN 1
#elif defined(__arm__) || defined(__arm) || defined(ARM) || defined(_ARM_) || defined(__ARM__) || defined(_M_ARM) || defined(__ANDROID__)
#define _ARCH _ARCH_ARM
#if defined(__ARMEB__)
	#define _ARCH_BIG_ENDIAN 1
#else
	#define _ARCH_LITTLE_ENDIAN 1
#endif
#elif defined(__arm64__) || defined(__arm64)
#define _ARCH _ARCH_ARM64
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
#elif defined(__AARCH64EL__)
	#define _ARCH _ARCH_AARCH64
	#define _ARCH_LITTLE_ENDIAN 1
#elif defined(__AARCH64EB__)
	#define _ARCH _ARCH_AARCH64
	#define _ARCH_BIG_ENDIAN 1
#endif


#if !defined(_ARCH)
#error "Unknown Hardware Architecture."
#endif

#if defined(_WIN32)
#define THREAD_ID uint32_t
#define NOMINMAX
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#define sprintf sprintf_s
#define snprintf sprintf_s
#define PATH_MAX MAX_PATH
#elif _OS_BSD
#define THREAD_ID	pthread_t
#else
#define THREAD_ID	pid_t
#endif

namespace Mona {

void DetectMemoryLeak();


///// TYPES /////

typedef int8_t			Int8;
typedef uint8_t			UInt8;
typedef int16_t			Int16;
typedef uint16_t		UInt16;
typedef int32_t         Int32;
typedef uint32_t        UInt32;
typedef int64_t			Int64;
typedef uint64_t		UInt64;



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
	Object& operator=(Object&& other) = delete;
public:
	Object() {};
	virtual ~Object() {};
};

class NullableObject : public virtual Object {
public:
	NullableObject() {}
	virtual operator bool() const = 0;
};

inline UInt32 abs(double value) { return (UInt32)std::abs((long long int)value); }

////// ASCII ////////

class ASCII : virtual Static {
public:
	enum Type {
		CONTROL  = 0x0001,
		BLANK    = 0x0002,
		SPACE    = 0x0004,
		PUNCT    = 0x0008,
		DIGIT    = 0x0010,
		HEXDIGIT = 0x0020,
		ALPHA    = 0x0040,
		LOWER    = 0x0080,
		UPPER    = 0x0100,
		GRAPH    = 0x0200,
		PRINT    = 0x0400,
		XML		 = 0x0800
	};

	static UInt8 ToLower(char value) { return Is(value, UPPER) ? (value + 32) : value; }
	static UInt8 ToUpper(char value) { return Is(value, LOWER) ? (value - 32) : value; }

	static bool Is(char value,UInt16 type) {return value&0x80 ? 0 : ((_CharacterTypes[int(value)]&type) != 0);}
private:
	static const UInt16 _CharacterTypes[128];
};


inline bool isalnum(char value) { return ASCII::Is(value, ASCII::ALPHA | ASCII::DIGIT); }
inline bool isalpha(char value) { return ASCII::Is(value,ASCII::ALPHA); }
inline bool isblank(char value) { return ASCII::Is(value,ASCII::BLANK); }
inline bool iscntrl(char value) { return ASCII::Is(value,ASCII::CONTROL); }
inline bool isdigit(char value) { return ASCII::Is(value,ASCII::DIGIT); }
inline bool isgraph(char value) { return ASCII::Is(value,ASCII::GRAPH); }
inline bool islower(char value) { return ASCII::Is(value,ASCII::LOWER); }
inline bool isprint(char value) { return ASCII::Is(value,ASCII::PRINT); }
inline bool ispunct(char value) { return ASCII::Is(value,ASCII::PUNCT); }
inline bool isspace(char value) { return  ASCII::Is(value,ASCII::SPACE); }
inline bool isupper(char value) { return ASCII::Is(value,ASCII::UPPER); }
inline bool isxdigit(char value) { return ASCII::Is(value,ASCII::HEXDIGIT); }
inline bool isxml(char value) { return ASCII::Is(value,ASCII::XML); }
inline char tolower(char value) { return ASCII::ToLower(value); }
inline char toupper(char value) { return ASCII::ToUpper(value); }

const char* strrpbrk(const char* value, const char* markers);


} // namespace Mona
