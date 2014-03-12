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

#include "Mona/WebSocket/WSSession.h"
#include "Mona/WebSocket/WS.h"
#include "Mona/WebSocket/WSUnmasking.h"
#include "Mona/JSONReader.h"
#include "Mona/RawReader.h"


using namespace std;


namespace Mona {


WSSession::WSSession(const SocketAddress& address,const SocketManager& sockets, Protocol& protocol, Invoker& invoker) : TCPSession(address, sockets,protocol, invoker), _writer(*this), _pListener(NULL), _pPublication(NULL) {
}


WSSession::~WSSession() {
	kill();
}

void WSSession::kill(){
	if(died)
		return;
	closePublication();
	closeSusbcription();
	TCPSession::kill();
}

void WSSession::closeSusbcription(){
	if (_pListener) {
		invoker.unsubscribe(peer,_pListener->publication.name());
		_pListener=NULL;
	}
}

void WSSession::closePublication(){
	if(_pPublication) {
		invoker.unpublish(peer,_pPublication->name());
		_pPublication=NULL;
	}
}


bool WSSession::buildPacket(PoolBuffer& pBuffer,PacketReader& packet) {
	if (packet.available()<2)
		return false;
	UInt8 type = packet.read8() & 0x0F;
	UInt8 lengthByte = packet.read8();

	UInt32 size=lengthByte&0x7f;
	if (size==127) {
		if (packet.available()<8)
			return false;
		size = (UInt32)packet.read64();
	} else if (size==126) {
		if (packet.available()<2)
			return false;
		size = packet.read16();
	}

	if(lengthByte&0x80)
		size += 4;

	if (packet.available()<size)
		return false;

	packet.shrink(size);

	if (lengthByte & 0x80) {
		shared_ptr<WSUnmasking> pWSUnmasking(new WSUnmasking(invoker, packet.current(),packet.available(), type));
		decode<WSUnmasking>(pWSUnmasking);
	} else {
		packet.reset(packet.position()-1);
		*(UInt8*)packet.current() = type;
	}
	return true;
}


void WSSession::packetHandler(PacketReader& packet) {
	UInt8 type = 0;
	Exception ex;
	if(peer.connected) {
		type = packet.read8();	
		
		switch(type) {
			case WS::TYPE_BINARY: {
				RawReader reader(packet);
				peer.onMessage(ex, "onMessage",reader,WS::TYPE_BINARY);
				break;
			}
			case WS::TYPE_TEXT: {
				readMessage<JSONReader>(ex, packet);
				break;
			}
			case WS::TYPE_CLOSE:
				_writer.close(packet.available() ? packet.read16() : 0);
				break;
			case WS::TYPE_PING:
				_writer.writePong(packet.current(),packet.available());
				break;
			case WS::TYPE_PONG:
				peer.setPing(_writer.ping = (UInt16)_time.elapsed());
				break;
			default:
				ex.set(Exception::PROTOCOL, Format<UInt8>("Type %#x unknown", type), WS::CODE_MALFORMED_PAYLOAD);
				break;
		}
		
		if (ex) {
			ERROR(ex.error());
			_writer.close((ex.code()==Exception::APPLICATION || ex.code() == Exception::SOFTWARE) ? (int)WS::CODE_PROTOCOL_ERROR : (int)ex.code());	
		}
		
	}

	if(!peer.connected || type==WS::TYPE_CLOSE)
		kill();
	else
		_writer.flush();
}


void WSSession::manage() {
	if(peer.connected && _time.isElapsed(60000)) { // 1 mn
		_writer.writePing();
		_time.update();
	}
}


} // namespace Mona
