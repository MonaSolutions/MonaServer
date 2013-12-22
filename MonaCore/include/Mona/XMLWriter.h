/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include <stack>

namespace Mona {

class TagPos : public virtual Object {

public:
	TagPos(const std::string val) : name(val), counter(1) {}

	std::string name;
	UInt8		counter;
};

class XMLWriter : public DataWriter, public virtual NullableObject {
public:
	XMLWriter();

	virtual void beginObject(const std::string& type="",bool external=false);
	virtual void endObject();

	virtual void writePropertyName(const std::string& value);

	virtual void beginArray(UInt32 size);
	virtual void endArray() {}

	virtual void writeDate(const Time& date) { writeRaw(date.toString(Time::ISO8601_FRAC_FORMAT, _buffer)); }
	virtual void writeNumber(double value) { writeRaw(String::Format(_buffer, value)); }
	virtual void writeString(const std::string& value) { writeRaw(value); }
	virtual void writeBoolean(bool value) { writeRaw(value? "true" : "false"); }
	virtual void writeNull() { writeRaw("null"); }
	virtual void writeBytes(const UInt8* data, UInt32 size);

	virtual void	clear();

protected:
	void	begin();
	void	writeRaw(const char* value);
	void	writeRaw(const std::string& value);
	void	end();

	std::stack<TagPos>	_queueObjects;
	std::string			_value;
	UInt32				_arraySize;
	bool				_closeLast;	/// if true : it is necessary to close the last tag
	std::string			_buffer;
};



} // namespace Mona
