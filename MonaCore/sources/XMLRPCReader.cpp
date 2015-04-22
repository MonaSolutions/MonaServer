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

#include "Mona/XMLRPCReader.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"
#include <sstream>

using namespace std;

namespace Mona {


XMLRPCReader::XMLRPCReader(PacketReader& packet,const PoolBuffers& poolBuffers) : _pBuffer(poolBuffers), _isResponse(false),_countingLevel(0),_nextType(END), XMLParser((const char*)packet.current(),packet.available(),poolBuffers), DataReader(packet),_validating(3),_first(true) {
	// validating!
	Exception ex; // ignore exception here
	XMLParser::parse(ex);
	if (!_validating && ex)
		_validating = 3;
	else if (isValid())
		save(_state);
}

void XMLRPCReader::reset() {
	if (!isValid())
		return;
	XMLParser::reset(_state);
	_first = !_isResponse;
	_nextType = END;
	_xmls.clear();
	_xmls.emplace_back(PARAMS);
	_countingLevel = 0;
}

bool XMLRPCReader::onStartXMLElement(const char* name, Parameters& attributes) {

	_data = NULL;
	_size = 0;

	if (_validating) {
		switch (_validating) {
			case 3:
				if (String::ICompare("methodResponse", name) == 0) {
					_validating -= 2;
					_isResponse = true;
					_first = false;
					return true;
				}
				if (String::ICompare("methodCall", name) != 0)
					break;
				--_validating;
				return true;
			case 2:
				if (String::ICompare("methodName", name) != 0)
					break;
				--_validating;
				return true;
			case 1:
				if (String::ICompare("params", name) != 0)
					break;
				--_validating;
				_xmls.emplace_back(PARAMS);
				// OK!
		}
		return false; // end of validating step
	}

	if (_xmls.empty()) {
		_nextType = END;
		return false;
	}

	XMLName xml(_xmls.back());

	if (xml == VALUE) {

		if (String::ICompare(name, "array") == 0) {
			_nextType = ARRAY;
			_xmls.emplace_back(ARRAY);
			if (!attributes.getString("size", _attribute))
				_attribute.clear();
			return _countingLevel ? true : false; // stop if not counting
		}

		if (String::ICompare(name, "struct") == 0) {
			_xmls.emplace_back(STRUCT);
			_nextType = attributes.getString("type", _attribute) ? OBJECT_TYPED : OBJECT;
			return _countingLevel ? true : false; // stop
		}
	
		// primitive type
		_xmls.emplace_back(TYPE);

		if (String::ICompare(name, "i4") == 0 || String::ICompare(name, "int") == 0 || String::ICompare(name, "double") == 0)
			_nextType = NUMBER;
		else if (String::ICompare(name, "boolean") == 0)
			_nextType = BOOLEAN;
		else if (String::ICompare(name, "string") == 0)
			_nextType = STRING;
		else if (String::ICompare(name, "dateTime.iso8601") == 0)
			_nextType = DATE;
		else if (String::ICompare(name, "base64") == 0)
			_nextType = BYTES;
		else {
			_xmls.back() = UNKNOWN;
			ERROR("XML-RPC type ", name, " unknown");
		}

		return true;// continue the parse
	}



	XMLName nextXML(UNKNOWN);

	switch (xml) {
		case PARAMS:
			if (String::ICompare(name, "param") == 0)
				nextXML = PARAM;
			else
				ERROR("XML-RPC element ", name, " ignored, invalid after 'params'");
			break;
		case PARAM:
			if (String::ICompare(name, "value") == 0) {
				nextXML = VALUE;
				_nextType = NIL;
			} else
				ERROR("XML-RPC element ", name, " ignored, invalid after 'param'");
			break;
		case ARRAY:
			if (String::ICompare(name, "data") == 0)
				nextXML = DATA;
			else
				ERROR("XML-RPC element ", name, " ignored, invalid after 'array'");
			break;
		case DATA:
			if (String::ICompare(name, "value") == 0)
				nextXML = VALUE;
			else
				ERROR("XML-RPC element ", name, " ignored, invalid after 'data'");
			break;
		case STRUCT:
			if (String::ICompare(name, "member") == 0)
				nextXML = MEMBER;
			else
				ERROR("XML-RPC element ", name, " ignored, invalid after 'struct'");
			break;
		case MEMBER:
			if (String::ICompare(name, "name") == 0)
				nextXML = NAME;
			else if (String::ICompare(name, "value") == 0)
				nextXML = VALUE;
			else
				ERROR("XML-RPC element ", name, " ignored, invalid after 'member'");
			break;
		case NAME:
			ERROR("XML-RPC element ", name, " ignored, invalid in a member name element");
			break;
		case TYPE:
			ERROR("XML-RPC element ", name, " ignored, invalid in a primivite type");
			break;
		case UNKNOWN:
		case VALUE:
			break;
	}

	_xmls.emplace_back(nextXML);
	return true; // continue the parse
}

bool XMLRPCReader::onInnerXMLElement(const char* name, const char* data, UInt32 size) {
	if (_validating == 1 && !_isResponse) {
		_method.assign(data,size);
		return true;
	}
	_data = data;
	_size = size;
	return true;
}

bool XMLRPCReader::onEndXMLElement(const char* name) {
	if (_validating)
		return true; // continuing the parse

	if (_xmls.empty()) {
		 // end of document
		_nextType = END;
		return false;
	}
	XMLName xml(_xmls.back());
	_xmls.pop_back();

	if (_countingLevel) {
		// just here to count array element
		if ( _xmls.size() > _countingLevel)
			return true;
		if (_xmls.size() < _countingLevel)
			_nextType = END;
		else
			_nextType = ARRAY;
		return false; // stop the parse
	}
	
	if (xml == TYPE)
		return false; // stop the parse to get a valid _data + _size
	if (xml == NAME) {
		_nextType = NAME;
		return false; // stop the parse to get a valid _data + _size
	}
	if (xml==VALUE && _nextType==NIL)
		return false; // stop the parse to get a NIL value

	if (xml == STRUCT)
		return false; // stop object writing

	return true;
}

UInt8 XMLRPCReader::parse() {
	_nextType = END;
	Exception ex;
	XMLParser::parse(ex);
	if (!ex)
		return _nextType;
	ERROR("XML parsing error, ",ex.error())
	_validating = 3;
	return END;
}


UInt8 XMLRPCReader::followingType() {
	if (!isValid())
		return END;

	if (_first)
		return STRING; // _method!

	// parse one more step
	return parse();
}

bool XMLRPCReader::readOne(UInt8 type, DataWriter& writer) {
	if (_first) {
		_first = false;
		writer.writeString(_method.data(),_method.size());
		return true;
	}

	Exception ex;

	switch (type) {
		case NAME:
			// means multiple <name></name> in <member>
			return readNext(writer);
		case NIL:
			writer.writeNull();
			return true;
		case STRING:
			writer.writeString(_data,_size);
			return true;
		case NUMBER: {
			 double number(String::ToNumber<double>(ex, _data, _size));
			 if (ex) {
				ERROR("Bad XML-RPC number value, ",ex.error());
				writer.writeNull();
			} else
				writer.writeNumber(number);
			return true;
		}
		case DATE: {
			Date date;
			date.update(ex,_data, _size);
			 if (ex) {
				ERROR("Bad XML-RPC dateTime value, ",ex.error());
				writer.writeNull();
			} else
				writer.writeDate(date);
			return true;
		}
		case BOOLEAN: {
			if (_size == 0 || *_data=='0' || (_size==5 && String::ICompare(_data,"false")==0))
				writer.writeBoolean(false);
			else
				writer.writeBoolean(true);
			return true;
		}
		case BYTES: {
			if (Util::FromBase64(BIN _data, _size, *_pBuffer)) {
				writer.writeBytes(_pBuffer->data(), _pBuffer->size());
				return true;
			}
			WARN("XMLRPC base64 data should be in a base64 encoding format");
			writer.writeBytes(BIN _data, _size);
			return true;
		}

		case ARRAY: {
			UInt32 count(0);
			if (_attribute.empty() || !String::ToNumber(_attribute, count)) {
				// count ARRAY element
				XMLState state;
				save(state);
				_countingLevel = _xmls.size() + 1; // +1 for <data>
				while (parse() == ARRAY)
					++count;
				_countingLevel = 0;
				XMLParser::reset(state);
				if (!isValid())
					count = 0;
			}
			
			writer.beginArray(count);

			while (count-- > 0) {
				if (!readNext(writer))
					writer.writeNull();
			}
	
			writer.endArray();
			return true;
		}
	}

	// OBJECT or OBJECT_TYPED
	if (type==OBJECT)
		writer.beginObject();
	else
		writer.beginObject(_attribute.c_str());

	while (parse() == NAME) {
		writer.writePropertyName(_data);
		if (!readNext(writer))
			writer.writeNull();
	}

	writer.endObject();

	return true;
}

} // namespace Mona
