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

namespace Mona {


class Exception : ObjectFix {

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
		ASSERT
	};

	Exception() : _code(NIL) {}

	template <typename ...Args>
	void set(Code code, const Args&... args);

	void set(const Exception& other);

	operator bool() const { return !_error.empty() || _code != Exception::NIL; }

	const std::string&	error() const { return _error; }
	Code				code() const { return _code; }

private:
	void reset();

	Code		_code;
	std::string	_error;
};

#undef		ASSERT
#define		ASSERT(CHECK)					if(!(CHECK)) { ex.set(Exception::ASSERT, #CHECK);return;}
#define		ASSERT_RETURN(CHECK,RETURN)		if(!(CHECK)) { ex.set(Exception::ASSERT, #CHECK);return RETURN;}


} // namespace Mona
