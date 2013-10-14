/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Poco/Format.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

TCPSession::TCPSession(StreamSocket& socket,Protocol& protocol,Invoker& invoker) : _decoding(false),TCPClient(socket,invoker.sockets),Session(protocol,invoker) {
	(SocketAddress&)peer.address = peerAddress();
	(*peer.addresses.begin()) = peerAddress();
	protocol.check(*this);
}


TCPSession::~TCPSession() {
	
}

UInt32 TCPSession::onReception(const UInt8* data,UInt32 size) {
	if(!checked) {
		protocol.check(*this);
		(bool&)checked = true;
	}
	if(died)
		return size;
	_decoding=false;
	MemoryReader packet(data,size);
	UInt32 length=0;
	bool built = buildPacket(packet,length);
	if(!built || size<length)
		return size;
	if(_decoding)
		return size-length;
	UInt32 pos = packet.position();
	packet.reset();
	packet.shrink(length);
	DUMP(packet,format("Request from %s",peerAddress().toString()).c_str())
	packet.next(pos);
	packetHandler(packet);
	return size-length;
}


} // namespace Mona
