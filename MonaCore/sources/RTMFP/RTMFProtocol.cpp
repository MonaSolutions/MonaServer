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

#include "Mona/RTMFP/RTMFProtocol.h"


using namespace std;


namespace Mona {


RTMFProtocol::RTMFProtocol(const char* name, Invoker& invoker, Sessions& sessions) : UDProtocol(name, invoker, sessions) {

	onPacket = [this](PoolBuffer& pBuffer,const SocketAddress& address) {
		if (pBuffer->size() < RTMFP_MIN_PACKET_SIZE) {
			ERROR("Invalid RTMFP packet");
			return;
		}
		PacketReader packet(pBuffer->data(),pBuffer->size());
		UInt32 id = RTMFP::Unpack(packet);
		// TRACE("RTMFP Session ",id);
		RTMFPSession* pSession = id == 0 ? _pHandshake.get() : this->sessions.find<RTMFPSession>(id);
		if (!pSession) {
			WARN("Unknown RTMFP session ", id);
			return;
		}
		if (pSession->pRTMFPCookieComputing) {
			_pHandshake->commitCookie(pSession->pRTMFPCookieComputing->value);
			pSession->pRTMFPCookieComputing.reset();
		}
		pSession->decode(pBuffer, address);
	};

	OnPacket::subscribe(onPacket);
}

RTMFProtocol::~RTMFProtocol() {
	OnPacket::unsubscribe(onPacket);
}

bool RTMFProtocol::load(Exception& ex, const RTMFPParams& params) {
	if (!UDProtocol::load(ex, params))
		return false;
	(UInt16&)params.keepAliveServer *= 1000;
	(UInt16&)params.keepAlivePeer *= 1000;
	_pHandshake.reset(new RTMFPHandshake(*this, sessions, invoker));
	return true;
}



} // namespace Mona
