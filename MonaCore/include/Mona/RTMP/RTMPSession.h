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
	void			onNewData(const Poco::UInt8* data,Poco::UInt32 size);
	bool			buildPacket(MemoryReader& data,Poco::UInt32& packetSize);
	void			packetHandler(MemoryReader& packet);

	bool			performHandshake(MemoryReader& packet, bool encrypted);
	void			performComplexHandshake(const Poco::UInt8* farPubKey,const Poco::UInt8* challengeKey, bool encrypted);
	void			performSimpleHandshake(MemoryReader& packet);


	void			manage();


	Poco::UInt8							_handshaking;
		
	Poco::UInt16						_chunkSize;

	std::map<Poco::UInt8,RTMPWriter*>	_writers;
	RTMPWriter*							_pWriter;

	Poco::SharedPtr<RC4_KEY>			_pDecryptKey;
	Poco::SharedPtr<RC4_KEY>			_pEncryptKey;

	FlashMainStream						_mainStream;
};



} // namespace Mona
