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
#include "Mona/StringReader.h"


using namespace std;


namespace Mona {


WSSession::WSSession(const SocketAddress& address, Protocol& protocol, Invoker& invoker) : TCPSession(address, protocol, invoker), _writer(*this), _pListener(NULL), _pPublication(NULL) {
}


WSSession::~WSSession() {
	kill();
}

void WSSession::kill(){
	if(died)
		return;
	// Impossible to send something here, socket is null!
	if(_pPublication)
		invoker.unpublish(peer,_pPublication->name());
	_pPublication=NULL;
	if(_pListener)
		invoker.unsubscribe(peer,_pListener->publication.name());
	_pListener=NULL;
	TCPSession::kill();
}


bool WSSession::buildPacket(const shared_ptr<Buffer<UInt8>>& pData, MemoryReader& packet) {
	if (pData->size()<2)
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

	if (lengthByte & 0x80) {
		pData->resize(size+packet.position());
		shared_ptr<WSUnmasking> pUnmasking(new WSUnmasking(pData, type, invoker,packet.position()));
		decode<WSUnmasking>(pUnmasking);
	} else {
		packet.reset(packet.position()-1);
		*packet.current() = type;
	}

	packet.shrink(size);
	return true;
}


void WSSession::packetHandler(MemoryReader& packet) {
	
	UInt8 type = 0;
	Exception ex;
	if(peer.connected) {
		type = packet.read8();	
		
		switch(type) {
			case WS_BINARY: {
				StringReader reader(packet);
				peer.onMessage(ex, "onMessage",reader);
				break;
			}
			case WS_TEXT: {
				if(!JSONReader::IsValid(packet)) {
					StringReader reader(packet);
					peer.onMessage(ex, "onMessage",reader);
					break;
				}
				JSONReader reader(packet);
				if(reader.followingType()!=JSONReader::STRING) {
					peer.onMessage(ex, "onMessage",reader);
					break;
				}
				string name;
				reader.readString(name);
				if(name=="__publish") {
					if(reader.followingType()!=JSONReader::STRING) {
						ex.set(Exception::PROTOCOL, "__publish method takes a stream name in first parameter",WS_MALFORMED_PAYLOAD);
						break;
					}
					reader.readString(name);
					if(_pPublication)
						invoker.unpublish(peer,_pPublication->name());
					_pPublication = invoker.publish(ex, peer,name);
				} else if(name=="__play") {
					if(reader.followingType()!=JSONReader::STRING) {
						ex.set(Exception::PROTOCOL, "__play method takes a stream name in first parameter",WS_MALFORMED_PAYLOAD);
						break;
					}
					reader.readString(name);
					if(_pListener)
						invoker.unsubscribe(peer,_pListener->publication.name());
					_pListener = invoker.subscribe(ex, peer,name,_writer);
					
				} else if(name=="__closePublish") {
					if(_pPublication)
						invoker.unpublish(peer,_pPublication->name());
					_pPublication = NULL;
				} else if(name=="__closePlay") {
					if(_pListener)
						invoker.unsubscribe(peer,_pListener->publication.name());
					_pListener = NULL;
				} else if(name=="__close") {
					if(_pPublication)
						invoker.unpublish(peer,_pPublication->name());
					if(_pListener)
						invoker.unsubscribe(peer,_pListener->publication.name());
					_pListener = NULL;
					_pPublication = NULL;
				} else if(_pPublication) {
					reader.reset();
					_pPublication->pushData(reader);
				} else
					peer.onMessage(ex, name,reader);
				break;
			}
			case WS_CLOSE:
				_writer.close(packet.available() ? packet.read16() : 0);
				break;
			case WS_PING:
				_writer.writePong(packet.current(),packet.available());
				break;
			case WS_PONG:
				_writer.ping = (UInt16&)peer.ping = (UInt16)(_time.elapsed()/1000);
				break;
			default:
				ex.set(Exception::PROTOCOL, Format<UInt8>("Type %#x unknown", type), WS_MALFORMED_PAYLOAD);
				break;
		}
		
		if (ex) {
			if (ex.code() != Exception::APPLICATION) {
				ERROR(ex.error());
				_writer.close(ex.code());
			} else
				_writer.close(WS_PROTOCOL_ERROR); // onMessage displays already the error!
		}
		
	}

	if(!peer.connected || type==WS_CLOSE)
		kill();
	else
		_writer.flush();
}


void WSSession::manage() {
	if(peer.connected && _time.elapsed()>60000000) {
		_writer.writePing();
		_time.update();
	}
	_writer.flush();
}


} // namespace Mona
