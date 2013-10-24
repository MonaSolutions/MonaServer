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
#include "Mona/Logs.h"
#include "Poco/Format.h"

using namespace std;
using namespace Mona;
using namespace Poco;
using namespace Poco::Net;


ServerConnection::ServerConnection(const string& target,const SocketManager& manager,ServerHandler& handler,ServersHandler& serversHandler) : address(target),_size(0),_handler(handler),TCPClient(manager),_connected(false),_serversHandler(serversHandler),isTarget(true) {
	size_t found = target.find("?");
	if(found!=string::npos) {
		Mona::Util::UnpackQuery(target.substr(found+1),*this);
		((string&)address).assign(target.substr(0,found));
	}
}

ServerConnection::ServerConnection(const StreamSocket& socket,const SocketManager& manager,ServerHandler& handler,ServersHandler& serversHandler) : address(socket.peerAddress().toString()),_size(0),_handler(handler),TCPClient(socket,manager),_connected(false),_serversHandler(serversHandler),isTarget(false) {
	sendPublicAddress();
}


ServerConnection::~ServerConnection() {
	
}

Mona::UInt16 ServerConnection::port(const string& protocol) {
	map<string, Mona::UInt16>::const_iterator it = _ports.find(protocol);
	if(it==_ports.end())
		return 0;
	return it->second;
}

void ServerConnection::sendPublicAddress() {
	ServerMessage message;
	Mona::BinaryWriter& writer = message.writer;
	writer << _handler.host();
	writer.write8(_handler.ports().size());
	map<string, Mona::UInt16>::const_iterator it0;
	for(it0=_handler.ports().begin();it0!=_handler.ports().end();++it0) {
		writer << it0->first;
		writer.write16(it0->second);
	}
	for(auto& it: *this) {
		writer << it.first;
		writer << it.second;
	}
	send("",message);
}

void ServerConnection::connect() {
	if(connected())
		return;
	INFO("Attempt to join ",address," server")
	TCPClient::connect(SocketAddress(address));
	sendPublicAddress();
}

void ServerConnection::send(const string& handler,ServerMessage& message) {
	string handlerName(handler);
	if(handlerName.size()>255) {
		handlerName.resize(255);
		WARN("The server handler '",handlerName,"' truncated for 255 char (maximum acceptable size)")
	}

	// Search handler!
	Mona::UInt32 handlerRef = 0;
	bool   writeRef = false;
	if(!handlerName.empty()) {
		map<string, Mona::UInt32>::iterator it = _sendingRefs.lower_bound(handlerName);
		if(it!=_sendingRefs.end() && it->first==handlerName) {
			handlerRef = it->second;
			handlerName.clear();
			writeRef = true;
		} else {
			if(it!=_sendingRefs.begin())
				--it;
			handlerRef = _sendingRefs.size()+1;
			_sendingRefs.insert(it, pair<string, Mona::UInt32>(handlerName, handlerRef));
		}
	}

	Mona::UInt16 shift = handlerName.empty() ? Mona::Util::Get7BitValueSize(handlerRef) : handlerName.size();
	shift = 300-(shift+5);

	BinaryStream& stream = message.stream;
	Mona::BinaryWriter& writer = message.writer;
	stream.resetReading(shift);
	Mona::UInt32 size = stream.size();
	stream.resetWriting(shift);
	writer.write32(size-4);
	writer.writeString8(handlerName);
	if(writeRef)
		writer.write7BitEncoded(handlerRef);
	else if(handlerName.empty())
		writer.write8(0);
	stream.resetWriting(size+shift);

	DUMP_INTERN(stream.data()+4,stream.size()-4,format("To %s server",address).c_str());
	TCPClient::send(stream.data(),stream.size());
}


Mona::UInt32 ServerConnection::onReception(const Mona::UInt8* data, Mona::UInt32 size) {
	if(_size==0 && size<4)
		return size;

	MemoryReader reader(data,size);
	if(_size==0)
		_size = reader.read32();
	if(reader.available()<_size)
		return reader.available();

	size = reader.available()-_size;
	reader.shrink(_size);
	
	DUMP_INTERN(reader,format("From %s server",address).c_str());

	string handler;
	Mona::UInt8 handlerSize = reader.read8();
	if(handlerSize)
		_receivingRefs[_receivingRefs.size() + 1] = reader.readRaw(handlerSize, handler);
	else {
		Mona::UInt32 ref = reader.read7BitEncoded();
		if(ref>0) {
			map<Mona::UInt32, string>::const_iterator it = _receivingRefs.find(ref);
			if(it==_receivingRefs.end())
				ERROR("Impossible to find the ",ref," handler reference for the server ",peerAddress().toString())
			else
				handler.assign(it->second);
		}
	}

	_size=0;
	if(handler.empty()) {
		reader >> ((string&)host);
		if(host.empty())
			((string&)host) = peerAddress().host().toString();
		Mona::UInt8 ports = reader.read8();
		string protocol;
		while(ports>0) {
			reader >> protocol;
			_ports[protocol] = reader.read16();
			--ports;
		}
		while(reader.available()) {
			string key,value;
			reader >> key;
			reader >> value;
			setRaw(key,value);
		}
		if(!_connected) {
			_connected=true;
			_serversHandler.connection(*this);
			_handler.connection(*this);
		}
	} else
		_handler.message(*this,handler,reader);

	return size;
}

void ServerConnection::onDisconnection(){
	_sendingRefs.clear();
	_receivingRefs.clear();
	if(_connected) {
		_connected=false;
		bool autoDelete = _serversHandler.disconnection(*this);
		_handler.disconnection(*this,error());
		_ports.clear();
		((string&)host).clear();
		if(autoDelete)
			delete this;
	}
}
