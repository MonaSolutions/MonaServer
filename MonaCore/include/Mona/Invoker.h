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
#include "Mona/Clients.h"
#include "Mona/TaskHandler.h"
#include "Mona/PoolThreads.h"
#include "Mona/PoolBuffers.h"
#include "Mona/FlashMainStream.h"

namespace Mona {

class Invoker : public Entity,public TaskHandler, public virtual Object, public MapParameters {
	friend class FlashStream; // FlashStream manage _streams
public:
	// invocations
	const Clients			clients;
	const Entities<Group>	groups;
	Publications			publications;
	const RelayServer		relayer;

	const SocketManager		sockets;
	PoolThreads				poolThreads;
	const PoolBuffers		poolBuffers;

	std::shared_ptr<FlashStream>&	createFlashStream(Peer& peer);
	FlashStream&					flashStream(UInt32 id, Peer& peer,std::shared_ptr<FlashStream>& pStream);
	void							destroyFlashStream(UInt32 id) { _streams.erase(id); }

	Publication*			publish(Exception& ex, const std::string& name, Publication::Type type) { return publish(ex, name, type,NULL); }
	void					unpublish(const std::string& name) { unpublish(name, NULL); }

	Publication*			publish(Exception& ex, Peer& peer, const std::string& name, Publication::Type type) { return publish(ex, name, type, &peer); }
	void					unpublish(Peer& peer, const std::string& name) { unpublish(name,&peer);  }
	Listener*				subscribe(Exception& ex,Peer& peer,std::string& name,Writer& writer);
	Listener*				subscribe(Exception& ex,Peer& peer,const std::string& name,Writer& writer);
	void					unsubscribe(Peer& peer,const std::string& name);

	void					addBanned(const IPAddress& ip) { _bannedList.insert(ip); }
	void					removeBanned(const IPAddress& ip) { _bannedList.erase(ip); }
	void					clearBannedList() { _bannedList.clear(); }
	bool					isBanned(const IPAddress& ip) { return _bannedList.find(ip) != _bannedList.end(); }

protected:
	Invoker(UInt32 socketBufferSize,UInt16 threads);
	virtual ~Invoker();

private:
	Publication*			publish(Exception& ex,const std::string& name, Publication::Type type,Peer* pPeer);
	void					unpublish(const std::string& name, Peer* pPeer);

	std::string&			publicationName(std::string& name,std::string& query);

	virtual bool			onPublish(Exception& ex, const Publication& publication) = 0;
	virtual void			onUnpublish(const Publication& publication) = 0;

	std::map<std::string,Publication>				_publications;
	std::set<IPAddress>								_bannedList;
	UInt32											_nextId;
	std::map<UInt32,std::shared_ptr<FlashStream> >	_streams;

};


} // namespace Mona
