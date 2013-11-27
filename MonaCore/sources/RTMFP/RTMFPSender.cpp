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

#include "Mona/RTMFP/RTMFPSender.h"

namespace Mona {

RTMFPSender::RTMFPSender(): packet(_buffer,sizeof(_buffer)),farId(0) {
	packet.clear(11);
	packet.limit(RTMFP_MAX_PACKET_LENGTH); // set normal limit
}


bool RTMFPSender::run(Exception& ex) {
	packet.limit(); // no limit for sending!
	RTMFP::Encode(encoder,packet);
	RTMFP::Pack(packet,farId);
	return UDPSender::run(ex);
}


} // namespace Mona
