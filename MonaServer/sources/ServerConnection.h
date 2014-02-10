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

class ServerMessage : public Mona::AMFWriter {
public:
	ServerMessage(const Mona::PoolBuffers& poolBuffers) : AMFWriter(poolBuffers) { packet.next(300); }
};

class ServerConnection;
class ServerHandler {
public:
	virtual const std::string&	host()=0;
	virtual const std::map<std::string,Mona::UInt16>& ports()=0;
	virtual void connection(ServerConnection& server)=0;
	virtual void message(ServerConnection& server,const std::string& handler,Mona::PacketReader& packet)=0;
	virtual void disconnection(const ServerConnection& server)=0;
};

class ServersHandler {
public:
	virtual void connection(ServerConnection& server)=0;
	virtual void disconnection(ServerConnection& server)=0;
};


class ServerConnection : private Mona::TCPClient, public Mona::MapParameters  {
public:
	// Target version
	ServerConnection(const Mona::SocketManager& manager, ServerHandler& handler, ServersHandler& serversHandler,const Mona::SocketAddress& targetAddress);
	// Initiator version
	ServerConnection(const Mona::SocketAddress& peerAddress, const Mona::SocketManager& manager, ServerHandler& handler, ServersHandler& serversHandler);

	const std::string				host;
	const Mona::SocketAddress		address;
	const bool						isTarget;
	Mona::UInt16					port(const std::string& protocol);

	const Mona::PoolBuffers&		poolBuffers() { return socket().poolBuffers(); }

	void			connect();
	void			disconnect() { TCPClient::disconnect(); }

	void			send(const std::string& handler,ServerMessage& message);

private:
	void			sendPublicAddress();

	void			onError(const std::string& error);
	Mona::UInt32	onReception(const Mona::UInt8* data,Mona::UInt32 size);
	void			onDisconnection();

	ServerHandler&						_handler;
	ServersHandler&						_serversHandler;
	std::map<std::string,Mona::UInt32>	_sendingRefs;
	std::map<Mona::UInt32,std::string>	_receivingRefs;

	Mona::UInt32						_size;
	bool								_connected;
	std::map<std::string,Mona::UInt16>	_ports;
	std::string							_error;
};

