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

using namespace std;

namespace Mona {

SOAPReader::SOAPReader(PacketReader& packet) : XMLReader(packet), _body(false) {}

void SOAPReader::reset() {
	_body=false;
	XMLReader::reset();
}

bool SOAPReader::isValid() {
	
	followingType();

	return _body;
}

DataReader::Type SOAPReader::followingType() {
	
	if (!_body) {
		string name, tmp;
		bool external;
		UInt32 size = 0;
		Type type = NIL;

		if (XMLReader::followingType()==ARRAY) {

			readArray(size);
			while ((type=readItem(name))!=END) {

				// ignore attributes
				if (type != ARRAY) {
					readString(tmp);
					continue;
				}
				readArray(size);

				// Body tag? read items and return
				if (String::ICompare(name, "soapenv:Body")==0) {
					_body=true;
					break;
				}

				// We expect an object
				if (readItem(tmp)!=OBJECT)
					break;
				readObject(tmp, external);
			}
		}
	}

	if (_body) {
		_queueTags.clear();
		return XMLReader::followingType();
	} else
		return END;
}

} // namespace Mona
