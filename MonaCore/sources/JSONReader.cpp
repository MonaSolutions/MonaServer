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


JSONReader::JSONReader(PacketReader& packet,const PoolBuffers& poolBuffers) : _pos(packet.position()),_pBuffer(poolBuffers),DataReader(packet),_isValid(false) {

	// check first '[' and last ']' or '{ and '}'

	const UInt8* cur = current();
	if (!cur)
		return;
	bool isArray(*cur=='[');
	if (!isArray && *cur != '{')
		return;

	packet.next();
	_pos = packet.position();

	if (!(cur = current()))
		return;

	const UInt8* end(cur + packet.available());
	
	while (end-- > cur) {
		if (!isspace(*end)) {
			if (isArray) {
				if (*end == ']') {
					_isValid = true;
					packet.shrink(end-cur);
				}
			} else if (*end == '}') {
				_isValid = true;
				packet.shrink(end-cur);
			}
			return;
		}
	}
}


UInt8 JSONReader::followingType() {
	if (!_isValid)
		return END;

	const UInt8* cur = current();
	if(!cur)
		return END;

	if (*cur == ',') {
		// avoid two following comma
		packet.next();
		cur = current();
		if (!cur) {
			ERROR("JSON malformed, no end");
			return END;
		}
		if (*cur == ',' || *cur == '}' || *cur == ']') {
			ERROR("JSON malformed, double comma without any value between");
			packet.next(packet.available());
			return END;
		}
		return followingType();
	}

	if (*cur == '{')
		return OBJECT;

	if(*cur=='[')
		return ARRAY;

	if(*cur=='}' || *cur==']') {
		packet.next(packet.available());
		ERROR("JSON malformed, marker ",*cur," without beginning");
		return END;
	}

	
	if(*cur=='"') {
		const char* value(jumpToString(_size));
		if (!value)
			return END;
		Exception ex;
		if (_date.update(ex,value,_size))
			return DATE;
		return STRING;
	}

	// necessary one char
	UInt32 available(packet.available());
	_size = 0;
	const char* value((const char*)cur);
	do {
		--available;
		++_size;
		++cur;
	} while (available && *cur != ',' && *cur != '}' && *cur != ']');

	if (_size == 4) {
		if (String::ICompare(value, "true",4) == 0) {
			_number = 1;
			return BOOLEAN;
		}
		if(String::ICompare(value, "null",4) == 0)
			return NIL;
	} else if (_size == 5 && String::ICompare(value, "false",5) == 0) {
		_number = 0;
		return BOOLEAN;
	}
	if (String::ToNumber(value,_size, _number))
		return NUMBER;

	ERROR("JSON malformed, unknown ",string(value,_size)," value");
	packet.next(packet.available());
	return END;
}


bool JSONReader::readOne(UInt8 type, DataWriter& writer) {

	switch (type) {

		case STRING:
			packet.next(1); // skip "
			writer.writeString(STR packet.current(), _size);
			packet.next(_size+1); // skip string"
			return true;

		case NUMBER:
			packet.next(_size);
			writer.writeNumber(_number);
			return true;

		case BOOLEAN:
			packet.next(_size);
			writer.writeBoolean(_number!=0);
			return true;

		case DATE:
			packet.next(_size+2); // "date"
			writer.writeDate(_date);
			return true;

		case NIL:
			packet.next(4); // null
			writer.writeNull();
			return true;

		case ARRAY: {
			packet.next(); // skip [
			// count number of elements
			UInt32 count(0);
			countArrayElement(count);
			// write array
			writer.beginArray(count);
			while (count-- > 0) {
				if(!readNext(writer))
					writer.writeNull();
			}
			writer.endArray();
			// skip ]
			current();
			packet.next();

			return true;
		}
	}

	// Object
	packet.next(); // skip {

	bool started(false);
	const UInt8* cur(NULL);
	while ((cur=current()) && *cur != '}') {

		if (started) {
			if (*cur != ',') {
				// skip comma , (2nd iteration)
				ERROR("JSON malformed, comma object separator absent");
				packet.next(packet.available());
				writer.endObject();
				return true;
			}
			packet.next();
			cur = current();
			if (!cur)
				break;
		}

		const char* name(jumpToString(_size));
		if (!name) {
			if (!started)
				return  false;
			writer.endObject();
			return true;
		}
		packet.next(2+_size); // skip "string"

		if (!jumpTo(':')) {
			if (!started)
				return  false;
			writer.endObject();
			return true;
		}
		packet.next();

		// write key
		if (!started) {
			if (!(cur = current())) {
				ERROR("JSON malformed, value object of property ",string(name, _size)," absent");
				return false;
			}
			if (*cur == '"') {

				if (_size >= 4 && String::ICompare(name + (_size - 4), EXPAND("type"))==0) { // finish by "type" ("__type")
					UInt32 size;
					const char* value(jumpToString(size));
					if (!value)
						return false;
					packet.next(2+size); // skip "string"
					SCOPED_STRINGIFY(value,size, writer.beginObject( value) )
					started = true;
					continue;
				}
				
				if (_size >= 3 && String::ICompare(name + (_size - 3), EXPAND("raw"))==0) { // finish by "raw" ("__raw")
					UInt32 size;
					const char* value(jumpToString(size));
					if (!value)
						return false;
					if (Util::FromBase64((const UInt8*)value, size, *_pBuffer)) {
						packet.next(2 + size); // skip "string"
						writer.writeBytes(_pBuffer->data(), _pBuffer->size());
						ignoreObjectRest();
						return true;
					}
					WARN("JSON raw ", string(name, _size), " data must be in a base64 encoding format to be acceptable");
				}
			}

			writer.beginObject();
			started = true;
		}

		SCOPED_STRINGIFY(name, _size, writer.writePropertyName(name))
	
		// write value
		if (!readNext(writer)) {
			// here necessary position is at the end of the packet
			writer.writeNull();
			writer.endObject();
			return true;
		}

	}

	if (!started)
		writer.beginObject();
	writer.endObject();

	if (cur)
		packet.next(); // skip }
	else
		ERROR("JSON malformed, no object } end marker");

	return true;
}

