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

#include "Mona/QueryWriter.h"


using namespace std;


namespace Mona {

const char* QueryWriter::query() const {
	((BinaryWriter&)packet).write8(0);
	const char* query(STR packet.data());
	((BinaryWriter&)packet).clear(packet.size() - 1);
	return query;
}

PacketWriter& QueryWriter::writer() {
	if (_isProperty) {
		packet.write("=");
		_isProperty = false;
	} else if (!_first)
		packet.write("&");
	else
		_first = false;
	return packet;
}

void QueryWriter::writePropertyName(const char* value) {
	if (!_first)
		packet.write("&"); 
	else
		_first = false;
	Util::EncodeURI(value, packet);
	_isProperty = true;
}


} // namespace Mona
