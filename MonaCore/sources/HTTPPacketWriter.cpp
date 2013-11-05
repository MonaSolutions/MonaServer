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

#include "Mona/HTTPPacketWriter.h"

using namespace std;


namespace Mona {



void HTTPPacketWriter::writeDate(const Time& date) {
	string str;
	writeString(date.toString(Time::HTTP_FORMAT, str));
}

void HTTPPacketWriter::writePropertyName(const string& value) {
	writer.writeRaw(value);
	writer.writeRaw(": ",2);
}

void HTTPPacketWriter::writeString(const string& value) {
	writer.writeRaw(value);
	writer.writeRaw("\r\n",2);
}

void HTTPPacketWriter::writeBoolean(bool value) {
	if(value)
		writer.writeRaw("true\r\n",6);
	else
		writer.writeRaw("false\r\n",7);
}


} // namespace Mona
