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


class RawWriter : public DataWriter, virtual Object {
public:

	RawWriter(const PoolBuffers& buffers) : DataWriter(buffers) {}

	void beginObject(const std::string& type = "", bool external = false) {}
	void endObject() {}

	void writePropertyName(const std::string& value) { writeString(value);  }

	void beginArray(UInt32 size) {}
	void endArray(){}

	void writeDate(const Time& date) { writeString(date.toString(Time::SORTABLE_FORMAT,_buffer)); }
	void writeNumber(double value) { String::Format(_buffer, value); writeString(_buffer); }
	void writeString(const std::string& value) { packet.writeRaw(value); }
	void writeBoolean(bool value) { packet.writeRaw( value ? "true" : "false"); }
	void writeNull() { packet.writeRaw("null"); }
	void writeBytes(const UInt8* data, UInt32 size) { packet.writeRaw(data, size); }

private:
	std::string _buffer;
};



} // namespace Mona
