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

#include "Mona/HTTPPacketReader.h"
#include "Mona/Logs.h"
#include "Poco/String.h"
#include "Mona/TimeParser.h"
#include "Poco/NumberParser.h"



using namespace std;
using namespace Poco;

namespace Mona {


HTTPPacketReader::HTTPPacketReader(MemoryReader& reader):DataReader(reader),_number(0),_type(MAP) {
}

HTTPPacketReader::~HTTPPacketReader() {
}

void HTTPPacketReader::reset() {
	DataReader::reset();
	_number = 0;
	_type = MAP;
	_name.clear();
	_value.clear();
}


void HTTPPacketReader::readString(string& value) {
	value.assign(_value);
	_value.clear();
	_type = MAP;
}

Mona::Time HTTPPacketReader::readDate() {
	_type = MAP;
	return _date;
}

void HTTPPacketReader::readNull(){
	_type = MAP;
	_value.clear();
}

bool HTTPPacketReader::readBoolean() {
	bool result = _number==0 ? false : true;
	_number=0;
	_type = MAP;
	return result;
}

double HTTPPacketReader::readNumber() {
	double result = _number;
	_number=0;
	_type = MAP;
	return result;
}

const UInt8* HTTPPacketReader::readBytes(UInt32& size) {
	size = _value.size();
	_type = MAP;
	return (const UInt8*)_value.c_str();
}

void HTTPPacketReader::readLine() {
	if(_type==END)
		return;
	_type = END;
	UInt32 available = reader.available();
	if(available==0)
		return;
	const UInt8* cur = reader.current();

	do {
		if(*cur == ':' || (*cur == '\r' && *(cur+1) == '\n'))
			break;
		++cur;
	} while(--available>0);

	_name.assign((char*)reader.current(),cur-reader.current());
	reader.next(_name.size());
	trimInPlace(_name);

	_type = STRING;
	if(*cur==':') {
		reader.next(1);
		--available;
		_type=OBJECT;
		++cur;
	} else {
		_value.assign(_name);
		if(available==0)
			return;
		if(_value.empty()) { // means /r/n/r/n
			reader.next(2);
			_type=END;
			return;
		}
	}

	do {
		if(*cur == '\r') {
			if(*(cur+1) == '\n') {
				break;
			}
		}
		++cur;
	} while (--available>0);

	if(_type==OBJECT) {
		_value.assign((char*)reader.current(),cur-reader.current());
		reader.next(_value.size());
		trimInPlace(_value);
	}

	if(available>0) { // '\r\n' case
		reader.next(2);
		cur+=2;
	}
}


HTTPPacketReader::Type HTTPPacketReader::followingType() {
	if(_type==MAP)
		readLine();
	return _type;
}

HTTPPacketReader::Type HTTPPacketReader::readItem(string& name) {
	if(_type==MAP)
		readLine();
	
	if(_type!=OBJECT)
		return END;

	name.assign(_name);

	_type = STRING;
	if(icompare(_value,"TRUE")== 0) {
		_number = 1;
		_type = BOOLEAN;
	} else if(icompare(_value,"FALSE") == 0){
		_number = 0;
		_type = BOOLEAN;
	} else if(_value == "null")
		_type = NIL;
	else if(NumberParser::tryParseFloat(_value,_number))
		_type = NUMBER;
	else {
		if (_value.size() > 18 && _value.size() < 34 && _date.fromString(_value))
			_type = DATE;
	}
	return _type;
}



} // namespace Mona
