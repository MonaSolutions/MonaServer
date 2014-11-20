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
#include "Mona/UDPSocket.h"
#include "Mona/SocketManager.h"
#include <set>
#include <vector>

namespace Mona {

class Relay;
class RelaySocket : public UDPSocket, public virtual Object {
public:
	RelaySocket(const SocketManager& manager,UInt16 port);
	~RelaySocket();

	bool	relay(const SocketAddress& address1,const SocketAddress& address2,UInt16 timeout);
	bool	manage();

	const UInt16		port;
	
private:
	OnError::Type	onError; // executed in a parallel thread!
	OnPacket::Type	onPacket; // executed in a parallel thread!
	
	mutable std::mutex				_mutex;
	std::map<SocketAddress,Relay*>	_relayByAddress;
	std::vector<Relay*>				_relays;
	
};

class RelayServer : public virtual Object {
public:
	RelayServer(const PoolBuffers& poolBuffers,PoolThreads& poolThreads,UInt32 bufferSize=0);
	~RelayServer();
	
	UInt16	relay(const SocketAddress& address1,const SocketAddress& address2,UInt16 timeout) const;

	void	manage() const;

	bool start(Exception& ex) { return _manager.start(ex); }
	void stop();

private:

	struct Compare {
	   bool operator()(RelaySocket* a,RelaySocket* b) const {
		   return a->port < b->port;
	   }
	};

	RelaySocket* createSocket(std::set<RelaySocket*, Compare>::const_iterator& it, UInt16 port) const;

	SocketManager										_manager;

	mutable std::set<RelaySocket*,Compare>				_sockets;
};


} // namespace Mona
