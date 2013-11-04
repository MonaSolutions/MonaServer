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

#include "Mona/JSONWriter.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {


JSONWriter::JSONWriter() : _first(true),_started(false),_layers(0) {
	
}

void JSONWriter::clear() {
	_first=true;
	_started=false;
	_layers=0;
	DataWriter::clear();
}

void JSONWriter::beginObject(const string& type,bool external) {
	if(!_started) {
		_started=true;
		writer.write8('[');
	}	
	if(!_first)
		writer.write8(',');
	_first=true;
	++_layers;
	writer.write8('{');
	if(type.empty())
		return;
	writePropertyName("__type");
	writeString(type);
}

void JSONWriter::endObject() {
	if(_layers==0) {
		WARN("JSON object already finished")
		return;
	}
	writer.write8('}');
	--_layers;
}

void JSONWriter::beginArray(UInt32 size) {
	if(!_started) {
		_started=true;
		writer.write8('[');
	}	
	if(!_first)
		writer.write8(',');
	_first=true;
	++_layers;
	writer.write8('[');
}

void JSONWriter::endArray() {
	if(_layers==0) {
		WARN("JSON array already finished")
		return;
	}
	writer.write8(']');
	--_layers;
}


void JSONWriter::writePropertyName(const string& value) {
	writeString(value);
	writer.write8(':');
	_first=true;
}

void JSONWriter::writeString(const string& value) {
	if(!_started) {
		_started=true;
		writer.write8('[');
	}
	if(!_first)
		writer.write8(',');
	_first=false;
	writer.write8('"');
	writer.writeRaw(value);
	writer.write8('"');
}


void JSONWriter::writeDate(const Time& date) {
	string str;
	writeString(date.toString(Time::ISO8601_FRAC_FORMAT, str));
}


void JSONWriter::writeBytes(const UInt8* data,UInt32 size) {
	if(!_started) {
		_started=true;
		writer.write8('[');
	}
	if(!_first)
		writer.write8(',');
	_first=false;
	writer.writeRaw("{__raw:\"",8);

	Buffer<UInt8> result;
	Util::ToBase64(data, size, result);
	writer.writeRaw(result.data(),result.size());
	writer.writeRaw("\"}",2);
}

void JSONWriter::end() {
	if(_layers>0) {
		WARN("Finish JSON complex element before to end JSONWriter")
		return;
	}
	if(_started)
		writer.write8(']');
}


} // namespace Mona
