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
#include "Mona/DataReader.h"
#include "Mona/Time.h"
#include <deque>


namespace Mona {


class HTTPHeaderReader : public DataReader, virtual Object {
public:
	HTTPHeaderReader(std::deque<const char*>& headers);


	std::string&		readString(std::string& value) {return _value.assign(value);}
	double				readNumber();
	bool				readBoolean() { return _bool; }
	Time&				readTime(Time& time) {return time.update(_date);}
	void				readNull() {}

	bool				readObject(std::string& type, bool& external) { return true; }
	Type				readItem(std::string& name);
	
	Type				followingType() { return _header == _headers.end() ? END : OBJECT; }
	bool				available() { return _header != _headers.end(); }

	void				reset();

private:
	bool				readArray(UInt32& size) {return false;}
	const UInt8*		readBytes(UInt32& size) { size = _value.size(); return (const UInt8*)_value.c_str(); }
	
	std::deque<const char*>&				_headers;
	std::deque<const char*>::const_iterator _header;
	std::string								_value;
	Time									_date;
	bool									_bool;
};



} // namespace Mona
