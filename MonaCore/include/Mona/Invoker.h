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
#include "Mona/SocketManager.h"
#include "Mona/RelayServer.h"
#include "Mona/Group.h"
#include "Mona/Publications.h"
#include "Mona/Entities.h"
#include "Mona/TaskHandler.h"
#include "Mona/PoolThreads.h"
#include "Mona/PoolBuffers.h"
#include "Mona/Peer.h"

namespace Mona {

class Invoker : public Entity,public TaskHandler, public virtual Object, public MapParameters {
public:
	// invocations
	const Entities<Client>  clients;
	const Entities<Group>	groups;

	const RelayServer		relayer;

	const SocketManager		sockets;
	PoolThreads				poolThreads;
	const PoolBuffers		poolBuffers;

	void					addBanned(const IPAddress& ip) { _bannedList.insert(ip); }
	void					removeBanned(const IPAddress& ip) { _bannedList.erase(ip); }
	void					clearBannedList() { _bannedList.clear(); }
	bool					isBanned(const IPAddress& ip) { return _bannedList.find(ip) != _bannedList.end(); }
	Room*					defaultRoom;
	virtual const std::string&	rootPath() const = 0; // for to override! Otherwise the executable folder could see these files readable by un protocol

protected:
	Invoker(UInt32 socketBufferSize,UInt16 threads);
	virtual ~Invoker();

private:

	std::set<IPAddress>								_bannedList;

};


} // namespace Mona
