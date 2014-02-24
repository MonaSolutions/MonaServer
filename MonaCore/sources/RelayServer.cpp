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

#include "Mona/RelayServer.h"
#include "Mona/UDPSender.h"

using namespace std;


namespace Mona {

class Relay : virtual Object {
public:
	Relay(const Peer& peer1,const SocketAddress& address1,const Peer& peer2,const SocketAddress& address2,RelaySocket& socket,UInt16 timeout):
	 socket(socket),timeout(timeout*1000),peer1(peer1),address1(address1),peer2(peer2),address2(address2),received(false) {}
	const Peer&							peer1;
	const SocketAddress&				address1;
	const Peer&							peer2;
	const SocketAddress&				address2;
	bool								received;
	Time								lastTime;
	Int64								timeout;
	RelaySocket&						socket;
};


RelaySocket::RelaySocket(const SocketManager& manager,UInt16 port) : UDPSocket(manager),port(port) {
	
}

Relay& RelaySocket::createRelay(const Peer& peer1,const SocketAddress& address1,const Peer& peer2,const SocketAddress& address2,UInt16 timeout) {
	lock_guard<mutex> lock(_mutex);
	Addresses::iterator it1 = ((Addresses&)addresses).emplace(address1,nullptr).first;
	Addresses::iterator it2 = ((Addresses&)addresses).emplace(address2, nullptr).first;
	return *(it1->second = it2->second = new Relay(peer1,it1->first,peer2,it2->first,*this,timeout));
}

UInt32 RelaySocket::releaseRelay(Relay& relay) {
	lock_guard<mutex> lock(_mutex);
	//remove turnPeers
	if(relay.received) {
		INFO("Turn finishing from ", relay.address1.toString(), " to ", relay.address2.toString(), " on ", port, " relayed port")
		((Peer&)relay.peer1).turnPeers().remove((Peer&)relay.peer2);
		((Peer&)relay.peer2).turnPeers().remove((Peer&)relay.peer1);
	}
	((Addresses&)addresses).erase(relay.address1);
	((Addresses&)addresses).erase(relay.address2);
	delete &relay;
	return addresses.size();
}

// executed in a parallel thread!
void RelaySocket::onReception(const UInt8* data, UInt32 size, const SocketAddress& address) {
	lock_guard<mutex> lock(_mutex);
	Addresses::const_iterator itAddress = addresses.find(address);
	if(itAddress==addresses.end()) {
		DEBUG("Unknown relay ", address.toString()," address")
		return;
	}

	Relay& relay = *itAddress->second;
	if(!relay.received) {
		relay.received=true;
		INFO("Turn starting from ",relay.address1.toString()," to ",relay.address2.toString()," on ",port," relayed port")
		((Peer&)relay.peer1).turnPeers().add((Peer&)relay.peer2);
		((Peer&)relay.peer2).turnPeers().add((Peer&)relay.peer1);
	}

	SocketAddress destinator(relay.address1 == address ? relay.address2 : relay.address1);

	DUMP(data, size, "Request from ", address.toString())
	Exception ex;
	send(ex, data, size, destinator);
	if (ex) {
		WARN("Relay packet (size=", size, ") from ", address.toString(), " to ", destinator.toString()," on ",port,", ",ex.error())
	} else {
		relay.lastTime.update();
		DEBUG("Relay packet (size=", size, ") from ", address.toString(), " to ", destinator.toString()," on ",port)
	}
}



RelayServer::RelayServer(const PoolBuffers& poolBuffers,PoolThreads& poolThreads,UInt32 bufferSize) : _manager(poolBuffers,poolThreads,bufferSize,"RelayServer")  {
	
}

RelayServer::~RelayServer() {
	stop();
}

void RelayServer::stop() {
	{
		lock_guard<mutex> lock(_mutex);
		set<Relay*>::iterator it;
		for(it=_relays.begin();it!=_relays.end();++it) {
			Relay& relay(**it);
			// remove Relay of peers
			removePeerRelay(relay.peer1,relay);
			removePeerRelay(relay.peer2,relay);
			releaseRelay(relay);
		}
		_relays.clear();
	}
	_manager.stop();
};

UInt16 RelayServer::add(const Peer& peer1,const SocketAddress& address1,const Peer& peer2,const SocketAddress& address2,UInt16 timeout) const {
	if(peer1 == peer2) {
		ERROR("Relay useless between the two same peer")
		return 0;
	}

	if(address1==address2) {
		ERROR("Relay useless between the two same addresses")
		return 0;
	}

	set<RelaySocket*,Compare>::iterator it;
	RelaySocket* pSocket=NULL;
	UInt16 port=59999; // TODO make configurable?
	for(it=_sockets.begin();it!=_sockets.end();++it) {
		port = (*it)->port;
		RelaySocket::Addresses::const_iterator itRelay = (*it)->addresses.find(address1);
		if(itRelay!=(*it)->addresses.end()) {
			if((itRelay->second->address1==address1 && itRelay->second->address2==address2) || (itRelay->second->address1==address2 && itRelay->second->address2==address1))
				return port; // this relay exists already!
		} else if((itRelay = (*it)->addresses.find(address2)) == (*it)->addresses.end()) {
			pSocket = *it; // can use the same socket!
			break;
		}
	}

	while(!pSocket) { // if no found, means that we have to create a new socket
		if(port==0xFFFF) {
			ERROR("No more port available for a new relay");
			return 0;
		}
		++port;

		SocketAddress address(IPAddress::Wildcard(), port);

		Exception ex;
		pSocket = new RelaySocket(_manager,port);
		if (!pSocket->bind(ex, address)) {
			delete pSocket;
			DEBUG("Turn listening impossible on ", port, " port, ", ex.error())
			continue;
		}

		if (ex)
			WARN("Turn server listening on ", port," port, ",ex.error())
		else
			INFO("Turn server listening on ", port," port")

		if (_sockets.empty())
			it = _sockets.begin();
		_sockets.emplace_hint(it, pSocket);
	}

	Relay* pRelay = &pSocket->createRelay(peer1,address1,peer2,address2,timeout);

	_relays.insert(pRelay);
	_peers[&peer1].insert(pRelay);
	_peers[&peer2].insert(pRelay);

	((Peer&)peer1).relayable = true;
	((Peer&)peer2).relayable = true;

	DEBUG("Relay between ", address1.toString(), " and ", address2.toString(), " on ,",port," port")

	return port;
}

void RelayServer::remove(const Peer& peer) const {
	lock_guard<mutex> lock(_mutex);
	map<const Peer*,set<Relay*> >::iterator it = _peers.find(&peer);
	if(it==_peers.end())
		return;
	
	set<Relay*>::iterator it2;
	for(it2=it->second.begin();it2!=it->second.end();++it2) {
		Relay& relay(**it2);
		// remove Relay of remote peer
		removePeerRelay(&relay.peer1==&peer ? relay.peer2 : relay.peer1,relay);
		// remove relay
		_relays.erase(&relay);
		releaseRelay(relay);
	}

	_peers.erase(it);
	((Peer&)peer).relayable = false;
}

void RelayServer::manage() const {
	lock_guard<mutex> lock(_mutex);
	set<Relay*>::iterator it=_relays.begin();
	while(it!=_relays.end()) {
		Relay& relay(**it);
		if(relay.lastTime.isElapsed(relay.timeout)) {
			// remove Relay of peers
			removePeerRelay(relay.peer1,relay);
			removePeerRelay(relay.peer2,relay);
			// remove relay
			_relays.erase(it++);
			releaseRelay(relay);
			continue;
		}
		++it;
	}
}

void RelayServer::releaseRelay(Relay& relay) const {
	// remove addresses (and socket if necessary)
	set<RelaySocket*,Compare>::iterator it = _sockets.find(&relay.socket);
	if(it!=_sockets.end()) {
		RelaySocket& socket(**it);
		if(socket.releaseRelay(relay)==0) {
			socket.close();
			socket.timeout.update();
			delete *it;
			_sockets.erase(it);
		}
	}
}


void RelayServer::removePeerRelay(const Peer& peer,Relay& relay) const {
	map<const Peer*,set<Relay*> >::iterator it = _peers.find(&peer);
	if(it!=_peers.end()) {
		it->second.erase(&relay);
		if(it->second.empty()) {
			((Peer&)peer).relayable = false;
			_peers.erase(it);
		}
	}
}


} // namespace Mona
