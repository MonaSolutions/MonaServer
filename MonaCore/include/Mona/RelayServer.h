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
#include "Mona/Peer.h"
#include "Mona/DatagramSocket.h"
#include "Mona/SocketManager.h"

namespace Mona {

class Relay;
class RelaySocket : protected DatagramSocket, virtual Object {
public:
	RelaySocket(const SocketManager& manager);


	typedef std::map<SocketAddress,Relay*> Addresses;

	const UInt16 port;

	bool	load(Exception& ex, const SocketAddress& address);
	Relay&	createRelay(const Peer& peer1,const SocketAddress& address1,const Peer& peer2,const SocketAddress& address2,UInt16 timeout);
	UInt32	releaseRelay(Relay& relay);

	const Addresses	addresses;
private:
	void	onError(const std::string& error);
	void	onReadable(Exception& ex);

	std::mutex	_mutex;
};

class RelayServer : virtual Object {
public:
	RelayServer(PoolThreads& poolThreads,UInt32 bufferSize=0);
	virtual ~RelayServer();
	
	UInt16 add(const Peer& peer1,const SocketAddress& address1,const Peer& peer2,const SocketAddress& address2,UInt16 timeout=120) const;
	void remove(const Peer& peer) const;

	void manage() const;

	void start();
	void stop();

private:
	void releaseRelay(Relay& relay) const;
	void removePeerRelay(const Peer& peer,Relay& relay) const;
	void sendingError();

	void clear();

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
