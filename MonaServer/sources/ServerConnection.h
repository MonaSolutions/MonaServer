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

#pragma once

#include "ServerMessage.h"
#include "Mona/TCPClient.h"
#include "Mona/MemoryReader.h"
#include "Mona/MapParameters.h"


class ServerConnection;
class ServerHandler {
public:
	virtual const std::string&	host()=0;
	virtual const std::map<std::string,Mona::UInt16>& ports()=0;
	virtual void connection(ServerConnection& server)=0;
	virtual void message(ServerConnection& server,const std::string& handler,Mona::MemoryReader& reader)=0;
	virtual void disconnection(const ServerConnection& server,const char* error)=0;
};

class ServersHandler {
public:
	virtual void connection(ServerConnection& server)=0;
	virtual bool disconnection(ServerConnection& server)=0;
};


class ServerConnection : private Mona::TCPClient, public Mona::MapParameters  {
public:
	ServerConnection(const std::string& target,const Mona::SocketManager& manager,ServerHandler& handler,ServersHandler& serversHandler);
	ServerConnection(const Poco::Net::StreamSocket& socket,const Mona::SocketManager& manager,ServerHandler& handler,ServersHandler& serversHandler);
	virtual ~ServerConnection();

	const std::string							host;
	const std::string							address;
	const bool									isTarget;
	Mona::UInt16								port(const std::string& protocol);

	void			connect();

	void			send(const std::string& handler,ServerMessage& message);

private:
	void			sendPublicAddress();

	Mona::UInt32	onReception(const Mona::UInt8* data,Mona::UInt32 size);
	void			onDisconnection();

	ServerHandler&						_handler;
	ServersHandler&						_serversHandler;
	std::map<std::string,Mona::UInt32>	_sendingRefs;
	std::map<Mona::UInt32,std::string>	_receivingRefs;

	Mona::UInt32						_size;
	bool								_connected;
	std::map<std::string,Mona::UInt16>	_ports;
};

