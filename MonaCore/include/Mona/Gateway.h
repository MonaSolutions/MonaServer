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
#include "Mona/Peer.h"
#include "Poco/Net/SocketAddress.h"
#include <set>

namespace Mona {

class Session;
class Decoding;
class Protocol;
class Gateway {
public:
	Gateway(){}
	virtual ~Gateway(){}
	
	virtual Session*	session(const UInt8* peerId)=0;
	virtual Session*	session(const SocketAddress& address)=0;

	virtual Session&	registerSession(Session* pSession)=0;
	virtual void		readable(Protocol& protocol)=0;
	virtual void		receive(Decoding& decoded)=0;
};


} // namespace Mona
