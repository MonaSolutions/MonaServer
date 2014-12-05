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
#include "Mona/Parameters.h"
#include "Mona/DataWriter.h"
#include "Mona/DataReader.h"

namespace Mona {


class ParameterWriter : public DataWriter, public virtual Object {
public:

	ParameterWriter(Parameters& parameters) : _index(0),_parameters(parameters),_isProperty(false) {}
	UInt64 beginObject(const char* type = NULL) { return 0; }
	void   endObject() {}

	void writePropertyName(const char* value) { _property.assign(value); _isProperty=true; }

	UInt64 beginArray(UInt32 size) { return 0; }
	void   endArray(){}

	void writeNumber(double value) { std::string buffer;  set(String::Format(buffer, value)); }
	void writeString(const char* value, UInt32 size) { set(value, size); }
	void writeBoolean(bool value) { set(value ? "true" : "false");}
	void writeNull() { set("null",4); }
	UInt64 writeDate(const Date& date) { writeNumber((double)date); return 0; }
	UInt64 writeBytes(const UInt8* data, UInt32 size) { set(STR data, size); return 0; }

	void   clear(UInt32 size=0) { _index = 0; _isProperty = false; _property.clear(); _parameters.clear(); }
private:
	
	template <typename ...Args>
	void set(Args&&... args) {
		if (!_isProperty)
			String::Format(_property, _index++);
		_parameters.setString(_property,args ...);
		_isProperty = false;
		_property.clear();
	}

	std::string					_property;
	bool						_isProperty;
	UInt32						_index;

	Parameters&					_parameters;
};



} // namespace Mona
