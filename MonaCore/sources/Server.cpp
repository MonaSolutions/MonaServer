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

#include "Mona/Server.h"
#include "Mona/Sessions.h"


using namespace std;


namespace Mona {


ServerManager::ServerManager(Server& server):_server(server),Task(server),Startable("ServerManager") {
}

void ServerManager::run(Exception& ex) {
	do {
		waitHandle();
	} while (sleep(2000) != STOP);
}

void ServerManager::handle(Exception& ex) { _server.manage(); }

Server::Server(UInt32 socketBufferSize,UInt16 threads) : Startable("Server"),Handler(socketBufferSize,threads),_countClients(0),_protocols(*this),_manager(*this) {
	if (socketBufferSize>0)
		DEBUG("Socket Buffer size of ",socketBufferSize," bytes")
}

Server::~Server() {
	stop();
}

bool Server::start(const Parameters& parameters) {
	if(running()) {
		ERROR("Server is already running, call stop method before");
		return false;
	}

	// copy parametes on invoker parameters!
	Parameters::ForEach forEach([this](const string& key, const string& value) {
		setString(key, value);
	});
	parameters.iterate(forEach);

	Exception ex;
	bool result;
	EXCEPTION_TO_LOG(result = Startable::start(ex, Startable::PRIORITY_HIGH), "Server");
	if (result)
		TaskHandler::start();
	return result;
}

void Server::run(Exception& exc) {
	_countClients = 0;

	Exception ex;

#if !defined(_DEBUG)
	try {
#endif
		Exception exWarn;
		if (((SocketManager&)sockets).start(exWarn) && ((RelayServer&)relayer).start(exWarn)) {
			if (exWarn)
				WARN(exWarn.error());

			_pSessions.reset(new Sessions());

			_protocols.load(*_pSessions);

			onStart();

			if (!_manager.start(exWarn, Startable::PRIORITY_LOW))
				ex = exWarn;
			else if (exWarn)
				WARN(exWarn.error());
			while (!ex && sleep() != STOP)
				giveHandle(ex);
		} else
			ex = exWarn;
		if (ex)
			FATAL("Server, ", ex.error());
		
#if !defined(_DEBUG)
	} catch (exception& ex) {
		FATAL("Server, ",ex.what());
	} catch (...) {
		FATAL("Server, unknown error");
	}
#endif
	 
	// terminate the tasks (forced to do immediatly, because no more "giveHandle" is called)
	TaskHandler::stop();

	// terminate manager
	_manager.stop();

	// clean sessions, and send died message if need
	if (_pSessions)
		_pSessions.reset();

	// terminate relay server
	((RelayServer&)relayer).stop();

	// unload protocol servers (close server socket)
	_protocols.unload();

	// stop event to unload children resource (before to release sockets, threads, and buffers)
	onStop();

	// terminate sockets manager
	((SocketManager&)sockets).stop();

	// stop receiving and sending engine (it waits the end of sending last session messages)
	poolThreads.join();

	// release memory
	((PoolBuffers&)poolBuffers).clear();

	NOTE("Server stopped");
}

void Server::manage() {
	_protocols.manage();
	if (_pSessions)
		_pSessions->manage();
	if(clients.count() != _countClients)
		INFO((_countClients=clients.count())," clients");
	relayer.manage();
}



} // namespace Mona
