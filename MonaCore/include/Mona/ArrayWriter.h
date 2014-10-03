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
#include "Mona/DataWriter.h"

namespace Mona {

template<class ArrayType>
class ArrayWriter : public DataWriter, public virtual Object {
public:
	ArrayWriter(ArrayType& array) : _array(array),_size(0) {}

	UInt64 beginObject(const char* type = NULL) { return 0; }
	void   writePropertyName(const char* value) {}
	void   endObject() {}

	UInt64 beginArray(UInt32 size) { return 0; }
	void   endArray(){}

	void   writeNumber(double value) { _array.emplace_back();  _size += String::Format(_array.back(), value).size(); }
	void   writeString(const char* value, UInt32 size) { _array.emplace_back(value, size);  _size += size; }
	void   writeBoolean(bool value) { _array.emplace_back(); _size += String::Format(_array.back(), value).size();}
	void   writeNull() { _array.emplace_back(EXPAND("null"));  _size += 4; }
	UInt64 writeDate(const Date& date) { writeNumber((double)date); return 0; }
	UInt64 writeBytes(const UInt8* data, UInt32 size) { _array.emplace_back(STR data, size); _size += size; return 0; }

	UInt32 size() const { return _size; }
	UInt32 count() const { return _array.count(); }
	
	void   clear() { _array.clear(); _size = 0; DataWriter::clear(); }

private:

	ArrayType&					_array;
	UInt32						_size;
};



} // namespace Mona
