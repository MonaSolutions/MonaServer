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

template<class IteratorType>
class MapReader : public DataReader, public virtual Object {
public:

	template<typename MapType>
	MapReader(const MapType& map) : _objectReaden(false),_number(0),_begin(map.begin()),_it(map.begin()),_end(map.end()) {}

	std::string&	readString(std::string& value) { value.assign(_it->second); ++_it; return value; }
	double			readNumber() {++_it; return _number;}
	bool			readBoolean() {++_it; return _number==1;}
	Date&			readDate(Date& date) { ++_it; return date = _date; }
	void			readNull() { ++_it; }
	const UInt8*	readBytes(UInt32& size) { ++_it; return NULL; }

	bool			readObject(std::string& type,bool& external) { return true; }
	bool			readArray(UInt32& size) { return false; }

	Type			readItem(std::string& name) {
		if (_it == _end)
			return END;
		name.assign(_it->first);
		if (String::ICompare(_it->second, "true") == 0) {
			_number = 1;
			return BOOLEAN;
		}
		if (String::ICompare(_it->second, "false") == 0) {
			_number = 0;
			return BOOLEAN;
		}
		if(String::ICompare(_it->second, "null") == 0)
			return NIL;
		if (String::ToNumber(_it->second, _number))
			return NUMBER;
		Exception ex;
		if (_date.update(ex,_it->second))
			return DATE;
		return STRING;
	}
	
	Type			followingType() {
		if (_objectReaden)
			return END;
		_objectReaden = true;
		return OBJECT;
	}

	void			reset() { _it = _begin; _objectReaden=false; }

private:
	Date				_date;
	double				_number;
	IteratorType		_begin;
	IteratorType		_it;
	IteratorType		_end;
	bool				_objectReaden; // to write a object without parameter if there is no map entries
};


} // namespace Mona
