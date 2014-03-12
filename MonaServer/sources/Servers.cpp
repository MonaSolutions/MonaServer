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

#include "Servers.h"
#include "Mona/Logs.h"


using namespace std;
using namespace Mona;



Servers::Servers(Mona::UInt16 port, ServerHandler& handler, const SocketManager& manager, const string& targets) : Broadcaster(manager.poolBuffers),targets(manager.poolBuffers),initiators(manager.poolBuffers),TCPServer(manager), _port(port), _handler(handler), _manageTimes(1) {
	if (port > 0)
		NOTE("Servers incoming connection enabled on port ", port)
	else if (!targets.empty())
		NOTE("Servers incoming connection disabled (servers.port==0)")

	vector<string> values;
	String::Split(targets, ";", values, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
	for (string& target : values) {
		size_t found = target.find("?");
		string query;
		if (found != string::npos) {
			query = target.substr(found + 1);
			target = target.substr(0, found);
		}
		SocketAddress address;
		Exception ex;
		bool success;
		EXCEPTION_TO_LOG(success=address.set(ex, target), "Servers ", target, " target");
		if (success) {
			auto it = _targets.emplace(new ServerConnection(manager, _handler, *this,address));
			if (!query.empty())
				Util::UnpackQuery(query, **it.first);
		}
	}
}

Servers::~Servers() {
	stop();
	for (ServerConnection* pTarget : _targets)
		delete pTarget;
}

void Servers::manage() {
	if(_targets.empty() || (--_manageTimes)!=0)
		return;
	_manageTimes = 5; // every 10 sec
	for (ServerConnection* pTarget : _targets)
		pTarget->connect();
}

void Servers::start() {
	if (_port == 0)
		return;
	Exception ex;
	SocketAddress address;
	bool success = false;
	EXCEPTION_TO_LOG(success=address.set(ex, "0.0.0.0", _port), "Servers");
	if (success)
		EXCEPTION_TO_LOG(TCPServer::start(ex, address), "Servers");
}


void Servers::stop() {
	TCPServer::stop();
	_connections.clear();
	targets._connections.clear();
	initiators._connections.clear();
	while(!_clients.empty())
		(*_clients.begin())->disconnect();
}

void Servers::onConnectionRequest(Exception& ex) { 
	ServerConnection* pServer = acceptClient<ServerConnection>(ex, _handler, (ServersHandler&)*this);
	if (!pServer)
		return;
	_clients.emplace(pServer);
}

void Servers::connection(ServerConnection& server) {
	_connections.emplace(&server);
	if(server.isTarget)
		targets._connections.emplace(&server);
	else
		initiators._connections.emplace(&server);

    _handler.connection(server);
}

void Servers::disconnection(ServerConnection& server) {
	_clients.erase(&server);
	if (_connections.erase(&server) == 0) // not connected
		return;
	if(server.isTarget)
		targets._connections.erase(&server);
	else
		initiators._connections.erase(&server);

	_handler.disconnection(server);
}
