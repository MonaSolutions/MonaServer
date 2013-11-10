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
#include "Mona/TCPSession.h"
#include "Mona/FlashMainStream.h"
#include "Mona/RTMP/RTMPWriter.h"

namespace Mona {

class RTMPSession : public TCPSession, virtual Object {
public:

	RTMPSession(const SocketAddress& address, Protocol& protocol, Invoker& invoker);
	virtual ~RTMPSession();

private:
	bool			buildPacket(const std::shared_ptr<Buffer<UInt8>>& pData, MemoryReader& packet);
	void			packetHandler(MemoryReader& packet);

	bool			performHandshake(MemoryReader& packet, bool encrypted);
	void			performComplexHandshake(const UInt8* farPubKey,const UInt8* challengeKey, bool encrypted);
	void			performSimpleHandshake(MemoryReader& packet);


	void			manage();


	UInt8							_handshaking;
		
	UInt16						_chunkSize;

	std::map<UInt8,RTMPWriter*>	_writers;
	RTMPWriter*							_pWriter;
	PoolThread*							_pThread;

	std::shared_ptr<RC4_KEY>			_pDecryptKey;
	std::shared_ptr<RC4_KEY>			_pEncryptKey;

	FlashMainStream						_mainStream;
};



} // namespace Mona
