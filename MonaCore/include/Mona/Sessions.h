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
#include "Mona/Session.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"


namespace Mona {

class Sessions {
public:
	enum {
		BYPEER = 1,
		BYADDRESS = 2,
	};

	typedef std::map<UInt32,Session*>::const_iterator Iterator;

	Sessions();
	virtual ~Sessions();

	UInt32	 count() const { return _sessions.size(); }

	Iterator begin() const { return _sessions.begin(); }
	Iterator end() const { return _sessions.end(); }

	void	 manage();

	template<typename SessionType=Session>
	SessionType* findByAddress(const SocketAddress& address,Socket::Type type) {
		auto& map(type == Socket::DATAGRAM ? _sessionsByAddress[0] : _sessionsByAddress[1]);
		auto it = map.find(address);
		if (it == map.end())
			return NULL;
		return dynamic_cast<SessionType*>(it->second);
	}


	template<typename SessionType = Session>
	SessionType* findByPeer(const UInt8* peerId) {
		auto it = _sessionsByPeerId.find(peerId);
		if (it == _sessionsByPeerId.end())
			return NULL;
		return dynamic_cast<SessionType*>(it->second);
	}


	template<typename SessionType = Session>
	SessionType* find(UInt32 id) {
		auto it = _sessions.find(id);
		if (it == _sessions.end())
			return NULL;
		return dynamic_cast<SessionType*>(it->second);
	}

	template<typename SessionType, UInt8 options = 0,typename ...Args>
	SessionType& create(Args&&... args) {
		SessionType* pSession = new SessionType(args ...);

		auto it = _freeIds.begin();
		if (it != _freeIds.end()) {
			pSession->_id = *it;
			_freeIds.erase(it);
		} else {
			if (_sessions.empty())
				pSession->_id = 1;
			else {
				auto itSession = _sessions.end();
				--itSession;
				pSession->_id = itSession->first+1;
			}
		}

		while(!_sessions.emplace(pSession->_id, pSession).second) {
			CRITIC("Bad computing session id, id ", pSession->_id, " already exists");
			do {
				++pSession->_id;
			} while (find(pSession->_id));
		}

		pSession->_sessionsOptions = options;
		addByPeer(*pSession);
		addByAddress(*pSession);
		
		pSession->Events::OnAddressChanged::subscribe(onAddressChanged);
		DEBUG("Session ", pSession->name(), " created");
		return *pSession;
	}

private:

	void    remove(std::map<UInt32,Session*>::iterator it,UInt8 options);

	void	addByPeer(Session& session);
	void	removeByPeer(Session& session);
	
	void	addByAddress(Session& session);
	void	removeByAddress(Session& session) { removeByAddress(session.peer.address, session); }
	void	removeByAddress(const SocketAddress& address, Session& session);

	Session::OnAddressChanged::Type	onAddressChanged;

	std::map<UInt32,Session*>						_sessions;
	std::set<UInt32>								_freeIds;
	std::map<const UInt8*,Session*,CompareEntity>	_sessionsByPeerId;
	std::map<SocketAddress,Session*>				_sessionsByAddress[2]; // 0 - UDP, 1 - TCP
};



} // namespace Mona
