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
#include "Mona/MIME.h"


using namespace std;


namespace Mona {


WSSession::WSSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) : TCPSession(peerAddress, file,protocol, invoker), _writer(*this), _pListener(NULL), _pPublication(NULL) {
}


void WSSession::kill(UInt32 type){
	if(died)
		return;
	if (type!=Session::SOCKET_DEATH && _writer.state() != Writer::CLOSED)
		_writer.close(type);
	closePublication();
	closeSusbcription();
	TCPSession::kill(type);
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

	if (lengthByte & 0x80)
		decode<WSUnmasking>(packet.current(),packet.available(), type);
	else {
		packet.reset(packet.position()-1);
		*(UInt8*)packet.current() = type;
	}
	return true;
}


void WSSession::packetHandler(PacketReader& packet) {
	if (!packet.available())
		return ;

	UInt8 type = 0;
	Exception ex;
	if(peer.connected) {
		type = packet.read8();	
		
		switch(type) {
			case WS::TYPE_BINARY: {
				StringReader reader(packet);
				readMessage(ex, reader, WS::TYPE_BINARY);
				break;
			}
			case WS::TYPE_TEXT: {
				unique_ptr<DataReader> pReader;
				if (!MIME::CreateDataReader(MIME::JSON, packet, invoker.poolBuffers, pReader)) {
					pReader.reset(new StringReader(packet));
					readMessage(ex,*pReader, WS::TYPE_BINARY);
				} else
					readMessage(ex,*pReader);
				break;
			}
			case WS::TYPE_CLOSE:
				_writer.close(packet.available() ? packet.read16() : 0);
				break;
			case WS::TYPE_PING:
				_writer.writePong(packet.current(),packet.available());
				break;
			case WS::TYPE_PONG:
				peer.pong();
				break;
			default:
				ex.set(Exception::PROTOCOL, Format<UInt8>("Type %#x unknown", type), WS::CODE_MALFORMED_PAYLOAD);
				ERROR(ex.error());
				break;
		}
		
		if (ex)
			_writer.close((ex.code()==Exception::APPLICATION || ex.code() == Exception::SOFTWARE) ? (Int32)WS::CODE_PROTOCOL_ERROR : ex.code());	

	}

	if (!peer.connected) {
		_writer.close(WS::CODE_POLICY_VIOLATION);
		kill(REJECTED_DEATH);
	} else if (type==WS::TYPE_CLOSE)
		kill();
	else
		_writer.flush();
}


void WSSession::readMessage(Exception& ex, DataReader& reader, UInt8 responseType) {

	std::string name("onMessage");
	
	if (typeid(reader).name() != typeid(StringReader).name() && reader.readString(name)) {

		if(name=="__publish") {
			if(!reader.readString(name)) {
				ex.set(Exception::PROTOCOL, "__publish method takes a stream name in first parameter",WS::CODE_MALFORMED_PAYLOAD);
				return;
			}
			if(_pPublication)
				invoker.unpublish(peer,_pPublication->name());
			Publication::Type type(Publication::LIVE);
			std::string mode;
			if (reader.readString(mode)) {
				if(String::ICompare(mode,"record") == 0)
						type = Publication::RECORD;
			}
			_pPublication = invoker.publish(ex, peer,name,type);
			return;

		}
		
		if(name=="__play") {
			if(!reader.readString(name)) {
				ex.set(Exception::PROTOCOL, "__play method takes a stream name in first parameter",WS::CODE_MALFORMED_PAYLOAD);
				return;
			}	
			closeSusbcription();
			_pListener = invoker.subscribe(ex, peer,name,_writer);
			return;
		}
		
		if (name == "__closePublish") {
			closePublication();
			return;
		}

		if (name == "__closePlay") {
			closeSusbcription();
			return;
		}

		if (name == "__close") {
			closePublication();
			closeSusbcription();
			return;
		}

		if (_pPublication) {
			reader.reset();
			_pPublication->pushData(reader, peer.ping());
			return;
		}

	}

	if(!peer.onMessage(ex, name,reader,responseType))
		ex.set(Exception::APPLICATION, "Method '",name,"' not found on application ", peer.path);
}


void WSSession::manage() {
	if(peer.connected && peer.ping(40000)) // 40 sec
		_writer.writePing();
	TCPSession::manage();
}


} // namespace Mona
