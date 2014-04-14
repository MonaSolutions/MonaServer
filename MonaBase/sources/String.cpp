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

using namespace std;

namespace Mona {

const string String::Empty;

#if defined(WIN32)
	int String::output_exp_old_format = _set_output_format(_TWO_DIGIT_EXPONENT);
#endif

vector<string>& String::Split(const string& value, const string& separators, vector<string>& values, int options) {
	string::const_iterator it1 = value.begin(), it2, it3, end = value.end();

	while (it1 != end) {
		if (options & SPLIT_TRIM) {
			while (it1 != end && isspace(*it1))
				++it1;
		}
		it2 = it1;
		while (it2 != end && separators.find(*it2) == string::npos)
			++it2;
		it3 = it2;
		if (it3 != it1 && (options & SPLIT_TRIM)) {
			--it3;
			while (it3 != it1 && isspace(*it3))
				--it3;
			if (!isspace(*it3))
				++it3;
		}
		if (options & SPLIT_IGNORE_EMPTY) {
			if (it3 != it1)
				values.emplace_back(it1, it3);
		} else
			values.emplace_back(it1, it3);
		it1 = it2;
		if (it1 != end)
			++it1;
	}
	return values;
}

string& String::ToLower(string& value) {
	auto it = value.begin();
	for (it; it != value.end(); ++it)
		*it = tolower(*it);
	return value;
}

char* String::ToLower(char* value) {
	const char* end = value+strlen(value);
	while (value < end) {
		(*value) = tolower(*value);
		++value;
	}
	return value;
}


string& String::Trim(string& value, TrimOption option) {
	int first = 0;
	int last = value.size() - 1;

	if (option & 1) {
		while (first <= last && isspace(value[first]))
			++first;
	}
	if (option & 2) {
		while (last >= first && isspace(value[last]))
			--last;
	}

	value.resize(last + 1);
	value.erase(0, first);
	return value;
}

int String::ICompare(const char* value1, const char* value2,  size_t size) {
	if (value1 == value2)
		return 0;
	if (value1 == NULL)
		return -1;
	if (value2 == NULL)
		return 1;

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

template<typename T>
bool String::ToNumber(const std::string& value, T& result) {
	Exception ex;
	result = ToNumber<T>(ex, value, result);
	return !ex;
}

template<typename T>
T String::ToNumber(Exception& ex, const std::string& value, T result) {
	int digit = 1, comma = 0;
	bool beginning = true, negative = false;

	const char* str(value.c_str());
	long double current(0);

	if (!str || *str == '\0') {
		ex.set(Exception::FORMATTING, "Empty string is not a number");
		return false;
	}

	bool isSigned = numeric_limits<T>::is_signed;
	T max = numeric_limits<T>::max();

	do {
		if (isblank(*str)) {
			if (beginning)
				continue;
			ex.set(Exception::FORMATTING, str, " is not a correct number");
			return false;
		}

		if (*str == '-') {
			if (isSigned && beginning && !negative) {
				negative = true;
				continue;
			}
			ex.set(Exception::FORMATTING, str, " is not a correct number");
			return false;
		}

		if (*str == '.' || *str == ',') {
			if (comma == 0 && !beginning) {
				comma = 1;
				continue;
			}
			ex.set(Exception::FORMATTING, str, " is not a correct number");
			return false;
		}

		if (beginning)
			beginning = false;

		if (isdigit(*str) == 0) {
			ex.set(Exception::FORMATTING, str, " is not a correct number");
			return false;
		}

		current = current * 10 + (*str - '0');
		comma *= 10;
	} while ((*++str) != '\0');

	if (comma > 0)
		current /= comma;

	if (current > max) {
		ex.set(Exception::FORMATTING, str, " exceeds maximum number capacity");
		return false;
	}

	if (negative)
		current *= -1;

	return result = (T)current;
}


template bool String::ToNumber(const std::string& value, float& result);
template float String::ToNumber(Exception& ex, const std::string& value, float result);

template bool String::ToNumber(const std::string& value, double& result);
template double String::ToNumber(Exception& ex, const std::string& value, double result);

template bool String::ToNumber(const std::string& value, UInt8& result);
template UInt8 String::ToNumber(Exception& ex, const std::string& value, UInt8 result);

template bool String::ToNumber(const std::string& value, Int8& result);
template Int8 String::ToNumber(Exception& ex, const std::string& value, Int8 result);

template bool String::ToNumber(const std::string& value, UInt16& result);
template UInt16 String::ToNumber(Exception& ex, const std::string& value, UInt16 result);

template bool String::ToNumber(const std::string& value, Int16& result);
template Int16 String::ToNumber(Exception& ex, const std::string& value, Int16 result);

template bool String::ToNumber(const std::string& value, UInt32& result);
template UInt32 String::ToNumber(Exception& ex, const std::string& value, UInt32 result);

template bool String::ToNumber(const std::string& value, Int32& result);
template Int32 String::ToNumber(Exception& ex, const std::string& value, Int32 result);

template bool String::ToNumber(const std::string& value, UInt64& result);
template UInt64 String::ToNumber(Exception& ex, const std::string& value, UInt64 result);

template bool String::ToNumber(const std::string& value, Int64& result);
template Int64 String::ToNumber(Exception& ex, const std::string& value, Int64 result);

} // namespace Mona
