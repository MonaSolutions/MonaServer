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


#include "Broadcaster.h"
#include "Mona/TCPServer.h"



namespace ServerEvents {
	struct OnConnection : Mona::Event<void(ServerConnection&)> {};
};

class Servers : public Broadcaster,
		public ServerEvents::OnConnection,
		public ServerEvents::OnMessage,
		public ServerEvents::OnDisconnection {
public:
	Servers(const Mona::SocketManager& manager);
	virtual ~Servers();
	
	void start(const Mona::Parameters& parameters);
	void manage(const Mona::Parameters& parameters);
	void stop();

	Broadcaster			initiators;
	Broadcaster			targets;


private:
	Mona::UInt32		flush(const std::string& handler);
	Mona::UInt32		flush(Mona::UInt32 handlerRef);

	Mona::TCPServer::OnConnection::Type		onConnection;
	Mona::TCPServer::OnError::Type			onError;

	ServerConnection::OnHello::Type			onServerHello;
	ServerConnection::OnDisconnection::Type	onServerDisconnection;

	Mona::UInt8								_manageTimes;
	bool									_running;

	std::set<ServerConnection*>				_targets;
	std::set<ServerConnection*>				_clients;

	Mona::TCPServer							_server;
};

