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

#pragma once

#include "Mona/Mona.h"
#include "Mona/Session.h"
#include "Mona/Entities.h"
#include "Mona/Util.h"
#include <cstddef>

namespace Mona {

class Sessions : ObjectFix {
public:
	typedef std::map<UInt32,Session*>::const_iterator Iterator;

	Sessions();
	virtual ~Sessions();

	UInt32	count() const { return _sessions.size(); }

	Session* find(UInt32 id);
	Session* find(const UInt8* peerId);
	Session* find(const SocketAddress& address);
	
	void	 changeAddress(const SocketAddress& oldAddress,Session& session);
	Session& add(Session* pSession);

	Iterator begin() const { return _sessions.begin(); }
	Iterator end() const { return _sessions.end(); }
	
	void		manage();
	void		clear();

private:
	void    remove(std::map<UInt32,Session*>::iterator it);

	UInt32									_nextId;
	std::map<UInt32,Session*>				_sessions;
	Entities<Session>::Map					_sessionsByPeerId;
	std::map<SocketAddress,Session*>		_sessionsByAddress;
	UInt32									_oldCount;
};



} // namespace Mona
