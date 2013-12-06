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

TCPSession::TCPSession(const SocketAddress& peerAddress, Protocol& protocol, Invoker& invoker) : TCPClient(invoker.sockets, peerAddress), Session(protocol, invoker) {
	((SocketAddress&)peer.address).set(peerAddress);
	peer.addresses.begin()->set(peerAddress);
}

void TCPSession::onError(const string& error) {
	ERROR("Protocol ", protocol().name, ", ", error);
}

UInt32 TCPSession::onReception(const shared_ptr<Buffer<UInt8>>& pData) {
	UInt32 size = pData->size();
	if (died)
		return 0;
	MemoryReader packet(pData->data(), pData->size());
	if (!buildPacket(packet,pData))
		return size;
	
	UInt32 length = packet.position() + packet.available();
	pData->resize(length,true);
	if (pData.unique()) { // if not decoded!
		UInt32 pos = packet.position();
		packet.reset();
		if (!dumpJustInDebug || (dumpJustInDebug && Logs::GetLevel() >= 7))
			DUMP(packet, "Request from ", peer.address.toString());
		packet.next(pos);
		packetHandler(packet);
	}
	return size - length;
}



} // namespace Mona
