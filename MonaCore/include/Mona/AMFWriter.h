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
#include "Mona/AMF.h"
#include "Mona/DataWriter.h"
#include <deque>
#include <map>

namespace Mona {

class ObjectRef;

class AMFWriter : public DataWriter, virtual NullableObject {
public:
	AMFWriter(const PoolBuffers& poolBuffers);
	virtual ~AMFWriter();

	bool repeat(UInt32 reference);
	void clear();

	void beginObject(const std::string& type="",bool external=false);
	void endObject();

	void beginMap(UInt32 size,bool weakKeys=false);
	void endMap() { endObject(); }

	void writePropertyName(const std::string& value);

	void beginArray(UInt32 size) { beginObjectArray(size); endObject(); }
	void beginObjectArray(UInt32 size);
	void endArray() { endObject(); }

	void writeDate(const Time& date);
	void writeNumber(double value);
	void writeString(const std::string& value);
	void writeBoolean(bool value);
	void writeNull();
	void writeBytes(const UInt8* data,UInt32 size);

	bool				amf0Preference;

	static AMFWriter    Null;

private:
	AMFWriter() : _amf3(false), amf0Preference(false) {} // null version

	void writeInteger(Int32 value);
	void writeText(const std::string& value);

	std::map<std::string,UInt32>	_stringReferences;
	std::vector<UInt8>				_references;
	bool							_amf3;
	std::deque<ObjectRef*>			_lastObjectReferences;
};



} // namespace Mona
