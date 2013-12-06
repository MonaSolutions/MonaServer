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


JSONWriter::JSONWriter() {
	
}

void JSONWriter::clear() {

	while (!_queueObjects.empty())
		_queueObjects.pop();

	DataWriter::clear();
}

void JSONWriter::beginObject(const string& type,bool external) {
	manageSeparator(true);

	writer.write8('{');
	if(type.empty())
		return;
	writePropertyName("__type"); // TODO add it in JSONReader?
	writeString(type);
}

void JSONWriter::endObject() {
	if(_queueObjects.empty()) {
		WARN("JSON object already finished")
		return;
	}
	
	writer.write8('}');
	_queueObjects.pop();
}

void JSONWriter::beginArray(UInt32 size) {
	manageSeparator(true);
	
	writer.write8('[');
}

void JSONWriter::endArray() {
	if(_queueObjects.empty()) {
		WARN("JSON array already finished")
		return;
	}
	writer.write8(']');
	_queueObjects.pop();
}


void JSONWriter::writePropertyName(const string& value) {
	writeString(value);
	writer.write8(':');

	// reset cursor for property value
	JSONCursor& cur = _queueObjects.top();
	if (!cur.first)
		cur.first=true;
}

void JSONWriter::writeNumber(double value) {
	manageSeparator();
	string stValue;
	writer.writeRaw(String::Format(stValue, value));
}

void JSONWriter::writeString(const string& value) {
	manageSeparator();
	writer.write8('"');
	writer.writeRaw(value);
	writer.write8('"');
}


void JSONWriter::writeDate(const Time& date) {
	manageSeparator();
	string str;
	writeString(date.toString(Time::ISO8601_FRAC_FORMAT, str));
}


void JSONWriter::writeBytes(const UInt8* data,UInt32 size) {
	manageSeparator();
	writer.writeRaw("{__raw:\"",8);

	Buffer<UInt8> result;
	Util::ToBase64(data, size, result);
	writer.writeRaw(result.data(),result.size());
	writer.writeRaw("\"}",2);
}

void JSONWriter::manageSeparator(bool create /*=false*/) {
	// Append appropriate separator before new element 
	if(!_queueObjects.empty()) {
		JSONCursor& cur = _queueObjects.top();

		if (cur.first)
			cur.first=false;
		else
			writer.write8(',');
	} else {
		writer.write8('[');
		_queueObjects.emplace(false);
	}

	// add object in queue
	if(create)
		_queueObjects.emplace();
}

void JSONWriter::end() {
	if(_queueObjects.size()>1) {
		WARN("Finish JSON complex element before end of JSONWriter")
		return;
	}

	// End of the stream
	if(!_queueObjects.empty()) {
		writer.write8(']');
		_queueObjects.pop();
	}
}


} // namespace Mona
