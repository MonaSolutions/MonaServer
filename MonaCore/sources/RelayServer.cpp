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
#include "Mona/Logs.h"

using namespace std;


namespace Mona {

class Relay : public virtual Object {
public:
	Relay(const SocketAddress& address1,const SocketAddress& address2,UInt16 timeout):
	 _timeout(timeout*1000),address1(address1),address2(address2),_received(false) {}
	const SocketAddress&				address1;
	const SocketAddress&				address2;
	bool								obsolete() const { return _lastTime.isElapsed(_timeout); }
	bool								received() const { return _received; }
	bool								receive() { _lastTime.update(); if (_received) return false; return _received = true; }
	void								setTimeout(UInt16 timeout) { _timeout = timeout; _lastTime.update(); }
private:
	Int64								_timeout;
	Time								_lastTime;
	bool								_received;
};


RelaySocket::RelaySocket(const SocketManager& manager,UInt16 port) : UDPSocket(manager),port(port) {

	// executed in a parallel thread!
	onError = [this](const Exception& ex) { DEBUG("Relay socket ", this->port, ", ", ex.error()); };

	// executed in a parallel thread!
	onPacket = [this](PoolBuffer& pBuffer, const SocketAddress& address) {
		lock_guard<mutex> lock(_mutex);
		auto it = _relayByAddress.find(address);
		if(it==_relayByAddress.end()) {
			DEBUG("Unknown relay ", address.toString()," address")
			return;
		}

		Relay& relay(*it->second);
		if(relay.receive())
			INFO("Turn starting from ",relay.address1.toString()," to ",relay.address2.toString()," on ",this->port," relayed port")

		SocketAddress destinator(relay.address1 == address ? relay.address2 : relay.address1);

		DUMP("RELAY",pBuffer->data(), pBuffer->size(), "Relayed from ", address.toString(), " to ",destinator.toString())
		Exception ex;
		send(ex, pBuffer->data(), pBuffer->size(), destinator);
		if (ex)
			WARN("Relay packet (size=", pBuffer->size(), ") from ", address.toString(), " to ", destinator.toString()," on ",this->port,", ",ex.error())
		else
			DEBUG("Relay packet (size=", pBuffer->size(), ") from ", address.toString(), " to ", destinator.toString()," on ",this->port)
	};

	OnError::subscribe(onError);
	OnPacket::subscribe(onPacket);
}

RelaySocket::~RelaySocket() {
	OnPacket::unsubscribe(onPacket);
	OnError::unsubscribe(onError);

	for (Relay* pRelay : _relays)
		delete pRelay;
}

bool RelaySocket::relay(const SocketAddress& address1,const SocketAddress& address2,UInt16 timeout) {
	lock_guard<mutex> lock(_mutex);
	auto it1 = _relayByAddress.lower_bound(address1);

	if(it1!=_relayByAddress.end() && it1->first==address1) {
		if ((it1->second->address1 == address1 && it1->second->address2 == address2) || (it1->second->address1 == address2 && it1->second->address2 == address1)) {
			it1->second->setTimeout(timeout);
			return true; // this relay exists already!
		}
		return false;
	}

	auto it2 = _relayByAddress.lower_bound(address2);
	if (it2 != _relayByAddress.end() && it2->first == address2)
		return false;

	it1 = _relayByAddress.emplace_hint(it1,address1,nullptr);
	it2 = _relayByAddress.emplace_hint(it2,address2,nullptr);
	_relays.emplace_back(new Relay(it1->first,it2->first,timeout));
	it1->second = it2->second = _relays.back();
	return true;
}

bool RelaySocket::manage() {
	lock_guard<mutex> lock(_mutex);
	auto it = _relays.begin();
	while (it != _relays.end()) {
		Relay* pRelay(*it);
		if (pRelay->obsolete()) {
			if(pRelay->received())
				INFO("Turn finishing from ", pRelay->address1.toString(), " to ", pRelay->address2.toString(), " on ", port, " relayed port")
			_relayByAddress.erase(pRelay->address1);
			_relayByAddress.erase(pRelay->address2);
			it = _relays.erase(it);
			delete pRelay;
			continue;
		}
		++it;
	}
	return !_relays.empty();
}


RelayServer::RelayServer(const PoolBuffers& poolBuffers,PoolThreads& poolThreads,UInt32 bufferSize) : _manager(poolBuffers,poolThreads,bufferSize,"RelayServer")  {
	
}

RelayServer::~RelayServer() {
	stop();
}

void RelayServer::stop() {
	for (RelaySocket* pSocket : _sockets)
		delete pSocket;
	_sockets.clear();
	_manager.stop();
};

UInt16 RelayServer::relay(const SocketAddress& address1,const SocketAddress& address2,UInt16 timeout) const {

	if(address1==address2) {
		ERROR("Relay useless between the two same addresses")
		return 0;
	}

	UInt16 port=60000; // TODO make configurable?
	
	auto it = _sockets.begin();
	while (it != _sockets.end()) {
		RelaySocket& socket(**it++);
		while (port < socket.port) {
			RelaySocket* pSocket(createSocket(it,port));
			if (pSocket) {
				if (pSocket->relay(address1, address2, timeout))
					return pSocket->port;
				delete pSocket;
			}
			++port;
		}
		if (socket.relay(address1, address2, timeout))
			return socket.port;
		++port;
	}

	while(port>=60000) {  // TODO make configurable?
		RelaySocket* pSocket(createSocket(it,port));
		if (pSocket) {
			if (pSocket->relay(address1, address2, timeout))
				return pSocket->port;
			delete pSocket;
		}
		++port;
	}

	ERROR("No more port available for a new relay");
	return 0;
}

RelaySocket* RelayServer::createSocket(set<RelaySocket*,Compare>::const_iterator& it,UInt16 port) const {
	SocketAddress address(IPAddress::Wildcard(), port);
	Exception ex;
	RelaySocket* pSocket = new RelaySocket(_manager,port);
	if (!pSocket->bind(ex, address)) {
		delete pSocket;
		DEBUG("Turn listening impossible on ", port, " port, ", ex.error())
		return NULL;
	}
	if (ex)
		WARN("Turn server listening on ", port, " port, ", ex.error())
	else
		INFO("Turn server listening on ", port, " port")
	_sockets.emplace_hint(it, pSocket);
	return pSocket;
}

void RelayServer::manage() const {
	auto it = _sockets.begin();
	while (it!=_sockets.end()) {
		if (!(*it)->manage()) {
			delete *it;
			it = _sockets.erase(it);
			continue;
		}
		++it;
	}
}


} // namespace Mona
