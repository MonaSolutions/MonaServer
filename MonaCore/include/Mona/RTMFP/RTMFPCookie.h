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
#include "Mona/Invoker.h"
#include "Mona/RTMFP/RTMFPCookieComputing.h"


namespace Mona {

class RTMFPCookie : public virtual Object {
public:
	RTMFPCookie(RTMFPHandshake& handshake,Invoker& invoker,const std::string& tag,const std::shared_ptr<Peer>& pPeer);
	
	const UInt32			id;
	const UInt32			farId;
	const std::string		tag;

	const std::shared_ptr<Peer> pPeer;

	bool					run(Exception& ex) { _pComputingThread = _invoker.poolThreads.enqueue<RTMFPCookieComputing>(ex, _pCookieComputing, _pComputingThread); return !ex; }

	const UInt8*			value() { return _pCookieComputing->value; }
	const std::shared_ptr<RTMFPEngine>&	decoder()  { return _pCookieComputing->pDecoder; }
	const std::shared_ptr<RTMFPEngine>&	encoder()  { return _pCookieComputing->pEncoder; }
	
	bool					computeSecret(Exception& ex, const UInt8* initiatorKey,UInt32 sizeKey,const UInt8* initiatorNonce,UInt32 sizeNonce);

	bool					obsolete() { return _createdTimestamp.isElapsed(120000);}  // after 2 mn

	UInt16					length() { return _pCookieComputing->packet.size() + 4; }
	void					read(BinaryWriter& packet) {packet.write32(id).write(_pCookieComputing->packet);}
private:
	PoolThread*								_pComputingThread;
	std::shared_ptr<RTMFPCookieComputing>	_pCookieComputing;
	Time									_createdTimestamp;
	Invoker&								_invoker;
};


} // namespace Mona
