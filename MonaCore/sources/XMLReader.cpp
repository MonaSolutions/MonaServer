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

#include "Mona/XMLReader.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"
#include <sstream>
#include <cstring>

using namespace std;

namespace Mona {

XMLReader::XMLReader(PacketReader& packet) : DataReader(packet),_last(NIL),_object(false) {}

void XMLReader::reset() {
	packet.reset(_pos);
	_text.clear();
	_last=NIL;
	_dval=0;
}

const UInt8* XMLReader::readBytes(UInt32& size) {
	_last=NIL;
	size = _text.size();
	return (const UInt8*)_text.c_str();
}

bool XMLReader::readBoolean() {
	_last=NIL;
	if(_last==STRING) {
		string tmp = _text;
		String::ToLower(tmp);
		if (tmp=="true")
			return true;
		else if (tmp=="false")
			return false;
	}	

	return _dval > 0;
}

Time& XMLReader::readTime(Time& time) {
	_last=NIL;
	return time.update(_date);
}

string& XMLReader::readString(string& value) {
	_last=NIL;
	value.assign(_text);
	return value;
}

double XMLReader::readNumber() {
	_last=NIL;
	return _dval;
}

bool XMLReader::readObject(string& type,bool& external) {
	_last=NIL;
	
	type.assign(_text);
	return true;
}

bool XMLReader::readArray(UInt32& size) {
	_last=NIL;

	return true;
}

XMLReader::Type XMLReader::readItem(string& name) {
	const UInt8* cur = current();
	if(!cur) {
		ERROR("XML item absent, no more data available")
		return END;
	}

	// Reading Attribute name
	Type type=END;
	if (_object) {
		
		// Ignore '/'
		if (cur[0]=='/') {
			packet.next(1);
			cur=current();

			if (!cur || cur[0]!='>')
				ERROR("XML item absent, no more data available")
			else {
				packet.next(1); // not an array
				_object=false;
			}
			return END;
		}

		if (cur[0]=='>') {
			_object=false;

			// Primitive value?
			UInt32 available = packet.available();
			while(available && isspace(*++cur))
				--available;

			if (available && (isalnum(cur[0]) || cur[0]=='-' || cur[0]=='\"')) {
				name="__value";
				return XMLReader::followingType();
			}

			return END;
		}

		UInt32 pos = packet.position();
		while((cur=current()) && cur[0]!='=')
			packet.next(1);

		if (!cur) {
			ERROR("XML item absent, no more data available")
			return END;
		}

		UInt32 size = packet.position()-pos;
		packet.reset(pos);
		packet.readRaw(size,name);
		packet.next(1);
		type = XMLReader::followingType();
	}

	return type;
}

XMLReader::Type XMLReader::followingType() {
	if(_last!=NIL)
		return _last;
	if(!_text.empty())
		_text.clear();
	if(_dval)
		_dval=0;
	if(!available())
		return END;

	const UInt8* cur = current();
	if(!cur)
		return END;

	// Tag begin/end
	if(cur[0]=='<') {
		packet.next(1);
		cur=current();

		// End of the tag/Header/Comment
		if (cur && (cur[0] == '/' || cur[0] == '?' || cur[0] == '!')) {

			do {
				packet.next(1);
			} while((cur=current()) && (cur[0]!='>'));

			if(!available())
				return END;

			// go to next object
			packet.next(1);
			return XMLReader::followingType();
		}

		// Read Tag name
		UInt32 pos = packet.position();
		while(available() && cur[0]!='>' && !isspace(cur[0])) {
			packet.next(1);
			cur=packet.current();
		}

		if (!available()) {
			ERROR("XML malformed, tag does not terminate");
			return END;
		}

		UInt32 size = packet.position()-pos;
		packet.reset(pos);
		packet.readRaw(size,_text);

		// object of primitive type? => return primitive
		cur=current();
		if (cur[0]=='>') {
			
			_object=false;
			packet.next(1);
			cur=current();
			if(cur && (isalnum(cur[0]) || cur[0]=='-' || cur[0]=='\"'))
				return XMLReader::followingType();
		}
		else 
			_object=true;

		_last = OBJECT;
		return _last;
	}
	// Value
	else if(isalnum(cur[0]) || cur[0]=='-' || cur[0]=='\"') {
		
		bool chained=false;
		if (cur[0]=='\"') {
			chained=true;
			packet.next(1);
		}

		UInt32 pos = packet.position();
		while((cur=current()) && cur[0]!='\"' && cur[0]!='<')
			packet.next(1);

		if(!cur) {
			ERROR("XML malformed, tag does not terminate");
			return END;
		}
		if(chained != (cur[0]=='\"')) {
			ERROR("XML malformed, character '\"' founded inappropriately");
			return END;
		}

		UInt32 size = packet.position()-pos;
		packet.reset(pos);
		packet.readRaw(size,_text);
		String::Trim(_text);

		_last=STRING;
		Exception ex;
		_dval = String::ToNumber<double>(ex, _text);
		if (!ex) 
			_last = NUMBER;
		else if (_text.size() > 18 && _text.size() < 34 && _date.fromString(_text))
			_last=TIME;

		if(chained)
			packet.next(1); // remove last '"'
		
		return _last;
	}
	// Tag with subtags
	else if(cur[0]=='>') {
		packet.next(1);
		
		_last=ARRAY;
		return _last;
	}

	return END;
}

const UInt8* XMLReader::current() {
	while(available() && isspace(*packet.current()))
		packet.next(1);
	if(!available())
		return NULL;
	return packet.current();
}

} // namespace Mona
