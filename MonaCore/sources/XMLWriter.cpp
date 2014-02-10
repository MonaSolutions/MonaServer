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

XMLWriter::XMLWriter(const PoolBuffers& buffers) : DataWriter(buffers) {
	
}

void XMLWriter::clear() {
	DataWriter::clear();
	_value.assign("");
	_queueTags.clear();
}

void XMLWriter::beginObject(const string& type,bool external) {

	if (_queueTags.empty())
		return;

	// Write start tag
	TagPos& tag = _queueTags.back();
	packet.write8('<');
	packet.writeRaw(tag.name);
}

void XMLWriter::setTagHasChilds() {

	// New item = this tag has childs
	if (!_queueTags.empty()) {
		TagPos& tag = _queueTags.back();
		if (!tag.childs) {
			tag.childs=true;
			packet.write8('>'); // End current start tag
		}
	}
}

void XMLWriter::writePropertyName(const string& value) {
	
	setTagHasChilds();
	_value.assign(value);
}

void XMLWriter::beginArray(UInt32 size) {

	// Creation of the tag
	if (!_value.empty()) {

		setTagHasChilds();

		_queueTags.emplace_back(_value);
		TagPos& tag = _queueTags.back();
		tag.counter = size;
		_value.assign("");
	} 
	// Empty tag
	else if (!_queueTags.empty() && !size) {

		TagPos& tag = _queueTags.back();

		packet.write8('<');
		packet.writeRaw(tag.name);
		packet.writeRaw("/>");

		// Release the tag if ended
		if(--tag.counter==0)
			_queueTags.pop_back();
	}
}

void XMLWriter::endObject() {

	// Write end of tag
	if (_queueTags.empty())
		return;
	TagPos& tag = _queueTags.back();

	if (tag.childs) {
		// End tag
		packet.writeRaw("</");
		packet.writeRaw(tag.name);
		packet.write8('>');
	} else
		packet.writeRaw("/>"); // Auto-ended tag

	// Release the tag if ended
	if(--tag.counter==0)
		_queueTags.pop_back();
	
}

void XMLWriter::writeBytes(const UInt8* data,UInt32 size) {
	begin();
	packet.writeRaw(data, size);
	end();
}

void XMLWriter::writeRaw(const char* value) { 
	begin();
	packet.writeRaw(value);
	end();
}

void XMLWriter::writeRaw(const string& value) {
	begin();
	packet.writeRaw(value);
	end();
}

void XMLWriter::begin() {

	// Attributes
	if (!_value.empty()) {

		if (_value!="__value") { // (__value is primitive text)
			packet.write8('<');
			packet.writeRaw(_value);
			packet.write8('>');
		}
	} 
	// Primitive Tag value
	else if (!_queueTags.empty()) {
		TagPos& tag = _queueTags.back();

		packet.write8('<');
		packet.writeRaw(tag.name);
		packet.write8('>');
	}
}

void XMLWriter::end() {

	// Attribute
	if (!_value.empty()) {

		if (_value!="__value") { // (__value is primitive text)
			packet.writeRaw("</");
			packet.writeRaw(_value);
			packet.write8('>');
		}
		_value.assign("");
	}
	// Primitive Tag value
	else if (!_queueTags.empty()) {

		// Write end tag
		TagPos& tag = _queueTags.back();
		packet.writeRaw("</");
		packet.writeRaw(tag.name);
		packet.write8('>');

		// Release the tag if ended
		if(--tag.counter==0)
			_queueTags.pop_back();
	}
}


} // namespace Mona
