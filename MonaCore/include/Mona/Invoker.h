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
#include "Mona/Group.h"
#include "Mona/Publications.h"
#include "Mona/Entities.h"
#include "Mona/Clients.h"
#include "Mona/SocketManager.h"
#include "Mona/TaskHandler.h"
#include "Mona/PoolThreads.h"
#include "Mona/PoolBuffers.h"
#include "Mona/ServerParams.h"
#include "Mona/FlashMainStream.h"
#include "Mona/RelayServer.h"

namespace Mona {

class Invoker : public Entity,public TaskHandler, virtual Object {
	friend class Peer; // Peer manage _clients,_clientsByName and _groups list!
	friend class FlashStream; // FlashStream manage _streams
public:
	// invocations
	Clients					clients;
	Entities<Group>			groups;
	Publications			publications;
	const SocketManager		sockets;
	const RelayServer		relay;
	PoolThreads				poolThreads;
	const PoolBuffers		poolBuffers;

	std::shared_ptr<FlashStream>&	createFlashStream(Peer& peer);
	FlashStream&					flashStream(UInt32 id, Peer& peer,std::shared_ptr<FlashStream>& pStream);
	void							destroyFlashStream(UInt32 id) { _streams.erase(id); }

	Publication*			publish(Exception& ex,const std::string& name) { return publish(ex,myself(), name); }
	void					unpublish(const std::string& name) { unpublish(myself(), name); }

	Publication*			publish(Exception& ex,Peer& peer,const std::string& name);
	void					unpublish(Peer& peer,const std::string& name);
	Listener*				subscribe(Exception& ex,Peer& peer,const std::string& name,Writer& writer,double start=-2000);
	void					unsubscribe(Peer& peer,const std::string& name);

	void					addBanned(const IPAddress& ip) { _bannedList.insert(ip); }
	void					removeBanned(const IPAddress& ip) { _bannedList.erase(ip); }
	void					clearBannedList() { _bannedList.clear(); }
	bool					isBanned(const IPAddress& ip) { return _bannedList.find(ip) != _bannedList.end(); }

	const ServerParams		params;

protected:
	Invoker(UInt32 bufferSize,UInt16 threads);
	virtual ~Invoker();

private:
	Publications::Iterator	createPublication(const std::string& name);
	void					destroyPublication(const Publications::Iterator& it);
	virtual Peer&			myself()=0;

	std::map<std::string,Publication*>					_publications;
	Entities<Group>::Map								_groups;
	Entities<Client>::Map								_clients;
	std::map<std::string,Client*>						_clientsByName;
	std::set<IPAddress>						    _bannedList;
	UInt32										_nextId;
	std::map<UInt32,std::shared_ptr<FlashStream> >	_streams;
};


} // namespace Mona
