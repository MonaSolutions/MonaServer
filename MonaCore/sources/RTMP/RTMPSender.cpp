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
	
bool RTMPSender::run(Exception& ex) {
	if (_pEncryptKey)
		RC4(_pEncryptKey.get(), size(), begin(), (UInt8*)begin());
	return TCPSender::run(ex);
}

AMFWriter& RTMPSender::writer(RTMPChannel& channel) {
	if (sizePos == 0)
		return _writer;
	// writer the size of the precedent playload!
	channel.bodySize = _writer.stream.size()-sizePos+4-headerSize;
	_writer.stream.resetWriting(sizePos);
	_writer.writer.write24(channel.bodySize);
	_writer.stream.resetWriting(sizePos-4+headerSize+channel.bodySize);
	sizePos=0;
	return _writer;
}


} // namespace Mona
