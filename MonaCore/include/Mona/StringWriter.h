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


class StringWriter : public DataWriter, public virtual Object {
public:

	StringWriter(const PoolBuffers& poolBuffers) : _pString(NULL),DataWriter(poolBuffers) {}
	StringWriter(std::string& buffer) : _pString(&buffer) {}

	UInt64 beginObject(const char* type = NULL) { return 0; }
	void   endObject() {}

	void   writePropertyName(const char* name) { append(name);  }

	UInt64 beginArray(UInt32 size) { return 0; }
	void   endArray(){}

	void   writeNumber(double value) { append(value); }
	void   writeString(const char* value, UInt32 size) { append(value,size); }
	void   writeBoolean(bool value) { append( value ? "true" : "false"); }
	void   writeNull() { packet.write("null",4); }
	UInt64 writeDate(const Date& date) { std::string buffer; append(date.toString(Date::SORTABLE_FORMAT, buffer)); return 0; }
	UInt64 writeBytes(const UInt8* data, UInt32 size) { append(data, size); return 0; }

	void   clear(UInt32 size = 0) { if (_pString) _pString->erase(size); else packet.clear(size); }
private:
	void append(const void* value, UInt32 size) {
		if (_pString)
			_pString->append(STR value, size);
		else
			packet.write(value, size);
	}

	template<typename ValueType>
	void append(const ValueType& value) {
		if (_pString)
			String::Append(*_pString, value);
		else
			String::Append(packet, value);
	}

	std::string* _pString;

};



} // namespace Mona
