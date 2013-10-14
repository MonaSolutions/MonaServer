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
#include "Mona/SocketManager.h"
#include "Mona/UDPSender.h"
#include "Mona/Util.h"
#include "Poco/Buffer.h"
#include "Poco/Net/SocketAddress.h"

namespace Mona {

class RelaySender : public UDPSender {
public:
	RelaySender(SocketHandler<Poco::Net::DatagramSocket>& handler) : _pBuffer(NULL),_size(0),UDPSender(handler,true) {}
	virtual ~RelaySender(){}

	void setBuffer(Poco::SharedPtr<Poco::Buffer<Mona::UInt8> >& pBuffer,Mona::UInt32 size);
	
private:
	const Mona::UInt8*	begin(bool displaying=false);
	Mona::UInt32		size(bool displaying=false);

	Poco::SharedPtr<Poco::Buffer<Mona::UInt8> >	_pBuffer;
	Mona::UInt32								_size;
};

inline void RelaySender::setBuffer(Poco::SharedPtr<Poco::Buffer<Mona::UInt8> >& pBuffer,Mona::UInt32 size) {
	_pBuffer = pBuffer;_size = size;
}

inline const Mona::UInt8* RelaySender::begin(bool displaying) {
	return _pBuffer.isNull() ? NULL : _pBuffer->begin();
}
inline Mona::UInt32 RelaySender::size(bool displaying) {
	return _size;
}


class Relay;
class RelaySocket : public SocketHandler<Poco::Net::DatagramSocket> {
public:
	RelaySocket(Mona::UInt16 port,const SocketManager& manager);
	virtual ~RelaySocket();

	typedef std::map<Poco::Net::SocketAddress,Relay*,Util::AddressComparator> Addresses;

	const Mona::UInt16 port;

	Relay&			createRelay(const Peer& peer1,const Poco::Net::SocketAddress& address1,const Peer& peer2,const Poco::Net::SocketAddress& address2,Mona::UInt16 timeout);
	Mona::UInt32	releaseRelay(Relay& relay);

	const Addresses	addresses;
private:
	void	onError(const std::string& error);
	void	onReadable();

	
	Poco::FastMutex	_mutex;
};

class RelayServer {
public:
	RelayServer(PoolThreads& poolThreads,Mona::UInt32 bufferSize=0);
	virtual ~RelayServer();
	
	Mona::UInt16 add(const Peer& peer1,const Poco::Net::SocketAddress& address1,const Peer& peer2,const Poco::Net::SocketAddress& address2,Mona::UInt16 timeout=120) const;
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
		   return a->getSocket()->address().port() < b->getSocket()->address().port();
	   }
	};

	SocketManager											_manager;
	mutable Poco::FastMutex									_mutex;

	mutable std::set<Relay*>								_relays;
	mutable std::map<const Peer*,std::set<Relay*> >		_peers;
	mutable std::set<RelaySocket*,Compare>							_sockets;
};


} // namespace Mona
