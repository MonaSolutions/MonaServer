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
#include "Mona/Entities.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"
#include <cstddef>

namespace Mona {

class Session;
class Sessions {
public:
	typedef std::map<UInt32,Session*>::const_iterator Iterator;

	Sessions();
	virtual ~Sessions();

	UInt32	count() const { return _sessions.size(); }

	void	 updateAddress(Session& session, const SocketAddress& oldAddress);

	Iterator begin() const { return _sessions.begin(); }
	Iterator end() const { return _sessions.end(); }

	void		manage();

	template<typename SessionType=Session>
	SessionType* find(const SocketAddress& address) {
		auto& it = _sessionsByAddress.find(address);
		if (it == _sessionsByAddress.end())
			return NULL;
		return dynamic_cast<SessionType*>(it->second);
	}


	template<typename SessionType = Session>
	SessionType* find(const UInt8* peerId) {
		auto& it = _sessionsByPeerId.find(peerId);
		if (it == _sessionsByPeerId.end())
			return NULL;
		return dynamic_cast<SessionType*>(it->second);
	}


	template<typename SessionType = Session>
	SessionType* find(UInt32 id) {
		auto& it = _sessions.find(id);
		if (it == _sessions.end())
			return NULL;
		return dynamic_cast<SessionType*>(it->second);
	}
	

	template<typename SessionType>
	SessionType& add(SessionType& session) {
		session._id = _nextId;
		_sessions[_nextId] = &session;
		_sessionsByPeerId[session.peer.id] = &session;
		_sessionsByAddress[session.peer.address] = &session;
		DEBUG("Session ", _nextId, " created");
		do {
			++_nextId;
		} while (_nextId == 0 && find(_nextId));
		return session;
	}

private:

	void    remove(std::map<UInt32,Session*>::iterator it);

	UInt32									_nextId;
	std::map<UInt32,Session*>				_sessions;
	Entities<Session>::Map					_sessionsByPeerId;
	std::map<SocketAddress,Session*>		_sessionsByAddress;
	UInt32									_oldCount;
};



} // namespace Mona
