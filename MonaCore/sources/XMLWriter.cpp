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

#include "Mona/XMLWriter.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {

XMLWriter::XMLWriter(const PoolBuffers& buffers) : DataWriter(buffers), _started(false), _close2Times(false) {
	
}

void XMLWriter::clear() {
	DataWriter::clear();
	_tagName.assign("");
	_queueTags.clear();
	_started=false;
	_close2Times=false;
}

void XMLWriter::beginObject(const string& type,bool external) {

	if(!_started) {
		beginDocument(true);
		_started=true;
	}

	// Initiate tag
	if (!_queueTags.empty()) {  // Array Intern
		TagPos& tag = _queueTags.back();

		if (tag.name!="__array" && tag.arrayLevel==1) {
			packet.write8('<');
			packet.writeRaw(tag.name);
			tag.childs=false; // reset childs flag
		}
	}
}

void XMLWriter::writePropertyName(const string& value) {

	setTagHasChilds();
	_tagName.assign(value);
}

void XMLWriter::beginArray(UInt32 size) {

	if(!_started) {
		beginDocument();
		_started=true;
	}

	// Creation of the tag _tagName
	if (!_tagName.empty()) {
		setTagHasChilds();
		_queueTags.emplace_back(_tagName);

		TagPos& tag = _queueTags.back();
		if(size==0) {
			packet.write8('<');
			packet.writeRaw(_tagName);
			packet.writeRaw("/>");
			_queueTags.pop_back();
		}
		else
			tag.childs=false; // reset childs flag
		_tagName.assign("");
	}
	// Array
	else {
		if (!_queueTags.empty()) {  // Array Intern
			TagPos& tag = _queueTags.back();

			// Tag Empty
			if(size==0) {

				packet.write8('<');
				packet.writeRaw(tag.name);
				packet.writeRaw("/>");
				tag.empty=true;
			} 
			// Array in Tag
			else if (tag.arrayLevel==1) { // First is an array attribute

				packet.write8('<');
				packet.writeRaw(tag.name);
				packet.write8('>');
			}
			else // next are just arrays
				packet.writeRaw("<__array>");
			tag.arrayLevel++;
		} else {

			_queueTags.emplace_back("__array");
			packet.writeRaw("<__array>");
		}
	}
}

void XMLWriter::endObject() {

	if (_queueTags.empty())
		return;

	// End of Array
	TagPos& tag = _queueTags.back();
	if (tag.arrayLevel==1 && tag.name!="__array") {
		packet.writeRaw("</");
		packet.writeRaw(tag.name);
		packet.write8('>');
	}
}

void XMLWriter::endArray() {

	if (_queueTags.empty())
		return;

	// End of Array
	TagPos& tag = _queueTags.back();
	
	switch(tag.arrayLevel) {
	case 1: // End of tag
		if (tag.name=="__array")
			packet.writeRaw("</__array>");
		_queueTags.pop_back(); // delete Tag
		break;
	case 2: // End of an array attribute
		if (!tag.empty) {
			packet.writeRaw("</");
			packet.writeRaw(tag.name);
			packet.write8('>');
		}
		tag.arrayLevel--;
		break;
	default: // End of an array encapsulated
		packet.writeRaw("</__array>");
		tag.arrayLevel--;
	}
}

void XMLWriter::writeBytes(const UInt8* data,UInt32 size) {

	if (size == 0)
		begin(true);
	else {
		begin();
		packet.writeRaw(data, size);
		end();
	}
}

void XMLWriter::writeRaw(const char* value) { 

	if (!strlen(value))
		begin(true);
	else {
		begin();
		packet.writeRaw(value);
		end();
	}
}

void XMLWriter::writeRaw(const string& value) {

	if (value.empty())
		begin(true);
	else {
		begin();
		packet.writeRaw(value);
		end();
	}
}

void XMLWriter::begin(bool empty /*= false*/) {

	if(!_started) {
		beginDocument();
		_started=true;
	}

	// Primitive Tag value
	if (_tagName.empty()) {

		if (!_queueTags.empty()) {

			TagPos& tag = _queueTags.back();
			// Primitive tag
			if (tag.name != "__array" && (tag.arrayLevel==1)) {
				packet.write8('<');
				packet.writeRaw(tag.name);
			}
			else
				packet.writeRaw("<__noname");
		} else 
			packet.writeRaw("<__noname");
	} 
	// Attribute without '[]'
	else {
		packet.write8('<');
		packet.writeRaw(_tagName);
	}

	if (empty)
		packet.writeRaw("/>");
	else
		packet.write8('>');
}

void XMLWriter::end() {

	// Attribute
	if (_tagName.empty()) {

		if (!_queueTags.empty()) {

			TagPos& tag = _queueTags.back();
			// Primitive tag
			if (tag.name != "__array" && (tag.arrayLevel==1)) {
				packet.writeRaw("</");
				packet.writeRaw(tag.name);
				packet.write8('>');
				return;
			}
		}
		// No Tag => tag with no name
		packet.writeRaw("</__noname>");
	}
	else { // Attribute without '[]'
		packet.writeRaw("</");
		packet.writeRaw(_tagName);
		packet.write8('>');
		_tagName.assign("");
	}
}

void	XMLWriter::endWrite() {

	if (_started) {
		packet.writeRaw("</__array>");

		if (_close2Times)
			packet.writeRaw("</__array>");
	}
}

void	XMLWriter::beginDocument(bool doubleArray) {

	packet.writeRaw("<__array>");

	if (doubleArray) {
		// First element is a Tag => double array encapsulation
		_close2Times=true;
		packet.writeRaw("<__array>");
	}
}

void XMLWriter::setTagHasChilds() {

	// New item = this tag has childs
	if (!_queueTags.empty()) {
		TagPos& tag = _queueTags.back();
		if (!tag.childs && tag.name != "__array" && tag.arrayLevel==1) {
			tag.childs=true;
			packet.write8('>'); // Close current start tag
		}
	}
}

} // namespace Mona
