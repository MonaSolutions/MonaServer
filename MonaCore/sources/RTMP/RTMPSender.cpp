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

#include "Mona/RTMP/RTMPSender.h"

using namespace std;


namespace Mona {

void RTMPSender::pack(RTMPChannel& channel) {
	if (sizePos == 0)
		return;
	// writer the size of the precedent playload!
	channel.bodySize = _writer.packet.size()-sizePos+4-headerSize;
	BinaryWriter(_writer.packet.data()+sizePos,3).write24(channel.bodySize);
	sizePos=0;
}


} // namespace Mona
