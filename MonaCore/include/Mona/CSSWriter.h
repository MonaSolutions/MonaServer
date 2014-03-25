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


class CSSWriter : public DataWriter, public virtual Object {
public:
	CSSWriter(const PoolBuffers& buffers) : DataWriter(buffers) {}

	void beginObject(const std::string& type = "", bool external = false) {}
	void endObject() {}

	void writePropertyName(const std::string& value) {}

	void beginArray(UInt32 size) {}
	void endArray() {}

	void writeDate(const Date& date) {}
	void writeNumber(double value) {}
	void writeString(const std::string& value) {}
	void writeBoolean(bool value) {}
	void writeNull() {}
	void writeBytes(const UInt8* data,UInt32 size) {}

	void	clear() {}
};



} // namespace Mona
