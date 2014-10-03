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

#include "Mona/HTTPHeaderReader.h"
#include "Mona/Logs.h"
#include "Mona/Exceptions.h"


using namespace std;


namespace Mona {


bool HTTPHeaderReader::readOne(UInt8 type, DataWriter& writer) {

	writer.beginObject();
	writer.writePropertyName(*_header);
	++_header;
	if (++_header == _headers.end() || String::ICompare(*_header,"null")==0)
		writer.writeNull();
	else {
		if (String::ICompare(*_header, "false") == 0)
			writer.writeBoolean(false);
		else if (String::ICompare(*_header, "true") == 0)
			writer.writeBoolean(true);
		else {
			Exception ex;
			Date date;
			if (date.update(ex, *_header)) {
				if (ex)
					WARN("HTTPHeaderReader date, ", ex.error())
					writer.writeDate(date);
			} else {
				double number(0);
				if (String::ToNumber(*_header, number))
					writer.writeNumber(number);
				else
					writer.writeString(*_header,strlen(*_header));
			}
		}
	}
	writer.endObject();
	return true;
}


} // namespace Mona
