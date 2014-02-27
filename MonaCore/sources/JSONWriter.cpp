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


JSONWriter::JSONWriter(const PoolBuffers& buffers,bool modeRaw) : DataWriter(buffers),_modeRaw(modeRaw),_first(true),_started(false),_layers(0) {
	
}

void JSONWriter::clear() {
	_first=true;
	_started=false;
	_layers=0;
	DataWriter::clear();
}

void JSONWriter::beginObject(const string& type,bool external) {
	startData(true);

	packet.write8('{');
	if(type.empty())
		return;
	writePropertyName("__type"); // TODO add it in JSONReader?
	writeString(type);
}

void JSONWriter::endObject() {
	if(_layers==0) {
		WARN("JSON object already finished")
		return;
	}
	packet.write8('}');

	endData(true);
}

void JSONWriter::beginArray(UInt32 size) {
	startData(true);

	packet.write8('[');
}

void JSONWriter::endArray() {
	if(_layers==0) {
		WARN("JSON array already finished")
		return;
	}
	packet.write8(']');

	endData(true);
}


void JSONWriter::writePropertyName(const string& value) {
	writeString(value);
	packet.write8(':');
	_first=true;
}

void JSONWriter::writeString(const string& value) {
	if (_modeRaw) {
		writeRaw(value);
		_modeRaw = false;
		return;
	}

	startData();

	packet.write8('"');
	packet.writeRaw(value);
	packet.write8('"');

	endData();
}

void JSONWriter::writeBytes(const UInt8* data,UInt32 size) {
	startData();

	packet.writeRaw("{__raw:\"");
	Util::ToBase64(data, size, _buffer);
	packet.writeRaw((const UInt8*)_buffer.data(),_buffer.size());
	packet.writeRaw("\"}");

	endData();
}

void JSONWriter::startData(bool isContainer) {

	// Write first ']'
	if(!_started) {
		_started=true;
		packet.write8('[');
	}

	if(!_first)
		packet.write8(',');

	if (isContainer) {
		_first=true;
		++_layers;
	}
}

void JSONWriter::endData(bool isContainer) {

	if (isContainer)
		--_layers;
	
	if(_first)
		_first=false;
}

void JSONWriter::endWrite() {

	// Write last ']'
	if (_started)
		packet.write8(']');
}

} // namespace Mona
