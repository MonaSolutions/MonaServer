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

#include "Mona/RTMFP/RTMFPCookieComputing.h"
#include "Mona/RTMFP/RTMFPHandshake.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"
#include "Poco/RandomStream.h"

using namespace std;
using namespace Poco;

namespace Mona {

RTMFPCookieComputing::RTMFPCookieComputing(RTMFPHandshake& handshake,Invoker& invoker): _handshake(handshake),value(),Task(invoker),initiatorKey(0),sharedSecret(0) {
	RandomInputStream().read((char*)value,COOKIE_SIZE);
}

bool RTMFPCookieComputing::run(Exception& ex) {
	// First execution is for the DH computing if pDH == null, else it's to compute Diffie-Hellman keys
	if(diffieHellman.initialize())
		return true;

	// Compute Diffie-Hellman secret
	diffieHellman.computeSecret(initiatorKey,sharedSecret);

	DEBUG("Shared Secret : ",Util::FormatHex(sharedSecret.data(),sharedSecret.size()));
	
	waitHandle(ex);
	return true;
}

void RTMFPCookieComputing::handle(Exception& ex) {
	Session* pSession = _handshake.createSession(value);
	if (pSession)
		pSession->peer.setNumber("&RTMFPCookieComputing", (double)reinterpret_cast<unsigned>(this));
	else
		weak.reset();
}


} // namespace Mona
