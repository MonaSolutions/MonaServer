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
#include "Mona/Exceptions.h"
#include "Mona/AttemptCounter.h"
#include "Mona/RTMFP/RTMFPSession.h"
#include "Mona/RTMFP/RTMFPCookie.h"


namespace Mona {

class HelloAttempt : public Attempt {
public:
	HelloAttempt() : pCookie(NULL) {
	}
	~HelloAttempt() {
	}
	RTMFPCookie*		pCookie;
};

class RTMFPHandshake : public RTMFPSession, private AttemptCounter {
public:
	RTMFPHandshake(RTMFProtocol& protocol,Gateway& gateway,Invoker& invoker);
	virtual ~RTMFPHandshake();

	void		createCookie(Exception& ex, MemoryWriter& writer,HelloAttempt& attempt,const std::string& tag,const std::string& queryUrl);
	void		commitCookie(const Mona::UInt8* value);
	void		manage();
	void		clear();
	Session*	createSession(const Mona::UInt8* cookieValue);

private:

	void		flush(Exception& ex);
	void		flush(Exception& ex, RTMFPEngine::Type type);

	void		packetHandler(MemoryReader& packet);
	Mona::UInt8	handshakeHandler(Mona::UInt8 id,MemoryReader& request,MemoryWriter& response);

	struct CompareCookies {
	   bool operator()(const Mona::UInt8* a,const Mona::UInt8* b) const {
		   return std::memcmp(a,b,COOKIE_SIZE)<0;
	   }
	};
	
	std::map<const Mona::UInt8*,RTMFPCookie*,CompareCookies> _cookies; // RTMFPCookie, in waiting of creation session
	Mona::UInt8											_certificat[77];
	Gateway&											_gateway;
};

inline void RTMFPHandshake::flush(Exception& ex) {
	RTMFPSession::flush(ex, 0x0b,false);
}

inline void RTMFPHandshake::flush(Exception& ex, RTMFPEngine::Type type) {
	RTMFPSession::flush(ex, 0x0b,false,type);
}


} // namespace Mona
