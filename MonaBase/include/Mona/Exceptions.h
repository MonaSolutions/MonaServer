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
#include "Mona/String.h"

namespace Mona {


class Exception : virtual Object{
public:
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
		ASSERT
	};

	Exception() : _code(NIL) {}


	template <typename ...Args>
	void set(Code code, const Args&... args) {
		_code = code;
		String::Append(_error, args ...);
	}

	void set(const Exception& other) {
		_code = other._code;
		_error = other._error;
	}

	operator bool() const { return !_error.empty() || _code != Exception::NIL; }

	const std::string&	error() const { return _error; }
	Code				code() const { return _code; }

private:

	Code		_code;
	std::string	_error;
};

#undef		ASSERT
#define		ASSERT(CHECK)					if(!(CHECK)) { ex.set(Exception::ASSERT, #CHECK);return;}
#define		ASSERT_RETURN(CHECK,RETURN)		if(!(CHECK)) { ex.set(Exception::ASSERT, #CHECK);return RETURN;}

#define		FATAL_ASSERT(CHECK)				if(!(CHECK)) {throw std::exception( #CHECK ", "__FILE__"["LINE_STRING"]");}
#define		FATAL_THROW(...)				{string __error; throw std::exception( Mona::String::Format(__error,## __VA_ARGS__,", "__FILE__"["LINE_STRING"]").c_str());}

#define		EXCEPTION_TO_LOG(CALL,...)		{ bool __success = CALL; if (ex) { if (!__success) ERROR(## __VA_ARGS__,", ",ex.error()) else WARN(## __VA_ARGS__,", ", ex.error()); } else if (!__success) ERROR(## __VA_ARGS__,", unknown error"); }


} // namespace Mona
