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
#include "Mona/Logs.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
	#define I64_FMT "I64"
#elif defined(__APPLE__) 
	#define I64_FMT "q"
#else
	#define I64_FMT "ll"
#endif

namespace Mona {

/// This formatting class use concatenations of std::string (faster than using stream),
/// and sprintf
class String {

public:

	/// \brief match "char*" case
	template <typename ...Args>
	static std::string& Format(std::string& result, const char* value, Args... args) {
		result.append(value);
		return String::Format(result, args ...);
	}

	template <typename ...Args>
	static std::string Format(const char* value, Args... args) {
		_result.clear();
		String::Format(_result, value, args ...);
		// TODO verify the move usage (does we must use string& or string&& as return type?)
		return move(_result);
	}

	/// \brief match "std::string" case
	template <typename ...Args>
	static std::string& Format(std::string& result, const std::string& value, Args... args) {
		result.append(value);
		return String::Format(result, args ...);
	}

	/// \brief match "int" case
	template <typename ...Args>
	static std::string& Format(std::string& result, int value, Args... args) {
		sprintf_s(_Buffer, "%d", value);
		result.append(_Buffer);
		return String::Format(result, args ...);
	}

	template <typename ...Args>
	static std::string Format(int value, Args... args) {
		_result.clear();
		String::Format(_result, value, args ...);
		return move(_result);
	}

	/// \brief match "long" case
	template <typename ...Args>
	static std::string& Format(std::string& result, long value, Args... args) {
		sprintf_s(_Buffer, "%ld", value);
		result.append(_Buffer);
		return String::Format(result, args ...);
	}

	template <typename ...Args>
	static std::string Format(long value, Args... args) {
		_result.clear();
		String::Format(_result, value, args ...);
		return move(_result);
	}

	/// \brief match "unsigned int" case
	template <typename ...Args>
	static std::string& Format(std::string& result, unsigned int value, Args... args) {
		sprintf_s(_Buffer, "%u", value);
		result.append(_Buffer);
		return String::Format(result, args ...);
	}

	template <typename ...Args>
	static std::string Format(unsigned int value, Args... args) {
		_result.clear();
		String::Format(_result, value, args ...);
		return move(_result);
	}

	/// \brief match "unsigned long" case
	template <typename ...Args>
	static std::string& Format(std::string& result, unsigned long value, Args... args) {
		sprintf_s(_Buffer, "%lu", value);
		result.append(_Buffer);
		return String::Format(result, args ...);
	}

	template <typename ...Args>
	static std::string Format(unsigned long value, Args... args) {
		_result.clear();
		String::Format(_result, value, args ...);
		return move(_result);
	}

	/// \brief match "Int64" case
	template <typename ...Args>
	static std::string& Format(std::string& result, Int64 value, Args... args) {
		sprintf_s(_Buffer, "%" I64_FMT "d", value);
		result.append(_Buffer);
		return String::Format(result, args ...);
	}

	template <typename ...Args>
	static std::string Format(Int64 value, Args... args) {
		_result.clear();
		String::Format(_result, value, args ...);
		return move(_result);
	}

	/// \brief match "UInt64" case
	template <typename ...Args>
	static std::string& Format(std::string& result, UInt64 value, Args... args) {
		sprintf_s(_Buffer, "%" I64_FMT "u", value);
		result.append(_Buffer);
		return String::Format(result, args ...);
	}

	template <typename ...Args>
	static std::string Format(UInt64 value, Args... args) {
		_result.clear();
		String::Format(_result, value, args ...);
		return move(_result);
	}

	/// \brief match "double" and "float" case
	template <typename ...Args>
	static std::string& Format(std::string& result, double value, Args... args) {
		sprintf_s(_Buffer, "%g", value);
		result.append(_Buffer);
		return String::Format(result, args ...);
	}

	template <typename ...Args>
	static std::string Format(double value, Args... args) {
		_result.clear();
		String::Format(_result, value, args ...);
		return move(_result);
	}

	/// \brief A usefull form which use snprintf to format result
	///
	/// \param result This is the std::string which to append text
	/// \param value A pair of format text associate with value (ex: pair<char*, double>("%.2f", 10))
	/// \param args Other arguments to append
	template <typename T, typename ...Args>
	static std::string& Format(std::string& result, std::pair<const char*, T>& value, Args... args) {
		try {
			snprintf(_Buffer, sizeof(_Buffer), value.first, value.second);
		}
		catch (...) {
			ERROR("String formatting error during Format(...)");
			return result;
		}
		result.append(_Buffer);
		return String::Format(result, args ...);
	}

	template <typename T, typename ...Args>
	static std::string Format(std::pair<const char*, T>& value, Args... args) {
		_result.clear();
		String::Format(_result, value, args ...);
		return move(_result);
	}

private:

	static std::string& Format(std::string& result) { return result; }

	static char			_Buffer[64];
	static std::string		_result;
};

} // namespace Mona

