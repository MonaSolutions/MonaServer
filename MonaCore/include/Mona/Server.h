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

#include "Mona/Mona.h"
#include "Mona/Handler.h"
#include "Mona/Protocols.h"

namespace Mona {

class Server;
class ServerManager : private Task, public Startable, public virtual Object {
public:
	ServerManager(Server& server);
	virtual ~ServerManager() {}
private:
	void run(Exception& ex);
	void handle(Exception& ex);
	Server& _server;
};

class Server : protected Handler,private Startable {
	friend class ServerManager;
public:
	Server(UInt32 socketBufferSize=0,UInt16 threads=0);
	virtual ~Server();

	bool	start() { MapParameters parameters;  return start(parameters); }// params by default
	bool	start(const Parameters& parameters);
	void	stop() { Startable::stop(); }
	bool	running() { return Startable::running(); }

protected:
	virtual void		manage();

private:
	virtual void    onStart(){}
	virtual void    onStop(){}
	void			requestHandle() { wakeUp(); }

	void			run(Exception& ex);
	
	Protocols					_protocols;
	std::unique_ptr<Sessions>	_pSessions;	
	ServerManager				_manager;
	UInt32						_countClients;
};



} // namespace Mona
