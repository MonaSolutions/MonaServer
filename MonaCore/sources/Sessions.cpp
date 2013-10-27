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

#include "Mona/Sessions.h"
#include "Mona/Logs.h"

using namespace std;
using namespace Poco;


namespace Mona {

Sessions::Sessions():_nextId(1),_oldCount(0) {
}

Sessions::~Sessions() {
	clear();
}

void Sessions::clear() {
	// delete sessions
	_sessionsByAddress.clear();
	_sessionsByPeerId.clear();
	if(!_sessions.empty())
		WARN("sessions are deleting");
	Iterator it;
	for(it=begin();it!=end();++it)
		delete it->second;
	_sessions.clear();
}

Session& Sessions::add(Session* pSession) {
	(UInt32&)pSession->id=_nextId;

	_sessions[_nextId] = pSession;
	_sessionsByPeerId[pSession->peer.id] = pSession;
	_sessionsByAddress[pSession->peer.address] = pSession;
	DEBUG("Session ",_nextId," created");

	do {
		++_nextId;
	} while(_nextId==0 && find(_nextId));

	return* pSession;
}

void Sessions::remove(map<UInt32,Session*>::iterator it) {
	DEBUG("Session ",it->second->id," died");
	_sessionsByPeerId.erase(it->second->peer.id);
	_sessionsByAddress.erase(it->second->peer.address);
	delete it->second;
	_sessions.erase(it);
}

void Sessions::changeAddress(const SocketAddress& oldAddress,Session& session) {
	INFO("Session ",session.id," has changed its address (",oldAddress.toString()," -> ",session.peer.address.toString(),")");
	_sessionsByAddress.erase(oldAddress);
	_sessionsByAddress[session.peer.address] = &session;
}

Session* Sessions::find(const SocketAddress& address) {
	map<SocketAddress,Session*>::const_iterator it = _sessionsByAddress.find(address);
	if(it==_sessionsByAddress.end())
		return NULL;
	return it->second;
}


Session* Sessions::find(const UInt8* peerId) {
	Entities<Session>::Iterator it = _sessionsByPeerId.find(peerId);
	if(it==_sessionsByPeerId.end())
		return NULL;
	return it->second;
}


Session* Sessions::find(UInt32 id) {
	map<UInt32,Session*>::iterator it = _sessions.find(id);
	if(it==_sessions.end())
		return NULL;
	return it->second;
}

void Sessions::manage() {
	map<UInt32,Session*>::iterator it= _sessions.begin();
	while(it!=_sessions.end()) {
		it->second->manage();
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
