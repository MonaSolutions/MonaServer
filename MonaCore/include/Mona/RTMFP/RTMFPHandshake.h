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
#include "Mona/Exceptions.h"
#include "Mona/AttemptCounter.h"
#include "Mona/RTMFP/RTMFPSession.h"
#include "Mona/RTMFP/RTMFPCookie.h"


namespace Mona {

class HelloAttempt : public Attempt, public virtual Object {
public:
	HelloAttempt() : pCookie(NULL) {
	}
	RTMFPCookie*		pCookie;
};

class Sessions;
class RTMFPHandshake : public RTMFPSession, private AttemptCounter, public virtual Object {
public:
	RTMFPHandshake(RTMFProtocol& protocol, Sessions& sessions, Invoker& invoker);
	virtual ~RTMFPHandshake();

	void			commitCookie(const UInt8* value);
	void			manage();
	void			clear();
	RTMFPSession*	createSession(const UInt8* cookieValue);

private:

	void		receive(const SocketAddress& address, BinaryReader& packet);
	UInt8		handshakeHandler(UInt8 id,const SocketAddress& address, BinaryReader& request,BinaryWriter& response);

	struct CompareCookies {
	   bool operator()(const UInt8* a,const UInt8* b) const {
		   return std::memcmp(a,b,COOKIE_SIZE)<0;
	   }
	};
	
	std::map<const UInt8*,RTMFPCookie*,CompareCookies>  _cookies; // RTMFPCookie, in waiting of creation session
	UInt8												_certificat[77];
	Sessions&											_sessions;
	std::shared_ptr<Peer>								_pPeer;
};



} // namespace Mona
