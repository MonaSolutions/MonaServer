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

#pragma once

#include "Mona/AMFWriter.h"
#include "Mona/PacketReader.h"
#include "Mona/TCPClient.h"
#include "Mona/MapParameters.h"
#include "Mona/TCPSender.h"
#include "Mona/Event.h"

class ServerMessage : public Mona::TCPSender,public Mona::AMFWriter {
	friend class ServerConnection;
public:
	ServerMessage(const char* handler, const Mona::PoolBuffers& poolBuffers) : _shift(295), _handler(handler), AMFWriter(poolBuffers), TCPSender("ServerMessage") {
		packet.next(300);
	}
	
private:
	std::string		_handler;
	Mona::UInt16	_shift;

	const Mona::UInt8*	data() { return packet.data()+_shift; }
	Mona::UInt32		size() { return packet.size()-_shift; }
};


class ServerConnection;
namespace ServerEvents {
	struct OnHello : Mona::Event<void(ServerConnection&)> {};
	struct OnMessage : Mona::Event<void(ServerConnection&,const std::string&,Mona::PacketReader&)> {};
	struct OnDisconnection : Mona::Event<void(Mona::Exception&,ServerConnection&)> {};
}

class ServerConnection : public Mona::MapParameters,
		public ServerEvents::OnHello,
		public ServerEvents::OnMessage,
		public ServerEvents::OnDisconnection {
public:
	// Target version
	ServerConnection(const Mona::SocketManager& manager,const Mona::SocketAddress& targetAddress);
	// Initiator version
	ServerConnection(const Mona::SocketAddress& peerAddress, Mona::SocketFile& file, const Mona::SocketManager& manager);

	~ServerConnection();

	const std::string				host;
	const Mona::SocketAddress		address;
	const bool						isTarget;
	Mona::UInt16					port(const std::string& protocol);

	const Mona::PoolBuffers&  poolBuffers() const { return _pClient->manager().poolBuffers; }

	void			connect(const std::string& host,const std::map<std::string,Mona::UInt16>& ports);
	bool			connected() { return _connected; }

	void			close();

	void			send(const std::shared_ptr<ServerMessage>& pMessage);
	void			sendHello(const std::string& host,const std::map<std::string,Mona::UInt16>& ports);
	void			reject(const char* error);

private:

	Mona::UInt32	onData(Mona::PoolBuffer& pBuffer);
	void			onDisconnection();

	Mona::TCPClient::OnError::Type			_onError;
	Mona::TCPClient::OnData::Type			_onData;
	Mona::TCPClient::OnDisconnection::Type	_onDisconnection;


	std::map<std::string,Mona::UInt32>	_sendingRefs;
	std::map<Mona::UInt32,std::string>	_receivingRefs;

	bool								_connected;
	std::map<std::string,Mona::UInt16>	_ports;
	Mona::Exception						_ex;
	std::unique_ptr<Mona::TCPClient>	_pClient;
};

