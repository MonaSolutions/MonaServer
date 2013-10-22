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
#include "Mona/Invoker.h"
#include "Mona/MemoryWriter.h"
#include "Mona/RTMFP/RTMFPCookieComputing.h"
#include "Poco/Buffer.h"

namespace Mona {

class RTMFPCookie {
public:
	RTMFPCookie(Exception& ex, RTMFPHandshake& handshake,Invoker& invoker,const std::string& tag,const std::string& queryUrl); // For normal cookie
	virtual ~RTMFPCookie();

	const Mona::UInt32				id;
	const Mona::UInt32				farId;
	const std::string				tag;
	const std::string				queryUrl;

	const Mona::UInt8				peerId[ID_SIZE];
	const Poco::Net::SocketAddress	peerAddress;

	const Mona::UInt8*				value();
	const Mona::UInt8				decryptKey[HMAC_KEY_SIZE];
	const Mona::UInt8				encryptKey[HMAC_KEY_SIZE];
	
	void							computeSecret(Exception& ex, const Mona::UInt8* initiatorKey,Mona::UInt32 sizeKey,const Mona::UInt8* initiatorNonce,Mona::UInt32 sizeNonce);
	void							finalize();

	bool							obsolete();

	Mona::UInt16					length();
	Mona::UInt16					read(MemoryWriter& writer);
private:
	PoolThread*						_pComputingThread;
	Poco::AutoPtr<RTMFPCookieComputing>	_pCookieComputing;
	Mona::Time					_createdTimestamp;

	Mona::UInt8						_buffer[256];
	MemoryWriter					_writer;
	Invoker&						_invoker;
	Poco::Buffer<Mona::UInt8>		_initiatorNonce;
};

inline const Mona::UInt8* RTMFPCookie::value() {
	return _pCookieComputing->value;
}

inline Mona::UInt16	RTMFPCookie::length() {
	return _writer.length()+4;
}

inline bool RTMFPCookie::obsolete() {
	return _createdTimestamp.isElapsed(120000000); // after 2 mn
}



} // namespace Mona
