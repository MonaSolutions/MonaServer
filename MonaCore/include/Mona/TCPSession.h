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
#include "Mona/Session.h"
#include "Mona/TCPClient.h"

namespace Mona {

class TCPSession : public Session, protected TCPClient, virtual Object {
protected:
	TCPSession(const SocketAddress& address,Protocol& protocol,Invoker& invoker);

	template<typename DecodingType>
	void decode(const std::shared_ptr<DecodingType>& pDecoding,const SocketAddress& address) {
		WARN("TCP Session ", name(), " cannot updated its address (TCP session is in a connected way");
		Session::decode<DecodingType>(pDecoding);
	}

	template<typename DecodingType>
	void decode(const std::shared_ptr<DecodingType>& pDecoding) {
		Session::decode<DecodingType>(pDecoding);
	}

private:
	virtual bool	buildPacket(MemoryReader& packet,const std::shared_ptr<Buffer<UInt8>>& pData) = 0;
	virtual void	packetHandler(MemoryReader& packet)=0;

	// TCPClient implementation
	UInt32			onReception(const std::shared_ptr<Buffer<UInt8>>& pData);
	void			onError(const std::string& error);
	void			onDisconnection() { kill(); }
};



} // namespace Mona
