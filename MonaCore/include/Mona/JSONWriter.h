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
#include "Poco/Format.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "math.h"
#include <list>

namespace Mona {


class JSONWriter : public DataWriter {
public:
	JSONWriter();
	virtual ~JSONWriter();

	void beginObject(const std::string& type="",bool external=false);
	void endObject();

	void writePropertyName(const std::string& value);

	void beginArray(Mona::UInt32 size);
	void endArray();

	void writeDate(const Mona::Time& date);
	void writeNumber(double value);
	void writeString(const std::string& value);
	void writeBoolean(bool value);
	void writeNull();
	void writeBytes(const Mona::UInt8* data,Mona::UInt32 size);

	void	end();
	void	clear();
private:

	bool			_started;
	bool			_first;
	Mona::UInt32	_layers;
};

inline void JSONWriter::writeDate(const Mona::Time& date) {
	std::string str;
	date.toString(str, Time::ISO8601_FRAC_FORMAT);
	writeString(str);
}

inline void JSONWriter::writeNumber(double value) {
	writeString(ROUND(value)==value ? Poco::format("%u",(Mona::UInt32)value) : Poco::format("%f",value));
}

inline void JSONWriter::writeBoolean(bool value) {
	writeString(value ? "true" : "false");
}

inline void JSONWriter::writeNull() {
	writeString("null");
}



} // namespace Mona
