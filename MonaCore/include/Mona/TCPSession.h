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
#include "Mona/TCPClient.h"

namespace Mona {

class TCPSession : public Session, protected TCPClient, virtual Object {
protected:
	TCPSession(const SocketAddress& address,Protocol& protocol,Invoker& invoker);
	virtual ~TCPSession();

	template<typename DecodingType>
	void decode(std::shared_ptr<DecodingType>& pDecoding) {
		_decoding = true;
		Session::decode<DecodingType>(pDecoding);
	}

private:
	void			receive(MemoryReader& packet) { packetHandler(packet); }
	virtual bool	buildPacket(MemoryReader& data,Poco::UInt32& packetSize)=0;
	virtual void	packetHandler(MemoryReader& packet)=0;

	// TCPClient implementation
	UInt32			onReception(const Poco::UInt8* data,Poco::UInt32 size);
	void			onError(const std::string& error) { ERROR("Protocol ", protocol.name, ", ", error); }
	void			onDisconnection() { kill(); }

	bool			_decoding;
};



} // namespace Mona
