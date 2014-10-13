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


class JSONWriter : public DataWriter, public virtual Object {
public:
	JSONWriter(const PoolBuffers& poolBuffers);

	UInt64 beginObject(const char* type=NULL);
	void   writePropertyName(const char* value);
	void   endObject();

	UInt64 beginArray(UInt32 size);
	void   endArray();

	void   writeNumber(double value) { start(); String::Append(packet, value); end(); }
	void   writeString(const char* value, UInt32 size);
	void   writeBoolean(bool value) { start(); String::Append(packet,value); end(); }
	void   writeNull() { start(); packet.write("null", 4); end(); }
	UInt64 writeDate(const Date& date);
	UInt64 writeBytes(const UInt8* data,UInt32 size);

	void clear(UInt32 size=0);
	

private:

	/// \brief Add '[' for first data or ',' for next data of an array/object
	/// and update state members
	/// \param isContainer current data is Array or Object
	void start(bool isContainer = false);

	/// \brief Add last ']' if data ended and update state members
	/// \param isContainer current data is Array or Object
	void end(bool isContainer = false);


	bool		_first;
	UInt32		_layers;
};



} // namespace Mona
