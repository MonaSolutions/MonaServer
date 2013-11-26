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

#include "Mona/XMLWriter.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {

XMLWriter::XMLWriter() : _arraySize(1), _closeLast(false) {
	
}

void XMLWriter::clear() {
	DataWriter::clear();
	_arraySize = 1;
	_value.assign("");
	while (!_queueObjects.empty())
		_queueObjects.pop();
}

void XMLWriter::beginObject(const string& type,bool external) {
	
	// Creation of the tag
	if (!_value.empty()) {

		_queueObjects.emplace(_value);
		TagPos& tag = _queueObjects.top();
		tag.counter = _arraySize;
		_arraySize = 1;
		_value.assign("");
	}

	if (_queueObjects.empty())
		return;

	if (_closeLast) {
		writer.write8('>');
		_closeLast=false;
	}

	TagPos& tag = _queueObjects.top();
	writer.write8('<');
	writer.writeRaw(tag.name);
	_closeLast=true;
}

void XMLWriter::writePropertyName(const string& value) {

	_value.assign(value);
}

void XMLWriter::beginArray(UInt32 size) {

	_arraySize=size;
}

void XMLWriter::endObject() {
		
	// Write end of tag
	if (_queueObjects.empty())
		return;
	TagPos& tag = _queueObjects.top();

	// Auto-ended tag
	if (_closeLast) {

		writer.writeRaw("/>");
		_closeLast=false;
	} else {

		writer.writeRaw("</");
		writer.writeRaw(tag.name);
		writer.write8('>');
	}

	// Release the tag if ended
	if(--tag.counter==0)
		_queueObjects.pop();
	
}

void XMLWriter::writeDate(const Time& date) {
	string str;
	writePrimitive(date.toString(Time::ISO8601_FRAC_FORMAT, str));
}

void XMLWriter::writeBoolean(bool value) { 
	writePrimitive(value? "true" : "false");
}

void XMLWriter::writeNumber(double value) {

	string str;
	writePrimitive(String::Format(str, value));
}

void XMLWriter::writeString(const string& value) { 
		
	writePrimitive(value);
}

void XMLWriter::writeNull() {
	writeString("null");
}

void XMLWriter::writeBytes(const UInt8* data,UInt32 size) {

	writePrimitive("", data, size);
}

void XMLWriter::writePrimitive(const string& value, const UInt8* data /*= NULL*/, UInt32 size /*= 0*/) {

	if (!_value.empty()) {
		
		if (_value=="__value")
			_value="value";

		if (_closeLast) {
			writer.write8('>');
			_closeLast=false;
		}

		writer.write8('<');
		writer.writeRaw(_value);
		writer.write8('>');
	}

	// Write value
	if (data)
		writer.writeRaw(data,size);
	else
		writer.writeRaw(value);

	if (!_value.empty()) {
		
		writer.writeRaw("</");
		writer.writeRaw(_value);
		writer.write8('>');

		_value.assign("");
	}
}

} // namespace Mona
