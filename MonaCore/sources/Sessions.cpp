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


Sessions::Sessions() {
	onAddressChanged = [this](Session& session, const SocketAddress& oldAddress) {
		INFO("Session ",session.name()," has changed its address, ",oldAddress.toString()," -> ",session.peer.address.toString());
		if (!(session._sessionsOptions&BYADDRESS))
			return;
		removeByAddress(oldAddress, session);
		auto it = _sessionsByAddress.lower_bound(session.peer.address);
		if (it != _sessionsByAddress.end() && it->first == session.peer.address) {
			if (it->second != &session) {
				INFO("Session ", it->second->name(), " overloaded by ",session.name()," (by ",session.peer.address.toString(),")");
				auto itSession = _sessions.find(it->second->_id);
				if (itSession == _sessions.end())
					CRITIC("Session overloaded ",it->second->name()," impossible to find in sessions collection")
				else
					remove(itSession);
				it->second = &session;
			}
		} else
			_sessionsByAddress.emplace_hint(it, session.peer.address, &session);
	};
}

Sessions::~Sessions() {
	// delete sessions
	if (!_sessions.empty())
		WARN("sessions are deleting");
	Iterator it;
	for (it = begin(); it != end(); ++it) {
		it->second->kill(Session::SERVER_DEATH);
		delete it->second;
	}
}

void Sessions::removeByAddress(Session& session) {
	if (session._sessionsOptions&BYADDRESS)
		removeByAddress(session.peer.address, session);
}

void Sessions::removeByAddress(const SocketAddress& address,Session& session) {
	if (_sessionsByAddress.erase(address)==0) {
		ERROR("Session ",session.name()," unfound in address sessions collection with key ",address.toString());
		for (auto it = _sessionsByAddress.begin(); it != _sessionsByAddress.end();++it) {
			if (it->second == &session) {
				INFO("The correct key was ",it->first.toString());
				_sessionsByAddress.erase(it);
				break;
			}
		}
	}
}

void Sessions::remove(map<UInt32,Session*>::iterator it) {
	Session& session(*it->second);
	DEBUG("Session ",session.name()," died");

	if (session._sessionsOptions&BYPEER) {
		if (_sessionsByPeerId.erase(session.peer.id)==0) {
 			ERROR("Session ",session.name()," unfound in peer sessions collection with key ",Util::FormatHex(session.peer.id,ID_SIZE,LOG_BUFFER));
 			for (auto it = _sessionsByPeerId.begin(); it != _sessionsByPeerId.end();++it) {
 				if (it->second == &session) {
					INFO("The correct key was ",Util::FormatHex(it->first,ID_SIZE,LOG_BUFFER));
 					_sessionsByPeerId.erase(it);
					break;
				}
			}
		}
	}

	removeByAddress(session);
	session.kill();
	_freeIds.emplace(session._id);
	delete &session;
	_sessions.erase(it);
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
}




} // namespace Mona
