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
#include "math.h"
#include <stack>

namespace Mona {

class JSONCursor : public virtual Object {

public:
	JSONCursor(bool started = true) : first(started) {}

	bool		first;
};

class JSONWriter : public DataWriter, virtual Object {
public:
	JSONWriter();

	virtual void beginObject(const std::string& type="",bool external=false);
	virtual void endObject();

	virtual void writePropertyName(const std::string& value);

	virtual void beginObjectArray(UInt32 size) { beginArray(size); beginObject(); }
	virtual void beginArray(UInt32 size);
	virtual void endArray();

	virtual void writeDate(const Time& date);
	virtual void writeNumber(double value);
	virtual void writeString(const std::string& value);
	virtual void writeBoolean(bool value) { writeString(value ? "true" : "false"); }
	virtual void writeNull() { writeString("null"); }
	virtual void writeBytes(const UInt8* data,UInt32 size);
	virtual void clear();

	void	manageSeparator(bool create = false);
			///\brief Add separator/Create object if needed

	void	end();
			///\brief Finish the stream
private:

	std::stack<JSONCursor>	_queueObjects;
};



} // namespace Mona
