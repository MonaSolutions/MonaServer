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



Servers::Servers(Mona::UInt16 port, const SocketManager& manager, const string& targets) : Broadcaster(manager.poolBuffers),targets(manager.poolBuffers),initiators(manager.poolBuffers),TCPServer(manager), _address(IPAddress::Wildcard(),port), _manageTimes(1) {
	
	
	onServerHello = [this](ServerConnection& server) {
		_connections.emplace(&server);
		if(server.isTarget)
			this->targets._connections.emplace(&server);
		else
			initiators._connections.emplace(&server);
		OnConnection::raise(server);
	};
	
	onServerMessage = [this](ServerConnection& server,const std::string& handler,Mona::PacketReader& message) {
		OnMessage::raise(server,handler,message);
	};

	onServerGoodbye = [this](ServerConnection& server) {
		if (_connections.erase(&server) == 0) // not connected
			return;
		if(server.isTarget)
			this->targets._connections.erase(&server);
		else
			initiators._connections.erase(&server);
		 OnDisconnection::raise(server);
	};


	onServerDisconnection = [this](ServerConnection& server) { 
		if (_clients.erase(&server) > 0)
			delete &server;
	};

	
	
	if (_address)
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
			ServerConnection& server(**_targets.emplace(new ServerConnection(manager,address)).first);
			if (!query.empty())
				Util::UnpackQuery(query, server);
			server.OnHello::addListener(onServerHello);
			server.OnMessage::addListener(onServerMessage);
			server.OnGoodbye::addListener(onServerGoodbye);
			server.OnDisconnection::addListener(onServerDisconnection);
		}
	}
}

Servers::~Servers() {
	stop();
	for (ServerConnection* pClient : _clients)
		pClient->close(); // will be deleted by onDisconnection!
	for (ServerConnection* pClient : _clients)
		delete pClient; // client remaining? (no onDisconnection event!)
	for (ServerConnection* pTarget : _targets)
		delete pTarget;
}

void Servers::manage() {
	if(_targets.empty() || (--_manageTimes)!=0)
		return;
	_manageTimes = 5; // every 10 sec
	for (ServerConnection* pTarget : _targets)
		pTarget->connect(host, ports);
}

void Servers::start() {
	if (!_address)
		return;
	Exception ex;
	EXCEPTION_TO_LOG(TCPServer::start(ex, _address), "Servers");
}


void Servers::stop() {
	TCPServer::stop();
	_connections.clear();
	targets._connections.clear();
	initiators._connections.clear();
	for (ServerConnection* pTarget : _targets)
		pTarget->disconnect();
	for (ServerConnection* pClient : _clients)
		pClient->disconnect();
}

void Servers::onConnection(Exception& ex, const SocketAddress& peerAddress, SocketFile& file) {
	ServerConnection& server(**_clients.emplace(new ServerConnection(peerAddress, file, manager())).first);
	server.OnHello::addListener(onServerHello);
	server.OnMessage::addListener(onServerMessage);
	server.OnGoodbye::addListener(onServerGoodbye);
	server.OnDisconnection::addListener(onServerDisconnection);
	server.sendHello(host,ports);
}

