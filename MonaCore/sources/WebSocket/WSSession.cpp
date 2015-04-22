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
#include "Mona/JSONReader.h"
#include "Mona/StringReader.h"
#include "Mona/MIME.h"


using namespace std;


namespace Mona {


class WSSource : public virtual Object {
public:
	WSSource(PoolBuffer& pBuffer) : pBuffer(pBuffer.poolBuffers) { this->pBuffer.swap(pBuffer); }
	PoolBuffer	pBuffer;
};


WSSession::WSSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) : TCPSession(peerAddress, file,protocol, invoker), _writer(*this), _enabled(false),_pListener(NULL), _pPublication(NULL),
	_decoder(invoker),onDecoded([this](WSReader& reader,const SocketAddress& address){receive(reader);}), onDecodedEnd([this](){flush();}) {
}

void WSSession::enable() {
	DEBUG("Upgrading ", name(), " to WebSocket")
	((string&)this->peer.protocol) = "WebSocket";
	_decoder.OnDecodedEnd::subscribe(onDecodedEnd);
	_decoder.OnDecoded::subscribe(onDecoded);
	_enabled = true;
}

void WSSession::kill(UInt32 type){
	if(died)
		return;
	if (type!=Session::SOCKET_DEATH && _writer.state() != Writer::CLOSED)
		_writer.close(type);
	closePublication();
	closeSusbcription();
	TCPSession::kill(type);

	// no more reception
	_decoder.OnDecoded::unsubscribe(onDecoded);
	_decoder.OnDecodedEnd::unsubscribe(onDecodedEnd);
}

bool WSSession::openSubscribtion(Exception& ex, const string& name, Writer& writer) {
	closeSusbcription();
	_pListener=invoker.subscribe(ex, peer, name, writer,peer.query.c_str());
	return _pListener ? true : false;
}
void WSSession::closeSusbcription(){
	if (_pListener) {
		 // not remove onListenerStart and onListenerStop to raise events on invoker.unsubscribe
		invoker.unsubscribe(peer,_pListener->publication.name());
		_pListener=NULL;
	}
}

bool WSSession::openPublication(Exception& ex, const string& name, Publication::Type type) {
	closeSusbcription();
	_pPublication=invoker.publish(ex, peer, name, type);
	return _pPublication ? true : false;
}
void WSSession::closePublication(){
	if(_pPublication) {
		invoker.unpublish(peer,_pPublication->name());
		_pPublication=NULL;
	}
}

void WSSession::receive(WSReader& packet) {

	if (!TCPSession::receive(packet))
		return;

	Exception ex;
	if(peer.connected) {
		switch(packet.type) {
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
				ERROR(ex.set(Exception::PROTOCOL, Format<UInt8>("Type %#x unknown ", packet.type), WS::CODE_MALFORMED_PAYLOAD).error());
				break;
		}
		
		if (ex)
			_writer.close((ex.code()==Exception::APPLICATION || ex.code() == Exception::SOFTWARE) ? (Int32)WS::CODE_PROTOCOL_ERROR : ex.code());	

	}

	if (!peer.connected) {
		_writer.close(WS::CODE_POLICY_VIOLATION);
		kill(REJECTED_DEATH);
	} else if (packet.type==WS::TYPE_CLOSE)
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
			Publication::Type type(Publication::LIVE);
			std::string mode;
			if (reader.readString(mode)) {
				if(String::ICompare(mode,"record") == 0)
						type = Publication::RECORD;
			}
			EXCEPTION_TO_LOG(openPublication(ex, name, type),"Publish ",name);
			return;

		}
		
		if(name=="__play") {
			if(!reader.readString(name)) {
				ex.set(Exception::PROTOCOL, "__play method takes a stream name in first parameter",WS::CODE_MALFORMED_PAYLOAD);
				return;
			}	
			EXCEPTION_TO_LOG(openSubscribtion(ex, name, _writer),"Play ",name);
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
