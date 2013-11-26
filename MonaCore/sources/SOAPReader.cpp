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

#include "Mona/SOAPReader.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"
#include <sstream>
#include <cstring>

using namespace std;

namespace Mona {

SOAPReader::SOAPReader(MemoryReader& reader) : XMLReader(reader), _body(false) {}

SOAPReader::Type SOAPReader::followingType() {
	
	string name, tmp;
	bool external;
	UInt32 size = 0;

	// Search the Body tag
	while(!_body && XMLReader::followingType()==OBJECT) {
		readObject(name, external);

		// ignore attributes
		while(readItem(tmp)!=END)
			readString(tmp);

		if (XMLReader::followingType()==ARRAY)
			readArray(size);

		if (name == "soapenv:Body") {
			_body = true;
		}
	}

	if (!_body) {

		ERROR(Exception::PROTOCOL, "SOAP error, tag Body not founded");
		return END;
	} else {

		return XMLReader::followingType();
	}
}

} // namespace Mona
