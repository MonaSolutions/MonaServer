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
#include "Mona/Util.h"

namespace Mona {


class QueryWriter : public DataWriter, public virtual Object {
public:
	QueryWriter(const PoolBuffers& poolBuffers) : DataWriter(poolBuffers), _first(true), _isProperty(false) {}

	const char* query() const;

	UInt64 beginObject(const char* type = NULL) { return 0; }
	void   writePropertyName(const char* value);
	void   endObject() {}

	UInt64 beginArray(UInt32 size) { return 0; }
	void   endArray() {}

	void   writeNumber(double value) { String::Append(writer(), value); }
	void   writeString(const char* value, UInt32 size) { Util::EncodeURI(value,size, writer()); }
	void   writeBoolean(bool value) { writer().write(value ? "true" : "false"); }
	void   writeNull() { writer().write("null",4); }
	UInt64 writeDate(const Date& date) { std::string buffer; writer().write(date.toString(Date::ISO8601_SHORT_FORMAT,buffer)); return 0; }
	UInt64 writeBytes(const UInt8* data, UInt32 size) { Util::ToBase64(data, size, writer()); return 0; }

	void clear(UInt32 size=0) { _isProperty = false; _first = true;  DataWriter::clear(size); }
	
private:
	PacketWriter& writer();

	bool _isProperty;
	bool _first;
};



} // namespace Mona
