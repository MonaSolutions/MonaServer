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
#include "math.h"

namespace Mona {


class HTTPPacketWriter : public DataWriter {
public:
	HTTPPacketWriter();
	virtual ~HTTPPacketWriter();

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
};


inline void HTTPPacketWriter::writeDate(const Mona::Time& date) {
	std::string str;
	date.toString(str, Time::HTTP_FORMAT);
	writeString(str);
}

inline void HTTPPacketWriter::writeNumber(double value) {
	writeString(ROUND(value)==value ? Poco::format("%u",(Mona::UInt32)value) : Poco::format("%f",value));
}

inline void HTTPPacketWriter::writeNull() {
	writer.writeRaw("null\r\n",6);
}
inline void HTTPPacketWriter::writeBytes(const Mona::UInt8* data,Mona::UInt32 size) {
	writer.writeRaw(data,size);
}


inline void HTTPPacketWriter::beginObject(const std::string& type,bool external) {}
inline void HTTPPacketWriter::endObject() {writer.writeRaw("\r\n",2);}
inline void HTTPPacketWriter::beginArray(Mona::UInt32 size) {}
inline void HTTPPacketWriter::endArray() {}


} // namespace Mona
