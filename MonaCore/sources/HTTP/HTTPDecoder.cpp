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

#include "Mona/HTTP/HTTPDecoder.h"
#include "Mona/HTTP/HTTP.h"

using namespace std;

namespace Mona {

UInt32 HTTPDecoder::decoding(Exception& ex, UInt8* data,UInt32 size) {
	std::shared_ptr<HTTPPacket>	pPacket(new HTTPPacket(_rootPath));
	UInt32 consumed = pPacket->build(ex, data, size);
	if (consumed)
		receive(pPacket);
	return consumed;
}

} // namespace Mona
