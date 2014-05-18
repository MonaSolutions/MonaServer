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


namespace Mona {

template<class ArrayType>
class ArrayReader : public DataReader, public virtual Object {
public:

	ArrayReader(const ArrayType& array) : _number(0),_begin(array.begin()),_it(array.begin()),_end(array.end()) {}

	std::string&	readString(std::string& value) { value.assign(*_it); ++_it; return value; }
	double			readNumber() {++_it; return _number;}
	bool			readBoolean() {++_it; return _number==1;}
	Date&			readDate(Date& date) { ++_it; return date = _date; }
	void			readNull() { ++_it; }
	const UInt8*	readBytes(UInt32& size) { ++_it; return NULL; }

	bool			readObject(std::string& type,bool& external) { return false; }
	bool			readArray(UInt32& size) { return false; }
	Type			readItem(std::string& name) { return END; }
	
	Type			followingType() {
		if (_it == _end)
			return END;
		if (String::ICompare(*_it, "true") == 0) {
			_number = 1;
			return BOOLEAN;
		}
		if (String::ICompare(*_it, "false") == 0) {
			_number = 0;
			return BOOLEAN;
		}
		if(String::ICompare(*_it, "null") == 0)
			return NIL;
		if (String::ToNumber(*_it, _number))
			return NUMBER;
		Exception ex;
		if (_date.update(ex,*_it))
			return DATE;
		return STRING;
	}

	void			reset() { _it = _begin; }

private:
	Date								_date;
	double								_number;
	typename ArrayType::const_iterator	_begin;
	typename ArrayType::const_iterator	_it;
	typename ArrayType::const_iterator	_end;
};


} // namespace Mona
