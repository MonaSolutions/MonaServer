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


ServerManager::ServerManager(Server& server):_server(server),Task(server),Startable("ServerManager"){
}

void ServerManager::run(Exception& ex) {
	do {
		waitHandle();
	} while (sleep(2000) != STOP);
}

void ServerManager::handle(Exception& ex) {
	_server.manage();
	_server.relay.manage();
}

Server::Server(UInt32 bufferSize,UInt16 threads) : Startable("Server"),Handler(bufferSize,threads),_protocols(*this),_manager(*this) {

}

Server::~Server() {
	stop();
}

bool Server::start(const ServerParams& params) {
	if(running()) {
		ERROR("Server is already running, call stop method before");
		return false;
	}
	(ServerParams&)this->params = params;
	Exception ex;
	bool result;
	EXCEPTION_TO_LOG(result = Startable::start(ex, params.threadPriority), "Server");
	return result;
}

void Server::run(Exception& exc) {
	Exception ex;
	try {
		TaskHandler::start();

		Exception exWarn;
		if (((SocketManager&)sockets).start(exWarn) && ((RelayServer&)relay).start(exWarn)) {
			if (exWarn)
				WARN(exWarn.error());

			_pSessions.reset(new Sessions());

			_protocols.load(*_pSessions);

			onStart();

			if (!_manager.start(exWarn, Startable::PRIORITY_LOW))
				ex.set(exWarn);
			else if (exWarn)
				WARN(exWarn.error());
			while (!ex && sleep() != STOP)
				giveHandle(ex);
		} else
			ex.set(exWarn);
		if (ex)
			FATAL("Server, %s", ex.error().c_str());
		
	} catch (exception& ex) {
		FATAL("Server, ",ex.what());
	} catch (...) {
		FATAL("Server, unknown error");
	}

	// terminate the tasks
	TaskHandler::stop();
	// terminate messages reception
	((SocketManager&)sockets).stop();
	((RelayServer&)relay).stop();
	// terminate manager
	_manager.stop();

	// clean sessions, and send died message if need
	if (_pSessions)
		_pSessions.reset();

	// stop receiving and sending engine (it waits the end of sending last session messages)
	poolThreads.join();

	_protocols.unload();

	// release memory
	poolBuffers.clear();

	onStop();
	NOTE("Server stopped");
}

void Server::manage() {
	_protocols.manage();
	if (_pSessions)
		_pSessions->manage();
}



} // namespace Mona
