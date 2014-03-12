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
#include "Mona/Peer.h"
#include "Mona/UDPSocket.h"
#include "Mona/SocketManager.h"
#include "Mona/Logs.h"

namespace Mona {

class Relay;
class RelaySocket : public UDPSocket, virtual Object {
public:
	RelaySocket(const SocketManager& manager,UInt16 port);

	typedef std::map<SocketAddress,Relay*> Addresses;

	Relay&	createRelay(const Peer& peer1,const SocketAddress& address1,const Peer& peer2,const SocketAddress& address2,UInt16 timeout);
	UInt32	releaseRelay(Relay& relay);

	const UInt16	port;
	const Addresses	addresses;
	Time			timeout;
private:
	// executed in a parallel thread!
	void	onReception(PoolBuffer& pBuffer, const SocketAddress& address);
	// executed in a parallel thread!
	void	onError(const std::string& error) { DEBUG("Relay socket ", port, ", error");} 
	
	std::mutex	_mutex;
};

class RelayServer : virtual Object {
public:
	RelayServer(const PoolBuffers& poolBuffers,PoolThreads& poolThreads,UInt32 bufferSize=0);
	virtual ~RelayServer();
	
	UInt16 add(const Peer& peer1,const SocketAddress& address1,const Peer& peer2,const SocketAddress& address2,UInt16 timeout=120) const;
	void remove(const Peer& peer) const;

	void manage() const;

	bool start(Exception& ex) { return _manager.start(ex); }
	void stop();

private:
	void releaseRelay(Relay& relay) const;
	void removePeerRelay(const Peer& peer,Relay& relay) const;
	void sendingError();

	struct Compare {
	   bool operator()(RelaySocket* a,RelaySocket* b) const {
		   return a->port < b->port;
	   }
	};

	SocketManager										_manager;
	mutable std::mutex									_mutex;

	mutable std::set<Relay*>							_relays;
	mutable std::map<const Peer*,std::set<Relay*> >		_peers;
	mutable std::set<RelaySocket*,Compare>				_sockets;
};


} // namespace Mona
