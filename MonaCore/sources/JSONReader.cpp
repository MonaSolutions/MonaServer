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

#include "Mona/JSONReader.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"
#include <sstream>

using namespace std;

namespace Mona {

bool JSONReader::isValid() {

	// Not tested yet?
	if (!_validated) {
		const UInt8* cur = current();
		_isValid = (packet.available() > 0) && (cur[0]=='{' || cur[0]=='[');
		_validated=true;
	}

	return _isValid;
}


JSONReader::JSONReader(PacketReader& packet) : DataReader(packet),_bool(false),_last(0),_validated(false),_isValid(false) {

	if (!isValid())
		return;

	// Ignore first '[' and last ']'
	if(followingType()==ARRAY) {
		if(readArray(_pos) && packet.available()>0) {
			const UInt8* cur = packet.current()+packet.available()-1;

			while(cur>=packet.current() && isspace(*cur))
				--cur;
			if(cur>=packet.current() && *cur== ']')
		 		packet.shrink(cur-packet.current());
		}
	}
	_pos = packet.position();
}


void JSONReader::reset() {
	packet.reset(_pos);
	_text.clear();
	_bool = false;
	_last =0;
}

const UInt8* JSONReader::readBytes(UInt32& size) {
	_last=0;
	size = _text.size();
	return (const UInt8*)_text.c_str();
}

bool JSONReader::readBoolean() {
	packet.next(_bool ? 4 : 5);
	return _bool;
}

Date& JSONReader::readDate(Date& date) {
	_last=0;
	return date = _date;
}

string& JSONReader::readString(string& value) {
	_last=0;
	value.assign(_text);
	_text.clear();
	return value;
}

double JSONReader::readNumber() {
	Exception ex;
	double result = String::ToNumber<double>(ex, _text);
	if (ex) {
		ERROR("JSON number malformed, ",ex.error());
		return 0;
	}
	return result;
}

bool JSONReader::readObject(string& type,bool& external) {
	const UInt8* cur = current();
	if(!cur) {
		ERROR("JSON array absent, no more data available")
		return false;
	}
	packet.next(1);
	external=false;
	if(cur[0]=='{')
		return true;
	ERROR("Char ",Format<UInt8>("%.2x",cur[0])," doesn't start a JSON object");
	return false;
}

bool JSONReader::readArray(UInt32& size) {
	UInt8* cur = (UInt8*)current();
	if(!cur) {
		ERROR("JSON array absent, no more data available")
		return false;
	}
	packet.next(1);
	if(cur[0]=='[') {
		UInt32 available = packet.available();
		if(available==0) {
			ERROR("JSON array without termination char")
			packet.next(1);
			return false;
		}
		size=0;
		bool isString=false;
		++cur;
		while(isString || (*cur)!=']') {
	
			if((*cur)=='"') {
				isString=!isString;
				if(size==0)
					size=1;
			} else if(!isString) {
				if((*cur)==',') {
					if(size==0)
						++size;
					++size;
				} else if(size==0 && !isspace(*cur))
					size=1;
			}

			if(--available==0)
				break;
			++cur;
		}
		if(isString) {
			ERROR("JSON string without termination char")
			packet.next(packet.available());
			return false;
		}
		return true;
	}
	ERROR("Char ",Format<UInt8>("%.2x",cur[0])," doesn't start a JSON array");
	return false;
}

JSONReader::Type JSONReader::readItem(string& name) {
	const UInt8* cur = current();
	if(!cur) {
		// TODO is it really an error?
		ERROR("JSON item absent, no more data available")
		return END;
	}
	if(cur[0]=='}' || cur[0]==']') {
		packet.next(1);
		cur = current();
		if(cur && cur[0]==',')
			packet.next(1);
		return END;
	}
	_bool=false;
	Type type = followingType();
	if(type==STRING && _bool) { // is key
		_bool = readString(name) == "__raw";
		type = followingType();
		_bool = false;
	}
	return type;
}


JSONReader::Type JSONReader::followingType() {
	if(_last==1)
		return STRING;
	if(_last==2)
		return DATE;
	if(!_text.empty())
		_text.clear();
	if(!available())
		return END;

	const UInt8* cur = current();
	if(!cur)
		return END;
	if(cur[0]==',' || cur[0]=='}' || cur[0]==']') {
		packet.next(1);
		return followingType();
	}
	if(cur[0]=='{')
		return OBJECT;
	if(cur[0]=='[')
		return ARRAY;
	if(cur[0]=='"') {
		packet.next(1);
		UInt32 pos = packet.position();

		while((cur=current()) && cur[0]!='"')
			packet.next(cur[0]=='\\' ? 2 : 1);
		if(!available()) {
			ERROR("JSON malformed, marker \" end of text not found");
			return END;
		}

		UInt32 size = packet.position()-pos;
		packet.reset(pos);
		packet.readRaw(size,_text);
		if(_bool)
			Util::FromBase64(_text);
		packet.next(1); // skip the second '"'
		cur = current();
		_last=1; // String marker
		if(cur && cur[0]==':') {
			packet.next(1);
			_bool=true;
			return STRING;
		}
		Exception ex;
		if (_date.update(ex, _text) && !ex) {
			_last=2;
			return DATE;
		}
		return STRING;
	}
	if(memcmp(cur,"null",4)==0)
		return NIL;
	if(memcmp(cur,"true",4)==0) {
		_bool = true;
		return BOOLEAN;
	}
	if(memcmp(cur,"false",5)==0) {
		_bool = false;
		return BOOLEAN;
	}

	// fill until the next ',' or '}' or ']'
	const UInt8* begin(cur);
	do {
		packet.next(1); ++cur;
	} while (available() && cur[0] != ',' && cur[0] != '}' && cur[0] != ']');
	_text.assign((const char*)begin, cur - begin);
	return NUMBER;
}

const UInt8* JSONReader::current() {
	while(available() && isspace(*packet.current()))
		packet.next(1);
	if(!available())
		return NULL;
	return packet.current();
}



} // namespace Mona
