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

#include "Mona/Sessions.h"
#include "Mona/Session.h"
#include "Mona/Logs.h"

using namespace std;

namespace Mona {


Sessions::Sessions():_nextId(1),_oldCount(0) {
}

Sessions::~Sessions() {
	// delete sessions
	_sessionsByAddress.clear();
	_sessionsByPeerId.clear();
	if (!_sessions.empty())
		WARN("sessions are deleting");
	Iterator it;
	for (it = begin(); it != end(); ++it)
		delete it->second;
	_sessions.clear();
}

void Sessions::remove(map<UInt32,Session*>::iterator it) {
	DEBUG("Session ",it->second->name()," died");
	_sessionsByPeerId.erase(it->second->peer.id);
	_sessionsByAddress.erase(it->second->peer.address);
	delete it->second;
	_sessions.erase(it);
}

void Sessions::updateAddress(Session& session, const SocketAddress& oldAddress) {
	INFO("Session ",session.name()," has changed its address (",oldAddress.toString()," -> ",session.peer.address.toString(),")");
	_sessionsByAddress.erase(oldAddress);
	_sessionsByAddress[session.peer.address] = &session;
}


void Sessions::manage() {
	auto it= _sessions.begin();
	while(it!=_sessions.end()) {
		if(!it->second->died)
			it->second->manage();
		if(!it->second->died)
			it->second->flush();
		if(it->second->died) {
			remove(it++);
			continue;
		}
		++it;
	}
	if(_sessions.size()!=_oldCount) {
		INFO(count()," clients");
		_oldCount=_sessions.size();
	}
}




} // namespace Mona
