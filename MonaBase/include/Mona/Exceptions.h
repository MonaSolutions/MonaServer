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
#include "Mona/String.h"
#include <stdexcept>
#include <map>

namespace Mona {

class Exception : virtual Object {
public:
	///// ADD TOO in Exceptions.cpp!!
	enum Code {
		NIL,
		APPLICATION = 1,
		SOFTWARE,
		FILE,
		ARGUMENT,
		OPTION,
		SERVICE,
		REGISTRY,
		PROTOCOL,
		NETWORK,
		SOCKET,
		NETADDRESS,
		FORMATTING,
		THREAD,
		MEMORY,
		SYSTEM,
		MATH,
		CRYPTO,
		PERMISSION,
		ASSERT
	};

	Exception() : _code(NIL) {}


	template <typename ...Args>
	void set(Code code, Args&&... args) {
		_code = code;
		String::Format(_error, args ...);
		if (_code != Exception::NIL && _error.empty()) 
			_error.assign(_CodeMessages[code]);
	}

	void set(const Exception& other) {
		_code = other._code;
		_error = other._error;
	}

	operator bool() const { return _code != Exception::NIL; }

	const std::string&	error() const { return _error; }
	Code				code() const { return _code; }

private:

	Code		_code;
	std::string	_error;

	static std::map<Code, const char*> _CodeMessages;
};

#undef		ASSERT
#define		ASSERT(CONDITION)					if(!(CONDITION)) { ex.set(Exception::ASSERT, #CONDITION);return;}
#define		ASSERT_RETURN(CONDITION,RETURN)		if(!(CONDITION)) { ex.set(Exception::ASSERT, #CONDITION);return RETURN;}

#if defined(_DEBUG)
#if defined(_WIN32)
#define		FATAL_ASSERT(CONDITION)				{_ASSERTE(CONDITION);}
#else
#define		FATAL_ASSERT(CONDITION)				if(!(CONDITION)) {raise(SIGTRAP);} // TODO test on linux
#endif
#if defined(_WIN32)
#define		FATAL_ERROR(...)				{string __error;_ASSERTE(!Mona::String::Format(__error,## __VA_ARGS__).c_str());}
#else
#define		FATAL_ERROR(...)				{raise(SIGTRAP);} // TODO test on linux
#endif
#else
#define		FATAL_ASSERT(CONDITION)				if(!(CONDITION)) {throw std::runtime_error( #CONDITION ", " __FILE__ "[" LINE_STRING "]");}
#define		FATAL_ERROR(...)				{string __error; throw std::runtime_error(Mona::String::Format(__error,## __VA_ARGS__,", " __FILE__ "[" LINE_STRING "]"));}
#endif

#define		EXCEPTION_TO_LOG(CALL,...)		{ \
    bool __success = CALL; \
    if (ex) { if (!__success) ERROR( __VA_ARGS__,", ",ex.error()) else WARN( __VA_ARGS__,", ", ex.error()); } \
    else if (!__success) ERROR( __VA_ARGS__,", unknown error"); }

} // namespace Mona
