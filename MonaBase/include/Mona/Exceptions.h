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
#include <map>
#include "assert.h"
#include "errno.h"

namespace Mona {

class Exception : public virtual Object {
public:
	///// ADD TOO in Exceptions.cpp!!
	enum Code {
		NIL=0,
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
		NETIP,
		NETPORT,
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
	Exception& set(Code code, Args&&... args) {
		_code = code;
		String::Format(_error, args ...);
		if (_code && _error.empty()) 
			_error.assign(_CodeMessages[code]);
		return *this;
	}

	Exception& operator=(const Exception& other) {
		_code = other._code;
		_error = other._error;
		return *this;
	}
	Exception& operator=(const Exception* pOther) {
		if (pOther)
			return operator=(*pOther);
		_code = Exception::NIL;
		_error.clear();
		return *this;
	}

	operator bool() const { return _code != Exception::NIL; }

	const char*	error() const { return _error.empty() ? "Unknown error" : _error.c_str(); }
	Code		code() const { return _code; }

private:

	Code		_code;
	std::string	_error;

	static std::map<Code, const char*> _CodeMessages;
};

#undef		ASSERT
#define		ASSERT(CONDITION)					if(!(CONDITION)) { ex.set(Exception::ASSERT, #CONDITION," assertion");return;}
#define		ASSERT_RETURN(CONDITION,RETURN)		if(!(CONDITION)) { ex.set(Exception::ASSERT, #CONDITION," assertion");return RETURN;}

#if defined(_DEBUG)
	#if defined(_WIN32)
		#define		FATAL_ASSERT(CONDITION)			{_ASSERTE(CONDITION);}
	#else
		#define		FATAL_ASSERT(CONDITION)			{assert(CONDITION);}
	#endif
	#if defined(_WIN32)
		#define		FATAL_ERROR(...)				{std::string __error;Mona::String::Format(__error,## __VA_ARGS__);if (_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, __error.c_str()) == 1) _CrtDbgBreak();}
	#elif defined(_OS_BSD) // BSD has no asser_fail function
		#define		FATAL_ERROR(...)				{std::string __error; throw std::runtime_error(Mona::String::Format(__error,## __VA_ARGS__,", " __FILE__ "[" LINE_STRING "]"));}
	#else
		#define		FATAL_ERROR(...)				{std::string __error;Mona::String::Format(__error,## __VA_ARGS__).c_str();__assert_fail(__error.c_str(),__FILE__,__LINE__,NULL);}
	#endif
#else
	#define		FATAL_ASSERT(CONDITION)				if(!(CONDITION)) {throw std::runtime_error( #CONDITION " assertion, " __FILE__ "[" LINE_STRING "]");}
	#define		FATAL_ERROR(...)					{std::string __error; throw std::runtime_error(Mona::String::Format(__error,## __VA_ARGS__,", " __FILE__ "[" LINE_STRING "]"));}
#endif

#define EXCEPTION_TO_LOG(FUNCTION,...) if(FUNCTION) { if(ex)  WARN( __VA_ARGS__,", ", ex.error()); } else { ERROR( __VA_ARGS__,", ", ex.error()) }

} // namespace Mona
