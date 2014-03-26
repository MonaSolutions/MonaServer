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

#include "ServerConnection.h"
#include <set>

class Broadcaster {
	friend class Servers;
public:
	Broadcaster(const Mona::PoolBuffers& poolBuffers) : poolBuffers(poolBuffers) {}
	virtual ~Broadcaster() {}

	typedef std::set<ServerConnection*>::const_iterator Iterator;
	
	Iterator begin() const { return _connections.begin(); }
	Iterator end() const { return _connections.end(); }
	ServerConnection* operator[](Mona::UInt32 index);
	ServerConnection* operator[](const std::string& address);
	Mona::UInt32 count() const { return _connections.size(); }
	
	void broadcast(const char* handler,Mona::PacketWriter& packet);

	const Mona::PoolBuffers& poolBuffers;
private:
	std::set<ServerConnection*>				_connections;
};


inline ServerConnection* Broadcaster::operator[](const std::string& address) {
	Iterator it;
	for(it=begin();it!=end();++it) {
		if((*it)->address.toString()==address || (*it)->host==address)
			return *it;
	}
	return *it;
}

inline ServerConnection* Broadcaster::operator[](Mona::UInt32 index) {
	if(index>=_connections.size())
		return NULL;
	Iterator it = begin();
	advance(it,index);
	return *it;
}

inline void Broadcaster::broadcast(const char* handler,Mona::PacketWriter& packet) {
	Iterator it;
	for (it = begin(); it != end(); ++it) {
		std::shared_ptr<ServerMessage> pMessage(new ServerMessage(handler,poolBuffers));
		memcpy(pMessage->packet.buffer(packet.size()),packet.data(),packet.size());
		(*it)->send(pMessage);
	}
}
