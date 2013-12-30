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

#include "Mona/TCPSession.h"
#include "Mona/Protocol.h"

using namespace std;


namespace Mona {

TCPSession::TCPSession(const SocketAddress& peerAddress, Protocol& protocol, Invoker& invoker) : TCPClient(peerAddress,invoker.sockets), Session(protocol, invoker),_consumed(false) {
	((SocketAddress&)peer.address).set(peerAddress);
}

void TCPSession::onError(const string& error) {
	WARN("Protocol ", protocol().name, ", ", error);
}

UInt32 TCPSession::onReception(const UInt8* data, UInt32 size) {
	if (died)
		return 0;
	MemoryReader packet(data, size);
	if (!buildPacket(packet)) {
		if (_pDecoding)
			_pDecoding.reset();
		if (_pLastDecoding) {
			Session::decode<Decoding>(_pLastDecoding); // flush
			_pLastDecoding.reset();
		}
		if (_consumed) {
			Session::flush(); // flush
			_consumed = false;
		}
		return size;
	}

	UInt32 length = packet.position() + packet.available();
	UInt32 rest = size - length;

	if (_pLastDecoding) {
		_pLastDecoding->_noFlush = true;
		Session::decode<Decoding>(_pLastDecoding);
		_pLastDecoding.reset();
	}

	if (_pDecoding) {
		if (rest == 0)
			Session::decode<Decoding>(_pDecoding); // flush
		else
			_pLastDecoding = _pDecoding;
		_pDecoding.reset();
	} else {
		UInt32 pos = packet.position();
		packet.reset();
		if (!dumpJustInDebug || (dumpJustInDebug && Logs::GetLevel() >= 7))
			DUMP(packet, "Request from ", peer.address.toString());
		packet.next(pos);
		packetHandler(packet);
		if (rest == 0) {
			Session::flush(); // flush
			_consumed = false;
		} else
			_consumed = true;
	}
	return rest;
}



} // namespace Mona