const char* JSONReader::jumpToString(UInt32& size) {
	if (!jumpTo('"'))
		return NULL;
	const UInt8* cur(packet.current()+1);
	const UInt8* end(cur+packet.available()-1);
	size = 0;
	while (cur<end && *cur != '"') {
		if (*cur== '\\') {
			++size;
			if (++cur == end)
				break;
		}
		++size;
		++cur;
	}
	if(cur==end) {
		packet.next(packet.available());
		ERROR("JSON malformed, marker \" end of text not found");
		return NULL;
	}
	return (const char*)packet.current() + 1;
}

bool JSONReader::jumpTo(char marker) {
	const UInt8* cur(current());
	if (!cur || *cur != marker) {
		ERROR("JSON malformed, marker ", marker , " unfound");
		packet.next(packet.available());
		return false;
	}
	return cur != NULL;
}

const UInt8* JSONReader::current() {
	while(packet.available() && isspace(*packet.current()))
		packet.next(1);
	if(!packet.available())
		return NULL;
	return packet.current();
}


bool JSONReader::countArrayElement(UInt32& count) {
	count = 0;
	const UInt8* cur(current());
	const UInt8* end(cur+packet.available());
	UInt32 inner(0);
	bool nothing(true);
	bool done(false);
	while (cur<end && (inner || *cur != ']')) {

		// skip string
		if (*cur == '"') {
			while (++cur<end && *cur != '"') {
				if (*cur== '\\') {
					if (++cur == end)
						break;
				}
			}
			if(cur==end) {
				packet.next(packet.available());
				ERROR("JSON malformed, marker \" end of text not found");
				return false;
			}
			nothing = false;
		} else if (*cur == '{' || *cur == '[') {
			++inner;
			nothing = true;
		} else if (*cur == ']' || *cur == '}') {
			if (inner == 0) {
				packet.next(packet.available());
				ERROR("JSON malformed, marker ", *cur, " without beginning");
				return false;
			}
			--inner;
			nothing = false;
		} else if (*cur == ',') {
			if (nothing) {
				packet.next(packet.available());
				ERROR("JSON malformed, comma separator without value before");
				return false;
			}
			nothing = true;
			if(inner==0)
				done = false;
		} else if (!isspace(*cur))
			nothing = false;

		if (inner == 0 && !done && !nothing) {
			++count;
			done = true;
		}
		++cur;
	}
	if (cur==end) {
		ERROR("JSON malformed, marker ] end of array not found");
		return false;
	}
	return true;
}


void JSONReader::ignoreObjectRest() {
	UInt8 c;
	UInt32 inner(0);
	while (packet.available() && (inner || (c=packet.read8()) != '}')) {

		// skip string
		if (c == '"') {
			while (packet.available() && (c=packet.read8()) != '"') {
				if (c== '\\') {
					if (packet.available()==0)
						break;
					packet.next();
				}
			}
			if(packet.available()==0) {
				packet.next(packet.available());
				ERROR("JSON malformed, marker \" end of text not found");
				return;
			}
		} else if (c == '{' || c == '[') {
			++inner;
		} else if (c == ']' || c == '}') {
			if (inner == 0) {
				packet.next(packet.available());
				ERROR("JSON malformed, marker ", c, " without beginning");
				return;
			}
			--inner;
		}
	}
	if (packet.available()==0)
		ERROR("JSON malformed, marker } end of object not found");
}

} // namespace Mona
