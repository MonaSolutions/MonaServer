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

#include "Mona/QueryReader.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"
#include <sstream>

using namespace std;

namespace Mona {

UInt8 QueryReader::followingType() {
	if (_type)
		return _type;

	bool hasProperty(false);
	Util::ForEachParameter forEach([this,&hasProperty](const string& key, const char* value) {
		if (value) {
			_property = std::move(key);
			hasProperty = true;
			_value.assign(value);
		} else
			_value = std::move(key);
		return false; // we just want the first following key
	});

	if (Util::UnpackQuery(STR packet.current(), packet.available(), forEach) == 0)
		return END;

	if (hasProperty)
		_type = OBJECT;
	else
		_type = valueType();
	
	return _type;
}


bool QueryReader::readOne(UInt8 type, DataWriter& writer) {
	
	const UInt8* cur = packet.current();
	const UInt8* end = cur+packet.available();
	if (type==OBJECT) {

		// OBJECT
		writer.beginObject();
		do {
			cur = packet.current();
			writer.writePropertyName(_property.c_str());
			writeValue(valueType(), writer);

			// next!
			while (cur<end && *cur != '&')
				cur++;
			packet.next(cur-packet.current() + (*cur=='&'));

			_type = END;
		} while (cur<end && (_type=followingType())==OBJECT);
		writer.endObject();
		return true;
	}

	writeValue(type, writer);

	// next!
	while (cur<end && *cur != '&')
		cur++;
	packet.next(cur-packet.current() + (*cur=='&'));
	_type = END;
	return true;
}

UInt8 QueryReader::valueType() {

	if (String::ICompare(_value, "null") == 0)
		return NIL;

	if (String::ICompare(_value, "false") == 0) {
		_number = 0;
		return BOOLEAN;
	}

	if (String::ICompare(_value, "true") == 0) {
		_number = 1;
		return BOOLEAN;
	}

	Exception ex;
	if (_date.update(ex, _value)) {
		if (ex)
			WARN("QueryReader date, ", ex.error());
		return DATE;
	}

	if (String::ToNumber(_value, _number))
		return NUMBER;

	return STRING;
}

void QueryReader::writeValue(UInt8 type, DataWriter& writer) {
	switch (type) {

		case STRING:
			writer.writeString(_value.data(),_value.size());
			break;

		case NUMBER:
			writer.writeNumber(_number);
			break;

		case BOOLEAN:
			writer.writeBoolean(_number != 0);
			break;

		case DATE:
			writer.writeDate(_date);
			break;

		default:
			writer.writeNull();
			break;
	}
}


} // namespace Mona
