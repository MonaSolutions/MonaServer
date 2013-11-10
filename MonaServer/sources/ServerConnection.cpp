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

#include "ServerConnection.h"
#include "Mona/Util.h"


using namespace std;
using namespace Mona;


ServerConnection::ServerConnection(const SocketAddress& peerAddress, const SocketManager& manager, ServerHandler& handler, ServersHandler& serversHandler,bool alreadyConnected) : address(peerAddress), _size(0), _handler(handler), TCPClient(manager, peerAddress), _connected(false), _serversHandler(serversHandler), isTarget(false) {
	if (alreadyConnected)
		sendPublicAddress();
}


ServerConnection::~ServerConnection() {
	
}

UInt16 ServerConnection::port(const string& protocol) {
	map<string, UInt16>::const_iterator it = _ports.find(protocol);
	if(it==_ports.end())
		return 0;
	return it->second;
}

void ServerConnection::sendPublicAddress() {
	ServerMessage message;
	BinaryWriter& writer = message.writer;
	writer.writeString(_handler.host());
	writer.write8(_handler.ports().size());
	map<string, UInt16>::const_iterator it0;
	for(it0=_handler.ports().begin();it0!=_handler.ports().end();++it0) {
		writer.writeString(it0->first);
		writer.write16(it0->second);
	}
	for(auto& it: *this) {
		writer.writeString(it.first);
		writer.writeString(it.second);
	}
	send("",message);
}

void ServerConnection::connect() {
	if(connected())
		return;
	INFO("Attempt to join ", address.toString(), " server")
	Exception ex;
	bool success = false;
	EXCEPTION_TO_LOG(success=TCPClient::connect(ex, address),"ServerConnection")
	if (success)
		sendPublicAddress();
}

void ServerConnection::send(const string& handler,ServerMessage& message) {
	string handlerName(handler);
	if(handlerName.size()>255) {
		handlerName.resize(255);
		WARN("The server handler '",handlerName,"' truncated for 255 char (maximum acceptable size)")
	}

	// Search handler!
	UInt32 handlerRef = 0;
	bool   writeRef = false;
	if(!handlerName.empty()) {
		map<string, UInt32>::iterator it = _sendingRefs.lower_bound(handlerName);
		if(it!=_sendingRefs.end() && it->first==handlerName) {
			handlerRef = it->second;
			handlerName.clear();
			writeRef = true;
		} else {
			if(it!=_sendingRefs.begin())
				--it;
			handlerRef = _sendingRefs.size()+1;
			_sendingRefs.insert(it, pair<string, UInt32>(handlerName, handlerRef));
		}
	}

	UInt16 shift = handlerName.empty() ? Util::Get7BitValueSize(handlerRef) : handlerName.size();
	shift = 300-(shift+5);

	BinaryStream& stream = message.stream;
	BinaryWriter& writer = message.writer;
	stream.resetReading(shift);
	UInt32 size = stream.size();
	stream.resetWriting(shift);
	writer.write32(size-4);
	writer.writeString8(handlerName);
	if(writeRef)
		writer.write7BitEncoded(handlerRef);
	else if(handlerName.empty())
		writer.write8(0);
	stream.resetWriting(size+shift);

	DUMP_INTERN(stream.data() + 4, stream.size() - 4, "To ", address.toString()," server");
	Exception ex;
	EXCEPTION_TO_LOG(TCPClient::send(ex,stream.data(),stream.size()),"Server ",address.toString());
}


UInt32 ServerConnection::onReception(const shared_ptr<Buffer<UInt8>>& pData) {
	if (_size == 0 && pData->size() < 4)
		return pData->size();

	MemoryReader reader(pData->data(), pData->size());
	if(_size==0)
		_size = reader.read32();
	if (reader.available() < _size)
		return pData->size();

	UInt32 rest = reader.available() - _size;
	reader.shrink(_size);
	
	DUMP_INTERN(reader, "From ", address.toString(), " server");

	string handler;
	UInt8 handlerSize = reader.read8();
	if(handlerSize)
		_receivingRefs[_receivingRefs.size() + 1] = reader.readRaw(handlerSize, handler);
	else {
		UInt32 ref = reader.read7BitEncoded();
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
		reader.readString((string&)host);

		if(host.empty())
			((string&)host) = address.host().toString();
		UInt8 ports = reader.read8();
		string protocol;
		while(ports>0) {
			reader.readString(protocol);
			_ports[protocol] = reader.read16();
			--ports;
		}
		while(reader.available()) {
			string key,value;
			reader.readString(key);
			reader.readString(value);
			setRaw(key,value);
		}
		if(!_connected) {
			_connected=true;
			_serversHandler.connection(*this);
			_handler.connection(*this);
		}
	} else
		_handler.message(*this,handler,reader);

	return rest;
}

void ServerConnection::onDisconnection(){
	_sendingRefs.clear();
	_receivingRefs.clear();
	if(_connected) {
		_connected=false;
		bool autoDelete = _serversHandler.disconnection(*this);
		_handler.disconnection(*this, _error);
		_ports.clear();
		((string&)host).clear();
		if(autoDelete)
			delete this;
	}
	_error.clear();
}
