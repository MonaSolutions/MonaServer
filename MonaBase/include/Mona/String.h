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
#pragma warning(disable:4146)

#if defined(_MSC_VER) || defined(__MINGW32__)
	#define I64_FMT "I64"
#elif defined(__APPLE__) 
	#define I64_FMT "q"
#else
	#define I64_FMT "ll"
#endif

namespace Mona {

template<typename Type>
class Format : ObjectFix {
public:
	Format(const char* format, Type value) : value(value), format(format) {}
	const Type	value;
	const char* format;
};

class Exception;

/// Utility class for generation parse of strings
class String : Fix {

public:

	enum SplitOption {
		TOK_IGNORE_EMPTY = 1, /// ignore empty tokens
		TOK_TRIM = 2  /// remove leading and trailing whitespace from tokens
	};

	static void Split(const std::string& value, const std::string& separators, std::vector<std::string>& values,int options = 0);
	
	template<typename T>
	static T ToNumber(Exception& ex,const std::string& value) { return ToNumber<T>(ex,value.c_str()); }

	template<typename T>
	static T ToNumber(Exception& ex,const char* value) {
		int digit = 1, comma = 0;
		bool beginning = true, negative = false;
		T resultat = 0;

		if (!value || *value == '\0') {
			ex.set(Exception::FORMATTING, "Empty string is not a number");
			return false;
		}
		

		T max = numeric_limits<T>::max();

		do {
			if (resultat >= max) {
				ex.set(Exception::FORMATTING, value," exceeds maximum number capacity");
				return false;
			}

			if (isblank(*value)) {
				if (beginning)
					continue;
				ex.set(Exception::FORMATTING, value, " is not a corect number");
				return false;
			}

			if (*value == '-') {
				if (beginning && !negative) {
					negative = true;
					continue;
				}
				ex.set(Exception::FORMATTING, value, " is not a corect number");
				return false;
			}

			if (beginning)
				beginning = false;

			if (*value == '.') {
				if (comma == 0) {
					comma = 1;
					continue;
				}
				ex.set(Exception::FORMATTING, value, " is not a corect number");
				return false;
			}

			if (isdigit(*value) == 0) {
				ex.set(Exception::FORMATTING, value, " is not a corect number");
				return false;
			}

			resultat = resultat * 10 + (*value - '0');
			comma *= 10;
		} while ((*++value) != '\0');

		if (comma > 0)
			resultat /= comma;


		if (negative)
			resultat = -resultat;
		
		return resultat;
	}

	static const std::string& toLower(std::string& str);

	template <typename ...Args>
	static const std::string& Format(std::string& result, const Args&... args) {
		result.clear();
		return String::Append(result, args ...);
	}
	
	/// \brief match "char*" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, const char* value, const Args&... args) {
		result.append(value);
		return String::Append(result, args ...);
	}

	/// \brief match "std::string" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, const std::string& value, const Args&... args) {
		result.append(value);
		return String::Append(result, args ...);
	}

	/// \brief match "int" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, int value, const Args&... args) {
		sprintf_s(_Buffer, "%d", value);
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "long" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, long value, const Args&... args) {
		sprintf_s(_Buffer, "%ld", value);
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "unsigned int" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, unsigned int value, const Args&... args) {
		sprintf_s(_Buffer, "%u", value);
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "unsigned long" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, unsigned long value, const Args&... args) {
		sprintf_s(_Buffer, "%lu", value);
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "Int64" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, Int64 value, const Args&... args) {
		sprintf_s(_Buffer, "%" I64_FMT "d", value);
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "UInt64" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, UInt64 value, const Args&... args) {
		sprintf_s(_Buffer, "%" I64_FMT "u", value);
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "float" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, float value, const Args&... args) {
		sprintf_s(_Buffer, "%.8g", value);
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

	/// \brief match "double" case
	template <typename ...Args>
	static const std::string& Append(std::string& result, double value, const Args&... args) {
		sprintf_s(_Buffer, "%.16g", value);
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

	/// \brief match pointer case
	template <typename ...Args>
	static const std::string& Append(std::string& result, const void* value, const Args&... args)	{

		sprintf_s(_Buffer, "%08lX", (UIntPtr) value);
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

	/// \brief A usefull form which use snprintf to format result
	///
	/// \param result This is the std::string which to append text
	/// \param value A pair of format text associate with value (ex: pair<char*, double>("%.2f", 10))
	/// \param args Other arguments to append
	template <class Type, typename ...Args>
	static const std::string& Append(std::string& result, const Mona::Format<Type>& custom, const Args&... args) {
		try {
			snprintf(_Buffer, sizeof(_Buffer), custom.format, custom.value);
		}
		catch (...) {
			// TODO remove the loop => ERROR("String formatting error during Append(...)");
			return result;
		}
		result.append(_Buffer);
		return String::Append(result, args ...);
	}

private:

	static const std::string& Append(std::string& result) { return result; }

	static char			_Buffer[64];
};


} // namespace Mona

