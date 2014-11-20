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

#pragma once

#include "Mona/Mona.h"
#include "Mona/UDPSender.h"
#include "Mona/PacketWriter.h"
#include "Mona/RTMFP/RTMFP.h"

namespace Mona {

class RTMFPSender : public UDPSender, public virtual Object {
public:
	RTMFPSender(const PoolBuffers& poolBuffers,const std::shared_ptr<RTMFPEngine>& pEncoder): _pEncoder(pEncoder),UDPSender("RTMFPSender"),packet(poolBuffers),farId(0) {
		packet.next(RTMFP_HEADER_SIZE);
	}
	
	UInt32			farId;
	PacketWriter	packet;

private:
	const UInt8*	data() const { return packet.size() < RTMFP_MIN_PACKET_SIZE ? NULL : packet.data(); }
	UInt32			size() const { return packet.size(); }
	
	bool			run(Exception& ex);

	const std::shared_ptr<RTMFPEngine>	_pEncoder;
};





} // namespace Mona
