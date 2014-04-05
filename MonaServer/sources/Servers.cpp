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



Servers::Servers(const MapParameters& configs,const SocketManager& manager) : _configs(configs),Broadcaster(manager.poolBuffers),targets(manager.poolBuffers),initiators(manager.poolBuffers),_server(manager), _manageTimes(1) {


	onServerHello = [this](ServerConnection& server) {
		if (_connections.emplace(&server).second) {
			if(server.isTarget)
				this->targets._connections.emplace(&server);
			else
				initiators._connections.emplace(&server);
			NOTE("Connection etablished with ", server.address.toString(), " server");
		} else {
			Exception ex;
			OnDisconnection::raise(ex,server);
		}
		if (!server.isTarget)
			server.sendHello(_configs);
		OnConnection::raise(server);
	};

	onServerDisconnection = [this](Exception& ex,ServerConnection& server) { 
		if (_connections.erase(&server) > 0) {
			// connected
			if (server.isTarget)
				this->targets._connections.erase(&server);
			else
				initiators._connections.erase(&server);

			if (ex)
				ERROR("Disconnection from ", server.address.toString(), " server, ", ex.error())
			else
			NOTE("Disconnection from ", server.address.toString(), " server ")

			OnDisconnection::raise(ex, server);
		}

		 if (_clients.erase(&server) > 0)
			delete &server;
	};


	string targets;
	_configs.getString("servers.targets",targets);
	
	vector<string> values;
	String::Split(targets, ";", values, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
	for (string& target : values) {
		size_t found = target.find("?");
		SocketAddress address;
		Exception ex;
		bool success;
		EXCEPTION_TO_LOG(success=address.set(ex, target.substr(0, found)), "Servers ", target, " target");
		if (success) {
			ServerConnection& server(**_targets.emplace(new ServerConnection(address,_server.manager())).first);
			if (found != string::npos)
				Util::UnpackQuery(target.substr(found + 1),server.properties);
			server.OnHello::subscribe(onServerHello);
			server.OnMessage::subscribe(*this);
			server.OnDisconnection::subscribe(onServerDisconnection);
		}
	}


	onConnection = [this](Exception& ex, const SocketAddress& peerAddress, SocketFile& file) {
		ServerConnection& server(**_clients.emplace(new ServerConnection(peerAddress, file, _server.manager())).first);
		server.OnHello::subscribe(onServerHello);
		server.OnMessage::subscribe(*this);
		server.OnDisconnection::subscribe(onServerDisconnection);
	};

	onError = [this](const Mona::Exception& ex) { WARN("Servers, ", ex.error()); };

	_server.OnError::subscribe(onError);
	_server.OnConnection::subscribe(onConnection);
}

Servers::~Servers() {
	stop();
	_server.OnError::unsubscribe(onError);
	_server.OnConnection::unsubscribe(onConnection);
	for (ServerConnection* pTarget : _targets)
		delete pTarget;
}

void Servers::manage() {
	if(_targets.empty() || (--_manageTimes)!=0)
		return;
	_manageTimes = 8; // every 16 sec
	for (ServerConnection* pTarget : _targets)
		pTarget->connect(_configs);
}

void Servers::start() {
	if (_server.running())
		return;

	SocketAddress address(IPAddress::Wildcard(), _configs.getNumber<UInt16>("servers.port"));
	if (!address)
		return;
	Exception ex;
	bool success(false);
	EXCEPTION_TO_LOG(success=_server.start(ex, address), "Servers");
	if (success)
		NOTE("Servers incoming connection on ", address.toString(), " started");
}

void Servers::stop() {
	if (!_server.running())
		return;

	NOTE("Servers incoming connection on ",_server.address().toString(), " stopped");
	_server.stop();
	_connections.clear();
	targets._connections.clear();
	initiators._connections.clear();
	for (ServerConnection* pTarget : _targets)
		pTarget->close();
	for (ServerConnection* pClient : _clients)
		delete pClient;
	_clients.clear();
}
