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


ServerConnection::ServerConnection(const SocketManager& manager, const SocketAddress& targetAddress) : address(targetAddress), _pClient(new TCPClient(manager)), _connected(false), isTarget(true),
	_onError([this](const Mona::Exception& ex) { _ex.set(ex); }),
	_onData([this](Mona::PoolBuffer& pBuffer) { return onData(pBuffer); }),
	_onDisconnection([this]() { onDisconnection(); }) {

	_pClient->OnError::subscribe(_onError);
	_pClient->OnDisconnection::subscribe(_onDisconnection);
	_pClient->OnData::subscribe(_onData);
}

ServerConnection::ServerConnection(const SocketAddress& peerAddress,SocketFile& file,const SocketManager& manager) : address(peerAddress), _pClient(new TCPClient(peerAddress,file,manager)), _connected(false), isTarget(false),
	_onError([this](const Mona::Exception& ex) { _ex.set(ex); }),
	_onData([this](Mona::PoolBuffer& pBuffer) { return onData(pBuffer); }),
	_onDisconnection([this]() { onDisconnection(); }) {

	_pClient->OnError::subscribe(_onError);
	_pClient->OnDisconnection::subscribe(_onDisconnection);
	_pClient->OnData::subscribe(_onData);
}

ServerConnection::~ServerConnection() {
	_pClient->OnData::unsubscribe(_onData);
	_pClient->OnDisconnection::unsubscribe(_onDisconnection);
	_pClient->OnError::unsubscribe(_onError);
}

UInt16 ServerConnection::port(const string& protocol) {
	map<string, UInt16>::const_iterator it = _ports.find(protocol);
	if(it==_ports.end())
		return 0;
	return it->second;
}

void ServerConnection::sendHello(const string& host,const map<string,UInt16>& ports) {
	shared_ptr<ServerMessage> pMessage(new ServerMessage("",_pClient->manager().poolBuffers));
	BinaryWriter& writer = pMessage->packet;
	writer.writeBool(true);
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

void ServerConnection::reject(const char* error) {
	if (!_connected)
		return;
	shared_ptr<ServerMessage> pMessage(new ServerMessage("",_pClient->manager().poolBuffers));
	BinaryWriter& writer = pMessage->packet;
	writer.writeBool(false);
	writer.writeString(error);
	send(pMessage);
	_pClient->disconnect();
}


void ServerConnection::close() {
	_pClient.reset(new TCPClient(_pClient->manager()));
	_pClient->OnError::subscribe(_onError);
	_pClient->OnDisconnection::subscribe(_onDisconnection);
	_pClient->OnData::subscribe(_onData);
}

void ServerConnection::connect(const string& host,const map<string,UInt16>& ports) {
	if(_connected)
		return;
	INFO("Attempt to join ", address.toString(), " server")
	_ex.set(Exception::NIL);
	Exception ex;
	bool success(false);
	EXCEPTION_TO_LOG(success=_pClient->connect(ex, address),"ServerConnection to ", address.toString(), ", ");
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
	_pClient->send(ex,pMessage);
	if (ex)
		WARN("Server ", address.toString(), " sending, ", ex.error())
}


UInt32 ServerConnection::onData(PoolBuffer& pBuffer) {

	// HEADER FORMAT
	// 4 size bytes, 1 byte for the handler size
	// handler>0 => handler bytes, the rest is the playload
	// handler==0 => Index handler 7BitEncoded bytes
	// Index handler>0 => the rest is the playload
	// Index handler==0 => HELLO MESSAGE => String host, 1 byte indication on ports count, String protocol name, UInt16 port, parameters (String key, String value)

	if (pBuffer->size() < 4)
		return pBuffer->size();

	PacketReader packet(pBuffer->data(), pBuffer->size());
	UInt32 size(packet.read32());
	if (packet.available() < size)
		return pBuffer->size();

	UInt32 rest = packet.available() - size;
	packet.shrink(size);
	
	DUMP_INTERN(packet.current(),packet.available(), "From ", address.toString(), " server");

	string handler;
	UInt8 handlerSize = packet.read8();
	if (handlerSize>0)
		_receivingRefs.emplace(_receivingRefs.size()+1,packet.readRaw(handlerSize, handler));
	else {
		UInt32 ref = packet.read7BitEncoded();
		if (ref == 0) {
			bool isHello(packet.readBool());
			if (!isHello) {
				ERROR("Connection with ", address.toString(), " server, ",packet.readString(handler));
				_pClient->disconnect();
				return 0;
			}
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
			_connected = true;
			OnHello::raise(*this);
			return rest;
		}

		auto it = _receivingRefs.find(ref);
		if(it==_receivingRefs.end())
			ERROR("Impossible to find the ", ref, " handler reference for the server ", address.toString())
		else
			handler.assign(it->second);
	}

	OnMessage::raise(*this,handler,packet);
	return rest;
}

void ServerConnection::onDisconnection(){
	_sendingRefs.clear();
	_receivingRefs.clear();
	if(_connected)
		_connected=false;
	_ports.clear();
	((string&)host).clear();
	OnDisconnection::raise(_ex,*this); // in last because can delete this
}
