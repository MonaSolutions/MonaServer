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

#include "Mona/Mona.h"
#include "Mona/Sessions.h"
#include "Mona/Handler.h"
#include "Mona/Protocols.h"
#include "Poco/Net/SocketAddress.h"


namespace Mona {

class Server;
class ServerManager : private Task, public Startable {
public:
	ServerManager(Server& server);
	virtual ~ServerManager() {}
private:
	void run(Exception& ex);
	void handle(Exception& ex) { _server.manage(); }
	Server& _server;
};

class Server : private Gateway,protected Handler,private Startable {
	friend class ServerManager;
public:
	Server(UInt32 bufferSize=0,UInt32 threads=0);
	virtual ~Server();

	void	start() { start(params); }
	void	start(const ServerParams& params);
	void	stop() { Startable::stop(); }
	bool	running() { Startable::running(); }

protected:
	virtual void		manage();

private:
	virtual void    onStart(){}
	virtual void    onStop(){}
	void			requestHandle() { wakeUp(); }

	void			receive(Decoding& decoded);
	void			run(Exception& ex);
	
	Session*		session(const UInt8* peerId) { return _sessions.find(peerId); }
	Session*		session(const SocketAddress& address) { return _sessions.find(address); }

	Session&		registerSession(Session* pSession) { return _sessions.add(pSession); }
	void			readable(Protocol& protocol);


	Protocols		_protocols;
	Sessions		_sessions;	
	ServerManager	_manager;
};



} // namespace Mona
