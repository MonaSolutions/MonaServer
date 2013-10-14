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

#include "Mona/RelayServer.h"
#include "Mona/Logs.h"
#include "Poco/Format.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

class Relay {
public:
	Relay(const Peer& peer1,const Poco::Net::SocketAddress& address1,const Peer& peer2,const Poco::Net::SocketAddress& address2,RelaySocket& socket,Mona::UInt16 timeout):
	  pSender(new RelaySender(socket)),socket(socket),timeout(timeout*1000000),peer1(peer1),address1(address1),peer2(peer2),address2(address2),received(false) {}
	const Peer&							peer1;
	const Poco::Net::SocketAddress&		address1;
	const Peer&							peer2;
	const Poco::Net::SocketAddress&		address2;
	bool								received;
	Mona::Time						lastTime;
	Mona::Int64							timeout;
	RelaySocket&						socket;
	Poco::AutoPtr<RelaySender>			pSender;
};

RelaySocket::RelaySocket(UInt16 port,const SocketManager& manager) : SocketHandler<DatagramSocket>(manager),port(port) {
	openSocket(new DatagramSocket(SocketAddress("0.0.0.0",port),true));
	INFO("Turn server listening on %hu port",port)
}

RelaySocket::~RelaySocket(){
}

Relay& RelaySocket::createRelay(const Peer& peer1,const SocketAddress& address1,const Peer& peer2,const SocketAddress& address2,UInt16 timeout) {
	ScopedLock<FastMutex> lock(_mutex);
	Addresses::iterator it1 = ((Addresses&)addresses).insert(pair<SocketAddress,Relay*>(address1,NULL)).first;
	Addresses::iterator it2 = ((Addresses&)addresses).insert(pair<SocketAddress,Relay*>(address2,NULL)).first;
	return *(it1->second = it2->second = new Relay(peer1,it1->first,peer2,it2->first,*this,timeout));
}

UInt32 RelaySocket::releaseRelay(Relay& relay) {
	ScopedLock<FastMutex> lock(_mutex);
	//remove turnPeers
	if(relay.received) {
		INFO("Turn finishing from %s to %s on %hu relayed port",relay.address1.toString().c_str(),relay.address2.toString().c_str(),getSocket()->address().port())
		((Peer&)relay.peer1).turnPeers.erase(relay.peer2.id);
		((Peer&)relay.peer2).turnPeers.erase(relay.peer1.id);
	}
	((Addresses&)addresses).erase(relay.address1);
	((Addresses&)addresses).erase(relay.address2);
	delete &relay;
	return addresses.size();
}

// executed in a parallel thread!
void RelaySocket::onError(const std::string& error) {
	DEBUG("Socket %s, %s",getSocket()->address().toString().c_str(),error.c_str());
}

// executed in a parallel thread!
void RelaySocket::onReadable() {
	UInt32 available = getSocket()->available();
	if(available==0)
		return;

	SharedPtr<Buffer<UInt8> >	pBuffer(new Buffer<UInt8>(available));
	SocketAddress address;
	available = getSocket()->receiveFrom(pBuffer->begin(),pBuffer->size(),address);

	ScopedLock<FastMutex> lock(_mutex);
	Addresses::const_iterator itAddress = addresses.find(address);
	if(itAddress==addresses.end()) {
		DEBUG("Unknown relay %s address",address.toString().c_str())
		return;
	}

	Relay& relay = *itAddress->second;
	if(!relay.received) {
		relay.received=true;
		INFO("Turn starting from %s to %s on %hu relayed port",relay.address1.toString().c_str(),relay.address2.toString().c_str(),getSocket()->address().port())
		((Peer&)relay.peer1).turnPeers[relay.peer2.id] = (Peer*)&relay.peer2;
		((Peer&)relay.peer2).turnPeers[relay.peer1.id] = (Peer*)&relay.peer1;
	}

	relay.pSender->setBuffer(pBuffer,available);
	relay.pSender->address = Util::SameAddress(relay.address1,address) ? relay.address2 : relay.address1;

	DUMP(pBuffer->begin(),pBuffer->size(),format("Request from %s",address.toString()).c_str())

	relay.lastTime = Mona::Time();
	DEBUG("Relay packet (size=%u) from %s to %s on %hu",pBuffer->size(),address.toString().c_str(),relay.pSender->address.toString().c_str(),getSocket()->address().port())
	relay.pSender = new RelaySender(relay.socket);
}



RelayServer::RelayServer(PoolThreads& poolThreads,UInt32 bufferSize) : _manager(poolThreads,bufferSize,"RelayServer")  {
	
}

RelayServer::~RelayServer() {
	stop();
}

void RelayServer::start() {
	_manager.start();
};
void RelayServer::stop() {
	clear();
	_manager.stop();
};

UInt16 RelayServer::add(const Peer& peer1,const SocketAddress& address1,const Peer& peer2,const SocketAddress& address2,UInt16 timeout) const {
	if(peer1 == peer2) {
		ERROR("Relay useless between the two same peer")
		return 0;
	}

	if(Util::SameAddress(address1,address2)) {
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
			if((Util::SameAddress(itRelay->second->address1,address1) && Util::SameAddress(itRelay->second->address2,address2)) || (Util::SameAddress(itRelay->second->address1,address2) && Util::SameAddress(itRelay->second->address2,address1)))
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
		try {
			pSocket = new RelaySocket(port,_manager);
			if(_sockets.empty())
				it = _sockets.begin();
			else
				--it;
			_sockets.insert(it,pSocket);
		} catch(Exception& ex) {
			ERROR("Turn listening impossible on port %hu, %s",port,ex.displayText().c_str())
		}
	}

	Relay* pRelay = &pSocket->createRelay(peer1,address1,peer2,address2,timeout);

	_relays.insert(pRelay);
	_peers[&peer1].insert(pRelay);
	_peers[&peer2].insert(pRelay);

	((Peer&)peer1).relayable = true;
	((Peer&)peer2).relayable = true;

	DEBUG("Relay between %s and %s on %hu port",address1.toString().c_str(),address2.toString().c_str(),port)

	return port;
}

void RelayServer::remove(const Peer& peer) const {
	ScopedLock<FastMutex>  lock(_mutex);
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
	ScopedLock<FastMutex>  lock(_mutex);
	set<Relay*>::iterator it=_relays.begin();
	while(it!=_relays.end()) {
		Relay& relay(**it);
		if(relay.lastTime.elapsed()>relay.timeout) {
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

void RelayServer::clear() {
	ScopedLock<FastMutex>  lock(_mutex);
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

void RelayServer::releaseRelay(Relay& relay) const {
	// remove addresses (and socket if necessary)
	set<RelaySocket*,Compare>::iterator it = _sockets.find(&relay.socket);
	if(it!=_sockets.end()) {
		if((*it)->releaseRelay(relay)==0) {
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
