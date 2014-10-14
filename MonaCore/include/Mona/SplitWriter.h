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
#include <set>

namespace Mona {

class SplitWriter : public DataWriter, public virtual Object {
public:

	SplitWriter() {}

	template <typename ...Args>
	SplitWriter(DataWriter& writer,Args&&... args) { addWriter(writer,args ...); }

	template <typename ...Args>
	void addWriter(DataWriter& writer,Args&&... args) {
		_writers.emplace(&writer);
		addWriter(args ...);
	}
	template <typename ...Args>
	void removeWriter(DataWriter& writer,Args&&... args) {
		_writers.erase(&writer);
		removeWriter(args ...);
	}

	UInt64 beginObject(const char* type = NULL)								{ for (DataWriter* pWriter : _writers) pWriter->beginObject(type); return 0; }
	void   endObject()														{ for (DataWriter* pWriter : _writers) pWriter->endObject(); }

	void   writePropertyName(const char* value)								{ for (DataWriter* pWriter : _writers) pWriter->writePropertyName(value); }

	UInt64 beginArray(UInt32 size)											{ for (DataWriter* pWriter : _writers) pWriter->beginArray(size); return 0; }
	void   endArray()														{ for (DataWriter* pWriter : _writers) pWriter->endArray(); }

	void   writeNumber(double value)										{ for (DataWriter* pWriter : _writers) pWriter->writeNumber(value); }
	void   writeString(const char* value, UInt32 size)						{ for (DataWriter* pWriter : _writers) pWriter->writeString(value,size); }
	void   writeBoolean(bool value)											{ for (DataWriter* pWriter : _writers) pWriter->writeBoolean(value); }
	void   writeNull()														{ for (DataWriter* pWriter : _writers) pWriter->writeNull(); }
	UInt64 writeDate(const Date& date)										{ for (DataWriter* pWriter : _writers) pWriter->writeDate(date); return 0; }
	UInt64 writeBytes(const UInt8* data, UInt32 size)						{ for (DataWriter* pWriter : _writers) pWriter->writeBytes(data,size); return 0; }

	UInt64 beginObjectArray(UInt32 size)									{ for (DataWriter* pWriter : _writers) pWriter->beginObjectArray(size); return 0; }

	UInt64 beginMap(Exception& ex, UInt32 size, bool weakKeys = false)		{ for (DataWriter* pWriter : _writers) pWriter->beginMap(ex, size,weakKeys); return 0; }
	void   endMap()															{ for (DataWriter* pWriter : _writers) pWriter->endMap(); }

	void   clear(UInt32 size=0)												{ for (DataWriter* pWriter : _writers) pWriter->clear(size); }

private:
	void addWriter() {}
	void removeWriter() {}

	std::set<DataWriter*> _writers;
};



} // namespace Mona
