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

#include "Mona/Mona.h"
#include <vector>
#include <limits>

#undef max
// TODO? #pragma warning(disable:4146)

#if defined(_MSC_VER) || defined(__MINGW32__)
	#define I64_FMT "I64"
#elif defined(__APPLE__) 
	#define I64_FMT "q"
#else
	#define I64_FMT "ll"
#endif

namespace Mona {

template<typename Type>
class Format : virtual Object {
public:
	Format(const char* format, Type value) : value(value), format(format) {}
	const Type	value;
	const char* format;
};

class Exception;

/// Utility class for generation parse of strings
class String : virtual Static{
public:

	enum SplitOption {
		SPLIT_IGNORE_EMPTY = 1, /// ignore empty tokens
		SPLIT_TRIM = 2  /// remove leading and trailing whitespace from tokens
	};

	enum TrimOption {
		TRIM_BOTH = 3,
		TRIM_LEFT = 1,
		TRIM_RIGHT = 2
	};

	static std::vector<std::string>& Split(const std::string& value, const std::string& separators, std::vector<std::string>& values, int options = 0);
	static std::string& Trim(std::string& value, TrimOption option = TRIM_BOTH);

	static std::string&	ToLower(std::string& str);

	static int ICompare(const char* value1, const char* value2, int size = -1);
	static int ICompare(const std::string& value1, const std::string& value2, int size = -1) { return ICompare(value1.empty() ? NULL : value1.c_str(), value2.empty() ? NULL : value2.c_str(), size); }
	static int ICompare(const std::string& value1, const char* value2, int size = -1) { return ICompare(value1.empty() ? NULL : value1.c_str(), value2, size); }
	static int ICompare(const char* value1, const std::string& value2, int size = -1) { return ICompare(value1, value2.empty() ? NULL : value2.c_str(), size); }

	template <typename ...Args>
	static std::string& Format(std::string& result, Args&&... args) {
		result.clear();
		return String::Append(result, args ...);
	}
	
	/// \brief match "char*" case
	template <typename ...Args>
	static std::string& Append(std::string& result, const char* value, Args&&... args) {
		result.append(value);
		return String::Append(result, args ...);
	}

	/// \brief match "std::string" case
	template <typename ...Args>
	static std::string& Append(std::string& result, const std::string& value, Args&&... args) {
		result.append(value);
		return String::Append(result, args ...);
	}

	// match le "char" cas
	template <typename ...Args>
	static std::string& Append(std::string& result, char value, Args&&... args) {
		result.append(1, value);
		return String::Append(result, args ...);
	}

	/// \brief match "int" case
	template <typename ...Args>
	static std::string& Append(std::string& result, int value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%d", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "long" case
	template <typename ...Args>
	static std::string& Append(std::string& result, long value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%ld", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "unsigned char" case
	template <typename ...Args>
	static std::string& Append(std::string& result, unsigned char value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%hu", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "unsigned short" case
	template <typename ...Args>
	static std::string& Append(std::string& result, unsigned short value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%hu", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "unsigned int" case
	template <typename ...Args>
	static std::string& Append(std::string& result, unsigned int value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%u", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "unsigned long" case
	template <typename ...Args>
	static std::string& Append(std::string& result, unsigned long value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%lu", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "Int64" case
	template <typename ...Args>
	static std::string& Append(std::string& result, Int64 value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%" I64_FMT "d", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "UInt64" case
	template <typename ...Args>
	static std::string& Append(std::string& result, UInt64 value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%" I64_FMT "u", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "float" case
	template <typename ...Args>
	static std::string& Append(std::string& result, float value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%.8g", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "double" case
	template <typename ...Args>
	static std::string& Append(std::string& result, double value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%.16g", value);
		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief match pointer case
	template <typename ...Args>
	static std::string& Append(std::string& result, const void* value, Args&&... args)	{
		char buffer[64];
		
		#if defined(MONA_PTR_IS_64_BIT)
            sprintf(buffer, "%016" I64_FMT "X", value);
		#else
            sprintf(buffer, "%08lX", value);
		#endif

		result.append(buffer);
		return String::Append(result, args ...);
	}

	/// \brief A usefull form which use snprintf to format result
	///
	/// \param result This is the std::string which to append text
	/// \param value A pair of format text associate with value (ex: pair<char*, double>("%.2f", 10))
	/// \param args Other arguments to append
	template <class Type, typename ...Args>
	static std::string& Append(std::string& result, const Mona::Format<Type>& custom, Args&&... args) {
		char buffer[64];
		try {
            snprintf(buffer, sizeof(buffer), custom.format, custom.value);
		}
		catch (...) {
			// TODO remove the loop => ERROR("String formatting error during Append(...)");
			return result;
		}
		result.append(buffer);
		return String::Append(result, args ...);
	}

	template<typename T>
	static bool ToNumber(const std::string& value, T& result);

	template<typename T>
	static T ToNumber(Exception& ex, const std::string& value, T result=0);

	static const std::string Empty;
private:

	static std::string& Append(std::string& result) { return result; }
#if defined(WIN32)
	static int output_exp_old_format; // for setting number of exponent digits to 2
#endif
};


} // namespace Mona

