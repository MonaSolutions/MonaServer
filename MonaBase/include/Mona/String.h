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

#include "Mona/Mona.h"
#include "Mona/Buffer.h"
#include <functional>
#include <vector>

#undef max

namespace Mona {

#define EXPAND(VALUE)		VALUE"",(sizeof(VALUE)-1) // "" concatenation is here to check that it's a valid const string is not a pointer of char*

template<typename Type>
class Format : public virtual Object {
public:
	Format(const char* format, Type value) : value(value), format(format) {}
	const Type	value;
	const char* format;
};

class Exception;

/// Utility class for generation parse of strings
class String : virtual Static {
public:

	enum SplitOption {
		SPLIT_IGNORE_EMPTY = 1, /// ignore empty tokens
		SPLIT_TRIM = 2,  /// remove leading and trailing whitespace from tokens
	};

	enum TrimOption {
		TRIM_BOTH = 3,
		TRIM_LEFT = 1,
		TRIM_RIGHT = 2
	};

	typedef std::function<bool(UInt32 index,const char* value)> ForEach; /// String::Split function type handler

	static std::size_t Split(const std::string& value, const char* separators, const String::ForEach& forEach, int options = 0) { return Split(value.data(), separators, forEach, options); }
	static std::size_t Split(const char* value, const char* separators, const String::ForEach& forEach, int options = 0);
	static std::vector<std::string>& Split(const char* value, const char* separators, std::vector<std::string>& values, int options = 0);
	static std::vector<std::string>& Split(const std::string& value, const char* separators, std::vector<std::string>& values, int options = 0) { return Split(value.c_str(),separators,values,options); }

	static std::string& Trim(std::string& value, TrimOption option = TRIM_BOTH);
	static const char*	TrimLeft(const char* value) { while (*value && isspace(*value)) ++value; return value; }
	
	static std::string&	ToLower(std::string& value) { for (auto it = value.begin(); it != value.end(); ++it) *it = tolower(*it); return value; }

	static int ICompare(const char* value1, const char* value2,  std::size_t size = std::string::npos);
	static int ICompare(const std::string& value1, const std::string& value2, std::size_t size = std::string::npos) { return ICompare(value1.empty() ? NULL : value1.c_str(), value2.empty() ? NULL : value2.c_str(), size); }
	static int ICompare(const std::string& value1, const char* value2,  std::size_t size = std::string::npos) { return ICompare(value1.empty() ? NULL : value1.c_str(), value2, size); }
	static int ICompare(const char* value1, const std::string& value2,  std::size_t size = std::string::npos) { return ICompare(value1, value2.empty() ? NULL : value2.c_str(), size); }

	
	template<typename Type>
	static bool ToNumber(const std::string& value, Type& result) { return ToNumber(value.data(), value.size(), result); }
	template<typename Type>
	static bool ToNumber(const char* value, Type& result) { return ToNumber(value, std::string::npos, result); }
	template<typename Type>
	static bool ToNumber(const char* value, std::size_t size, Type& result);

	template<typename Type>
	static Type ToNumber(Exception& ex, const std::string& value) { return ToNumber<Type>(ex, 0, value.data(),value.size()); }
	template<typename Type>
	static Type ToNumber(Exception& ex, const char* value, std::size_t size = std::string::npos) { return ToNumber<Type>(ex, 0, value, size ); }
	template<typename Type>
	static Type ToNumber(Exception& ex, Type failValue, const std::string& value) { return ToNumber<Type>(ex, failValue, value.data(),value.size()); }
	template<typename Type>
	static Type ToNumber(Exception& ex, Type failValue, const char* value, std::size_t size = std::string::npos);

	static bool IsTrue(const std::string& value) { return IsTrue(value.data(),value.size()); }
	static bool IsTrue(const char* value,std::size_t size=std::string::npos) { return ICompare(value, "1", size) == 0 || String::ICompare(value, "true", size) == 0 || String::ICompare(value, "yes", size) == 0 || String::ICompare(value, "on", size) == 0; }
	static bool IsFalse(const std::string& value) { return IsFalse(value.data(),value.size()); }
	static bool IsFalse(const char* value, std::size_t size = std::string::npos) { return ICompare(value, "0", size) == 0 || String::ICompare(value, "false", size) == 0 || String::ICompare(value, "no", size) == 0 || String::ICompare(value, "off", size) == 0; }


	static const std::string Empty;

	template <typename BufferType, typename ...Args>
	static BufferType& Format(BufferType& result, Args&&... args) {
		result.clear();
		return String::Append(result, args ...);
	}

	/// \brief match "std::string" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, const std::string& value, Args&&... args) {
		return String::Append(Buffer::Append<BufferType>(result,value.data(), value.size()), args ...);
	}

	
	/// \brief match "const char*" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, const char* value, Args&&... args) {
		return String::Append(Buffer::Append<BufferType>(result,value, strlen(value)), args ...);
	}

#if defined(_WIN32)
	/// \brief match "wstring" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, const std::wstring& value, Args&&... args) {
		return String::Append(value.c_str(), args ...);
	}
	
	/// \brief match "const wchar_t*" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, const wchar_t* value, Args&&... args) {
		char buffer[_MAX_PATH];
		ToUTF8(value, buffer);
		return String::Append(Buffer::Append<BufferType>(result,buffer, strlen(buffer)), args ...);
	}
#endif

	// match le "char" cas
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, char value, Args&&... args) {
		return String::Append(Buffer::Append(result,&value,1), args ...);
	}

	// match le "signed char" cas
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, signed char value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hhd", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "short" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, short value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hd", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "int" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, int value, Args&&... args) {
		char buffer[16];
		sprintf(buffer, "%d", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "long" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%ld", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "unsigned char" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, unsigned char value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hhu", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "unsigned short" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, unsigned short value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hu", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "unsigned int" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, unsigned int value, Args&&... args) {
		char buffer[16];
		sprintf(buffer, "%u", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "unsigned long" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, unsigned long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%lu", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "Int64" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, long long value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%lld", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "UInt64" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, unsigned long long value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%llu", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "float" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, float value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%.8g", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "double" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, double value, Args&&... args) {
		char buffer[64];
		sprintf(buffer, "%.16g", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief match "bool" case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, bool value, Args&&... args) {
		if (value)
			return String::Append(Buffer::Append(result,EXPAND("true")), args ...);
		return String::Append(Buffer::Append(result,EXPAND("false")), args ...);
	}

	/// \brief match pointer case
	template <typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, const void* value, Args&&... args)	{
		char buffer[64];
		sprintf(buffer,"%p", value);
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	/// \brief A usefull form which use snprintf to format result
	///
	/// \param result This is the std::string which to append text
	/// \param value A pair of format text associate with value (ex: pair<char*, double>("%.2f", 10))
	/// \param args Other arguments to append
	template <typename Type, typename BufferType, typename ...Args>
	static BufferType& Append(BufferType& result, const Mona::Format<Type>& custom, Args&&... args) {
		char buffer[64];
		try {
            snprintf(buffer, sizeof(buffer), custom.format, custom.value);
		}
		catch (...) {
			return String::Append(result, args ...);
		}
		return String::Append(Buffer::Append<BufferType>(result,buffer,strlen(buffer)), args ...);
	}

	
	template <typename BufferType>
	static BufferType& Append(BufferType& result) { return result; }

private:

#if defined(_WIN32)
	static const char* ToUTF8(const wchar_t* value, char buffer[_MAX_PATH]);
#endif
};


} // namespace Mona

