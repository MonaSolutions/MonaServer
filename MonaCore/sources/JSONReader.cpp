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
#include <cstring>

using namespace std;

namespace Mona {

bool JSONReader::IsValid(PacketReader& packet) {
	const UInt8* cur = packet.current();
	while(packet.available()>0 && isspace(*cur))
		 ++cur;
	if(packet.available()==0)
		return false;
	return cur[0]=='{' || cur[0]=='[';
}


JSONReader::JSONReader(PacketReader& packet) : DataReader(packet),_bool(false),_last(0) {
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

Time& JSONReader::readTime(Time& time) {
	_last=0;
	return time.update(_date);
}

string& JSONReader::readString(string& value) {
	_last=0;
	value.assign(_text);
	_text.clear();
	return value;
}

double JSONReader::readNumber() {
	const UInt8* cur = current();
	if(!cur) {
		ERROR("JSON number absent, no more data available")
		return 0;
	}
	UInt32 pos = packet.position();
	UInt8 c = packet.read8();
	while(available() && (isdigit(c) || c=='.'))
		c = packet.read8();

	UInt32 size = packet.position()-pos;
	string value((const char*)cur,size);

	Exception ex;
	double dval = String::ToNumber<double>(ex, value);
	if (ex) {
		ERROR("JSON number malformed, ",ex.error());
		return 0;
	}
	return dval;
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
		return TIME;
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
		if(_bool) {
			Buffer result;
			Util::FromBase64((const UInt8*)_text.c_str(), size, result);
			_text.assign((const char*)result.data(),result.size());
		}
		packet.next(1); // skip the second '"'
		cur = current();
		_last=1; // String marker
		if(cur && cur[0]==':') {
			packet.next(1);
			_bool=true;
			return STRING;
		}
		if (_date.fromString(_text)) {
			_last=2;
			return TIME;
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
