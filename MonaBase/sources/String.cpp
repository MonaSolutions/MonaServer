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

#include "Mona/String.h"
#include "Mona/Exceptions.h"
#include <limits>
#include <cctype>

#if defined(_WIN32)
    #include "windows.h"
#endif

using namespace std;

namespace Mona {

#if defined(_WIN32) && (_MSC_VER < 1900)
	 // for setting number of exponent digits to 2
	static int output_exp_old_format(_set_output_format(_TWO_DIGIT_EXPONENT));
#endif

const string String::Empty;

size_t String::Split(const char* value, const char* separators, const String::ForEach& forEach, int options) {
	const char* it1(value);
	const char* it2(NULL);
	const char* it3(NULL);
	size_t count(0);

	for(;;) {
		if (options & SPLIT_TRIM) {
			while (*it1 && isspace(*it1))
				++it1;
		}
		it2 = it1;
		while (*it2 && !strchr(separators,*it2))
			++it2;
		it3 = it2;
		if ((options & SPLIT_TRIM) && it3 != it1) {
			--it3;
			while (it3 != it1 && isspace(*it3))
				--it3;
			if (!isspace(*it3))
				++it3;
		}
		if (it3 != it1 || !(options&SPLIT_IGNORE_EMPTY)) {
			bool result(false);
			SCOPED_STRINGIFY(it3, 0, result = forEach(count++,&*it1))
			if (!result)
				return string::npos;
		}
		it1 = it2;
		if (!*it1)
			break;
		++it1;
	};
	return count;
}

vector<string>& String::Split(const char* value, const char* separators, vector<string>& values, int options) {
	ForEach forEach([&values](UInt32 index,const char* value){
		values.emplace_back(value);
		return true;
	});
	Split(value, separators, forEach, options);
	return values;
}


string& String::Trim(string& value, TrimOption option) {
	const char* data(value.c_str());
	size_t		size(value.size());

	if (option & 1) {
		data = value.erase(0, TrimLeft(data)-data).data();
		size = value.size();
	}
	if (option & 2) {
		// trim right
		const char* it(data+size);
		while (it!=data && isspace(*--it))
			--size;
		value.resize(size);
	}
	return value;
}


int String::ICompare(const char* value1, const char* value2,  size_t size) {
	if (value1 == value2)
		return 0;
	if (!value1 || !(*value1))
		return -1;
	if (!value2 || !(*value2))
		return 1;

	// value1 and value2 size > 0

	int f(0), l(0);
	do {
		if (size == 0)
			return f - l;
		if (((f = (unsigned char)(*(value1++))) >= 'A') && (f <= 'Z'))
			f -= 'A' - 'a';
		if (((l = (unsigned char)(*(value2++))) >= 'A') && (l <= 'Z'))
			l -= 'A' - 'a';
		if (size != std::string::npos)
			--size;
	} while (f && (f == l));

	return(f - l);
}

template<typename Type>
bool String::ToNumber(const char* value, size_t size, Type& result)  {
	Exception ex;
	result = ToNumber<Type>(ex, result, value, size);
	return !ex;
}

template<typename Type>
Type String::ToNumber(Exception& ex, Type failValue, const char* value, size_t size) {
	int digit = 1, comma = 0;	
	bool beginning = true, negative = false;

	long double result(0);

	bool isSigned = numeric_limits<Type>::is_signed;
	Type max = numeric_limits<Type>::max();

	const char* current(value);
	if (size == string::npos)
		size = strlen(value);

	while(current && size-->0) {

		if (iscntrl(*current) || *current==' ') {
			if (beginning) {
				++current;
				continue;
			}
			ex.set(Exception::FORMATTING, value, " is not a correct number");
			return failValue;
		}

		if (*current == '-') {
			if (isSigned && beginning && !negative) {
				negative = true;
				++current;
				continue;
			}
			ex.set(Exception::FORMATTING, value, " is not a correct number");
			return failValue;
		}

		if (*current == '.' || *current == ',') {
			if (comma == 0 && !beginning) {
				comma = 1;
				++current;
				continue;
			}
			ex.set(Exception::FORMATTING, value, " is not a correct number");
			return failValue;
		}

		if (beginning)
			beginning = false;

		if (isdigit(*current) == 0) {
			ex.set(Exception::FORMATTING, value, " is not a correct number");
			return failValue;
		}

		result = result * 10 + (*current - '0');
		comma *= 10;
		++current;
	}

	if (beginning) {
		ex.set(Exception::FORMATTING, "Empty string is not a number");
		return failValue;
	}

	if (comma > 0)
		result /= comma;

	if (result > max) {
		ex.set(Exception::FORMATTING, value, " exceeds maximum number capacity");
		return failValue;
	}

	if (negative)
		result *= -1;

	return (Type)result;
}

#if defined(_WIN32)
const char* String::ToUTF8(const wchar_t* value,char buffer[_MAX_PATH]) {
	WideCharToMultiByte(CP_UTF8, 0, value, -1, buffer, _MAX_PATH, NULL, NULL);
	return buffer;
}
#endif

template float String::ToNumber(Exception& ex, float failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, float& result);

template double String::ToNumber(Exception& ex, double failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, double& result);

template UInt8 String::ToNumber(Exception& ex, UInt8 failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, UInt8& result);

template Int8 String::ToNumber(Exception& ex, Int8 failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, Int8& result);

template UInt16 String::ToNumber(Exception& ex, UInt16 failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, UInt16& result);

template Int16 String::ToNumber(Exception& ex, Int16 failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, Int16& result);

template UInt32 String::ToNumber(Exception& ex, UInt32 failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, UInt32& result);

template Int32 String::ToNumber(Exception& ex, Int32 failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, Int32& result);

template UInt64 String::ToNumber(Exception& ex, UInt64 failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, UInt64& result);

template Int64 String::ToNumber(Exception& ex, Int64 failValue, const char* value, size_t size);
template bool  String::ToNumber(const char* value, size_t size, Int64& result);

} // namespace Mona
