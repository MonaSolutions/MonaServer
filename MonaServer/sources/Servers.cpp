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



Servers::Servers(const SocketManager& manager) :
	_running(false),Broadcaster(manager.poolBuffers),targets(manager.poolBuffers),initiators(manager.poolBuffers),_server(manager), _manageTimes(1) {
}

Servers::~Servers() {
	stop();
}

void Servers::manage(const Parameters& parameters) {
	if (!_running)
		return;
	if(_targets.empty() || (--_manageTimes)!=0)
		return;
	_manageTimes = 8; // every 16 sec
	for (ServerConnection* pTarget : _targets)
		pTarget->connect(parameters);
}

void Servers::start(const Parameters& parameters) {
	if (_running)
		stop();

	_running = true;

	onServerHello = [this,&parameters](ServerConnection& server) {
		if (!_connections.emplace(&server).second) {
			ERROR("Server ", server.address.toString(), " already connected");
			return;
		}
		if(server.isTarget)
			this->targets._connections.emplace(&server);
		else
			initiators._connections.emplace(&server);
		NOTE("Connection established with ", server.address.toString(), " server");
		if (!server.isTarget)
			server.sendHello(parameters);
		OnConnection::raise(server);
	};

	onServerDisconnection = [this](const Exception& ex,ServerConnection& server) { 
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
	parameters.getString("servers.targets",targets);
	
	String::ForEach forEach( [this](UInt32 index, const char* target) {
		const char* query = strchr(target,'?');
		if (query) {
			*(char*)query = '\0';
			++query;
		}
		SocketAddress address;
		Exception ex;
		bool success;
		EXCEPTION_TO_LOG(success=address.set(ex, target), "Servers ", target, " target");
		if (success) {
			ServerConnection& server(**_targets.emplace(new ServerConnection(address,_server.manager(),query)).first);
			server.OnHello::subscribe(onServerHello);
			server.OnMessage::subscribe(*this);
			server.OnDisconnection::subscribe(onServerDisconnection);
		}
		return true;
	});
	String::Split(targets, ";", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);


	onConnection = [this](Exception& ex, const SocketAddress& peerAddress, SocketFile& file) {
		ServerConnection& server(**_clients.emplace(new ServerConnection(peerAddress, file, _server.manager())).first);
		server.OnHello::subscribe(onServerHello);
		server.OnMessage::subscribe(*this);
		server.OnDisconnection::subscribe(onServerDisconnection);
	};

	onError = [this](const Mona::Exception& ex) { WARN("Servers, ", ex.error()); };

	_server.OnError::subscribe(onError);
	_server.OnConnection::subscribe(onConnection);


	SocketAddress address(IPAddress::Wildcard(), parameters.getNumber<UInt16>("servers.port"));
	if (!address)
		return;
	Exception ex;
	bool success(false);
	EXCEPTION_TO_LOG(success=_server.start(ex, address), "Servers");
	if (success)
		NOTE("Servers incoming connection on ", address.toString(), " started");
}

void Servers::stop() {
	if (!_running)
		return;
	_running = false;
	if (_server.running()) {
		NOTE("Servers incoming connection on ",_server.address().toString(), " stopped");
		_server.stop();
	}
	_connections.clear();
	targets._connections.clear();
	initiators._connections.clear();
	for (ServerConnection* pClient : _clients)
		delete pClient;
	_clients.clear();
	for (ServerConnection* pTarget : _targets)
		delete pTarget;
	_targets.clear();

	_server.OnError::unsubscribe(onError);
	_server.OnConnection::unsubscribe(onConnection);
}
