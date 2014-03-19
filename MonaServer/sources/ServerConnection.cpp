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

#include "ServerConnection.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"


using namespace std;
using namespace Mona;


ServerConnection::ServerConnection(const SocketManager& manager, const SocketAddress& targetAddress) : address(targetAddress), _size(0), TCPClient(manager), _connected(false), isTarget(true) {

}

ServerConnection::ServerConnection(const SocketAddress& peerAddress,SocketFile& file,const SocketManager& manager) : address(peerAddress), _size(0), TCPClient(peerAddress, file,manager), _connected(false), isTarget(false) {
	
}

ServerConnection::~ServerConnection() {
	close();
}

UInt16 ServerConnection::port(const string& protocol) {
	map<string, UInt16>::const_iterator it = _ports.find(protocol);
	if(it==_ports.end())
		return 0;
	return it->second;
}

void ServerConnection::sendHello(const string& host,const map<string,UInt16>& ports) {
	shared_ptr<ServerMessage> pMessage(new ServerMessage("",manager().poolBuffers));
	BinaryWriter& writer = pMessage->packet;
	writer.writeString(host);
	writer.write8(ports.size());
	/// ports
	for(auto& it : ports) {
		writer.writeString(it.first); // protocol
		writer.write16(it.second); // port
	}
	/// properties
	for(auto& it: *this) {
		writer.writeString(it.first); // name
		writer.writeString(it.second); // value
	}
	send(pMessage);
}

void ServerConnection::connect(const string& host,const map<string,UInt16>& ports) {
	if(_connected)
		return;
	INFO("Attempt to join ", address.toString(), " server")
	Exception ex;
	bool success = false;
	EXCEPTION_TO_LOG(success=TCPClient::connect(ex, address),"ServerConnection")
	if (success)
		sendHello(host,ports);
}

void ServerConnection::send(const shared_ptr<ServerMessage>& pMessage) {
	if (!pMessage)
		return;
	string& handler(pMessage->_handler);
	if(handler.size()>255) {
		handler.resize(255);
		WARN("The server handler '",handler,"' truncated for 255 char (maximum acceptable size)")
	}

	// Search handler!
	UInt32 handlerRef = 0;
	bool   writeRef = false;
	if(!handler.empty()) {
		map<string, UInt32>::iterator it = _sendingRefs.lower_bound(handler);
		if(it!=_sendingRefs.end() && it->first==handler) {
			handlerRef = it->second;
			handler.clear();
			writeRef = true;
		} else {
			handlerRef = _sendingRefs.size()+1;
			_sendingRefs.insert(it, pair<string, UInt32>(handler, handlerRef));
		}
	}

	pMessage->_shift -= (handler.empty() ? Util::Get7BitValueSize(handlerRef) : handler.size());

	BinaryWriter writer(pMessage->packet, pMessage->_shift);

	writer.write32(pMessage->size()-4);
	writer.writeString8(handler);
	if(writeRef)
		writer.write7BitEncoded(handlerRef);
	else if(handler.empty())
		writer.write8(0);

	DUMP_INTERN(pMessage->data() + 4, pMessage->size() - 4, "To ", address.toString()," server");
	Exception ex;
	EXCEPTION_TO_LOG(TCPClient::send(ex,pMessage),"Server ",address.toString());
}


UInt32 ServerConnection::onReception(PoolBuffer& pBuffer) {
	if (_size == 0 && pBuffer->size() < 4)
		return pBuffer->size();

	PacketReader packet(pBuffer->data(), pBuffer->size());
	if(_size==0)
		_size = packet.read32();
	if (packet.available() < _size)
		return pBuffer->size();

	UInt32 rest = packet.available() - _size;
	packet.shrink(_size);
	
	DUMP_INTERN(packet.current(),packet.available(), "From ", address.toString(), " server");

	string handler;
	UInt8 handlerSize = packet.read8();
	if(handlerSize)
		_receivingRefs[_receivingRefs.size() + 1] = packet.readRaw(handlerSize, handler);
	else {
		UInt32 ref = packet.read7BitEncoded();
		if(ref>0) {
			map<UInt32, string>::const_iterator it = _receivingRefs.find(ref);
			if(it==_receivingRefs.end())
				ERROR("Impossible to find the ", ref, " handler reference for the server ", address.toString())
			else
				handler.assign(it->second);
		}
	}

	_size=0;
	if(handler.empty()) {
		packet.readString((string&)host);

		if(host.empty())
			((string&)host) = address.host().toString();
		UInt8 ports = packet.read8();
		string protocol;
		while(ports>0) {
			packet.readString(protocol);
			_ports[protocol] = packet.read16();
			--ports;
		}
		while(packet.available()) {
			string key,value;
			packet.readString(key);
			packet.readString(value);
			setString(key,value);
		}
		if(!_connected) {
			_connected=true;
			OnHello::raise(*this);
			NOTE("Connection etablished with ",address.toString()," server ")
		}
	} else
		OnMessage::raise(*this,handler,packet);

	return rest;
}

void ServerConnection::onDisconnection(){
	_sendingRefs.clear();
	_receivingRefs.clear();
	if(_connected) {
		_connected=false;
		OnGoodbye::raise(*this);
		if (_error.empty())
			NOTE("Disconnection from ", address.toString(), " server ")
		else
			ERROR("Disconnection from ", address.toString(), " server, ",_error)
		_ports.clear();
		((string&)host).clear();
	}
	_error.clear();
	OnDisconnection::raise(*this);
}
