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
#include "Mona/Exceptions.h"
#include "Mona/Group.h"
#include "Mona/Publications.h"
#include "Mona/Entities.h"
#include "Mona/Clients.h"
#include "Mona/SocketManager.h"
#include "Mona/TaskHandler.h"
#include "Mona/PoolThreads.h"
#include "Mona/ServerParams.h"
#include "Mona/FlashStream.h"
#include "Mona/RelayServer.h"

namespace Mona {

class Invoker : public Entity,public TaskHandler {
	friend class Peer; // Peer manage _clients,_clientsByName and _groups list!
public:
	// invocations
	Clients					clients;
	Entities<Group>			groups;
	Publications			publications;
	const SocketManager		sockets;
	const RelayServer		relay;
	PoolThreads				poolThreads;

	Mona::UInt32				createFlashStream(Peer& peer);
	Poco::AutoPtr<FlashStream>  getFlashStream(Mona::UInt32 id);
	void						destroyFlashStream(Mona::UInt32 id);

	Publication*			publish(Exception& ex, const std::string& name);
	void					unpublish(const std::string& name);

	Publication*			publish(Exception& ex, Peer& peer,const std::string& name);
	void					unpublish(Peer& peer,const std::string& name);
	Listener*				subscribe(Exception& ex, Peer& peer,const std::string& name,Writer& writer,double start=-2000);
	void					unsubscribe(Peer& peer,const std::string& name);

	void					addBanned(const Poco::Net::IPAddress& ip);
	void					removeBanned(const Poco::Net::IPAddress& ip);
	void					clearBannedList();
	bool					isBanned(const Poco::Net::IPAddress& ip);

	const ServerParams		params;

protected:
	Invoker(Mona::UInt32 bufferSize,Mona::UInt32 threads);
	virtual ~Invoker();

private:
	Publications::Iterator	createPublication(const std::string& name);
	void					destroyPublication(const Publications::Iterator& it);
	virtual Peer&			myself()=0;

	std::map<std::string,Publication*>					_publications;
	Entities<Group>::Map								_groups;
	Entities<Client>::Map								_clients;
	std::map<std::string,Client*>						_clientsByName;
	std::set<Poco::Net::IPAddress>						_bannedList;
	Mona::UInt32										_nextId;
	std::map<Mona::UInt32,Poco::AutoPtr<FlashStream> >	_streams;
};


inline Publication* Invoker::publish(Exception& ex, const std::string& name) {
	return publish(ex, myself(),name);
}

inline void Invoker::unpublish(const std::string& name) {
	unpublish(myself(),name);
}


inline Poco::AutoPtr<FlashStream> Invoker::getFlashStream(Mona::UInt32 id) {
	return _streams[id];
}

inline void Invoker::destroyFlashStream(Mona::UInt32 id) {
	_streams.erase(id);
}

inline void Invoker::addBanned(const Poco::Net::IPAddress& ip) {
	_bannedList.insert(ip);
}

inline void Invoker::removeBanned(const Poco::Net::IPAddress& ip) {
	_bannedList.erase(ip);
}

inline void Invoker::clearBannedList() {
	_bannedList.clear();
}

inline bool Invoker::isBanned(const Poco::Net::IPAddress& ip) {
	return _bannedList.find(ip)!=_bannedList.end();
}


} // namespace Mona
