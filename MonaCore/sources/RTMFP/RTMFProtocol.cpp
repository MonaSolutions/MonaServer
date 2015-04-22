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

	setNumber("keepalivePeer",   10);
	setNumber("keepaliveServer", 15);
	// timesBeforeTurn, no by default

	onPacket = [this](PoolBuffer& pBuffer,const SocketAddress& address) {
		if (pBuffer->size() < RTMFP_MIN_PACKET_SIZE) {
			ERROR("Invalid RTMFP packet");
			return;
		}
		BinaryReader reader(pBuffer->data(),pBuffer->size());
		UInt32 id = RTMFP::Unpack(reader);
		pBuffer->clip(reader.position());

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
		pSession->decode(address,pBuffer);
	};

	OnPacket::subscribe(onPacket);
}

RTMFProtocol::~RTMFProtocol() {
	OnPacket::unsubscribe(onPacket);
}

bool RTMFProtocol::load(Exception& ex,const SocketAddress& address) {

	if (!UDProtocol::load(ex,address))
		return false;

	
	if (getNumber<UInt16,10>("keepalivePeer") < 5) {
		WARN("Value of RTMFP.keepalivePeer can't be less than 5 sec")
		setNumber("keepalivePeer", 5);
	}
	if (getNumber<UInt16,15>("keepaliveServer") < 5) {
		WARN("Value of RTMFP.keepaliveServer can't be less than 5 sec")
		setNumber("keepaliveServer", 5);
	}

	_pHandshake.reset(new RTMFPHandshake(*this, sessions, invoker));
	return true;
}



} // namespace Mona
