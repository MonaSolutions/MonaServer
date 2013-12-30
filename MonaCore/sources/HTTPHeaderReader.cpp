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

#include "Mona/HTTPHeaderReader.h"
#include "Mona/Logs.h"
#include "Mona/TimeParser.h"
#include "Mona/Exceptions.h"


using namespace std;


namespace Mona {


HTTPHeaderReader::HTTPHeaderReader(deque<const char*>& headers):DataReader(MemoryReader::Null),_bool(false),_date(0),_header(headers.begin()),_headers(headers) {
}


void HTTPHeaderReader::reset() {
	_header = _headers.begin();
	_value.clear();
	_date.update(0);
	_bool=false;
}

HTTPHeaderReader::Type HTTPHeaderReader::readItem(string& name) {
	if (_header == _headers.end())
		return END;
	auto it = _header;
	name.assign(it,++_header);
	if (_header == _headers.end() || ++_header == _headers.end())
		return NIL;

	it = _header;
	_value.assign(it, ++_header);

	if (String::ICompare(_value, "false") == 0) {
		_bool = false;
		return BOOLEAN;
	}
	if (String::ICompare(_value, "true") == 0) {
		_bool = true;
		return BOOLEAN;
	}
	if (String::ICompare(_value,"null")==0)
		return NIL;
	if (_date.fromString(_value))
		return TIME;
	return STRING; // string or number are the same thing in LUA
}


double HTTPHeaderReader::readNumber() {
	Exception ex;
	double value  = String::ToNumber<double>(ex, _value);
	if (ex)
		ERROR("Not a number, ",ex.error());
	return value;
}


} // namespace Mona
