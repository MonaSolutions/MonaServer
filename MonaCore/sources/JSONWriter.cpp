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

#include "Mona/JSONWriter.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {


JSONWriter::JSONWriter(const PoolBuffers& poolBuffers) : DataWriter(poolBuffers),_first(true),_layers(0) {
	packet.write("[]");
}

void JSONWriter::clear(UInt32 size) {
	_first=true;
	_layers=0;
	DataWriter::clear(size);
	packet.write("[]");
}

UInt64 JSONWriter::beginObject(const char* type) {
	start(true);

	packet.write8('{');
	if(!type)
		return 0;
	writePropertyName("__type");
	writeString(type,strlen(type));
	return 0;
}

void JSONWriter::endObject() {

	packet.write8('}');

	end(true);
}

UInt64 JSONWriter::beginArray(UInt32 size) {
	start(true);

	packet.write8('[');
	return 0;
}

void JSONWriter::endArray() {

	packet.write8(']');

	end(true);
}


void JSONWriter::writePropertyName(const char* value) {
	writeString(value,strlen(value));
	packet.write8(':');
	_first=true;
}

void JSONWriter::writeString(const char* value, UInt32 size) {

	start();
	packet.write8('"');
	packet.write(value,size);
	packet.write8('"');
	end();
}

UInt64 JSONWriter::writeDate(const Date& date) {
	string buffer;
	date.toString(Date::ISO8601_FRAC_FORMAT,buffer); 
	writeString(buffer.c_str(),buffer.size());
	return 0;
}



UInt64 JSONWriter::writeBytes(const UInt8* data,UInt32 size) {
	start();

	packet.write(EXPAND("{\"__raw\":\""));
	Util::ToBase64(data, size, packet,true);
	packet.write("\"}");

	end();
	return 0;
}

void JSONWriter::start(bool isContainer) {

	// remove the last ']'
	if (_layers == 0)
		packet.clear(packet.size() - 1);


	if(!_first)
		packet.write8(',');

	if (isContainer) {
		_first=true;
		++_layers;
	}
}

void JSONWriter::end(bool isContainer) {

	if (isContainer) {
		if (_layers==0) {
			ERROR("JSON container already closed")
			return;
		}
		--_layers;
	}
	
	if(_first)
		_first=false;

	if (_layers==0)
		packet.write8(']');
}


} // namespace Mona
