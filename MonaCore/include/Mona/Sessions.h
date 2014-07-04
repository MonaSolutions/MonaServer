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
		BYID = 0,
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
	SessionType* find(const SocketAddress& address) {
		auto it = _sessionsByAddress.find(address);
		if (it == _sessionsByAddress.end())
			return NULL;
		return dynamic_cast<SessionType*>(it->second);
	}


	template<typename SessionType = Session>
	SessionType* find(const UInt8* peerId) {
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

	template<typename SessionType, UInt8 options = BYID,typename ...Args>
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

		if (options&BYPEER) {
			auto it = _sessionsByPeerId.lower_bound(pSession->peer.id);
			if (it != _sessionsByPeerId.end() && memcmp(it->first,pSession->peer.id,ID_SIZE)==0) {
				INFO("Session ", it->second->name(), " overloaded by ",pSession->name()," (by peer id)");
				auto itSession = _sessions.find(it->second->_id);
				if (itSession == _sessions.end())
					CRITIC("Session overloaded ",it->second->name()," impossible to find in sessions collection")
				else
					remove(itSession);
				it->second = pSession;
			} else
				_sessionsByPeerId.emplace_hint(it, pSession->peer.id, pSession);
		}

		if (options&BYADDRESS) {
			auto it = _sessionsByAddress.lower_bound(pSession->peer.address);
			if (it != _sessionsByAddress.end() && it->first == pSession->peer.address) {
				INFO("Session ", it->second->name(), " overloaded by ",pSession->name()," (by ",pSession->peer.address.toString(),")");
				auto itSession = _sessions.find(it->second->_id);
				if (itSession == _sessions.end())
					CRITIC("Session overloaded ",it->second->name()," impossible to find in sessions collection")
				else
					remove(itSession);
				it->second = pSession;
			} else
				_sessionsByAddress.emplace_hint(it, pSession->peer.address, pSession);
		}

		pSession->_sessionsOptions = options;

		pSession->Events::OnAddressChanged::subscribe(onAddressChanged);
		DEBUG("Session ", pSession->name(), " created");
		return *pSession;
	}

private:

	void    remove(std::map<UInt32,Session*>::iterator it);
	void	removeByAddress(Session& session);
	void	removeByAddress(const SocketAddress& address, Session& session);

	Session::OnAddressChanged::Type	onAddressChanged;

	std::map<UInt32,Session*>						_sessions;
	std::set<UInt32>								_freeIds;
	std::map<const UInt8*,Session*,CompareEntity>	_sessionsByPeerId;
	std::map<SocketAddress,Session*>				_sessionsByAddress;
};



} // namespace Mona
