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
	ArrayWriter(ArrayType& array) : _array(array),_size(0),_isProperty(false) {}

	void beginObject(const std::string& type = "", bool external = false) {}
	void endObject() {}

	void writePropertyName(const std::string& value) {}

	void beginArray(UInt32 size) {}
	void endArray(){}

	void writeDate(const Date& date) { set(String::Format(_buffer, date)); }
	void writeNumber(double value) { set(String::Format(_buffer, value)); }
	void writeString(const std::string& value) { set(value); }
	void writeBoolean(bool value) { set( value ? "true" : "false");}
	void writeNull() { set("null"); }
	void writeBytes(const UInt8* data, UInt32 size) { set((const char*)data, size); }

	UInt32 size() const { return _size; }
	UInt32 count() const { return _array.count(); }
	

	void   clear() { _array.clear(); _size = 0; DataWriter::clear(); }
private:
	template <typename ...Args>
	void set(Args&&... args) {
		_array.emplace_back(args ...);
		_size += _array.back().size();
	}

	std::string					_property;
	bool						_isProperty;

	ArrayType&					_array;
	std::string					_buffer;
	UInt32						_size;
};



} // namespace Mona
