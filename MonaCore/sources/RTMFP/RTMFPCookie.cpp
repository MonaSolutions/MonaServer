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

#include "Mona/RTMFP/RTMFPCookie.h"
#include "Mona/RTMFP/RTMFP.h"


using namespace std;


namespace Mona {

RTMFPCookie::RTMFPCookie(RTMFPHandshake& handshake,Invoker& invoker,const string& tag,const shared_ptr<Peer>& pPeer) : _invoker(invoker), _pComputingThread(NULL),_pCookieComputing(new RTMFPCookieComputing(handshake,invoker)),tag(tag),id(0),farId(0),pPeer(pPeer) {
	
}

bool RTMFPCookie::computeSecret(Exception& ex, const UInt8* initiatorKey,UInt32 sizeKey,const UInt8* initiatorNonce,UInt32 sizeNonce) {
	_pCookieComputing->initiatorKey.resize(sizeKey,false);
	memcpy(_pCookieComputing->initiatorKey.data(),initiatorKey,sizeKey);
	_pCookieComputing->initiatorNonce.resize(sizeNonce,false);
	memcpy(_pCookieComputing->initiatorNonce.data(),initiatorNonce,sizeNonce);
	_pCookieComputing->weak = _pCookieComputing;
	_pComputingThread = _invoker.poolThreads.enqueue<RTMFPCookieComputing>(ex,_pCookieComputing, _pComputingThread);
	return !ex;
}


} // namespace Mona
