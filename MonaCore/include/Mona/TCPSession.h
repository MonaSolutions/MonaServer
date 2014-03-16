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
#include "Mona/Decoding.h"

namespace Mona {

class TCPSession : public Session, public TCPClient, virtual Object {
protected:
	TCPSession(const SocketAddress& peerAddress, SocketFile& file,Protocol& protocol,Invoker& invoker);
	virtual ~TCPSession() { TCPClient::close(); }

	template<typename DecodingType>
	void decode(const std::shared_ptr<DecodingType>& pDecoding,const SocketAddress& address) {
		WARN("TCP Session ", name(), " cannot updated its address (TCP session is in a connected way");
		decode(pDecoding);
	}

	template<typename DecodingType>
	void decode(const std::shared_ptr<DecodingType>& pDecoding) {
		_decodings.emplace_back(pDecoding);
		_decoding = true;
		Session::decode(pDecoding);
	}

	void kill() { disconnect(); Session::kill(); }

private:
	void receive(PacketReader& packet, const SocketAddress& address) {
		WARN("TCP Session ", name(), " cannot updated its address (TCP session is in a connected way");
		Session::receive(packet);
	}
	void receive(PacketReader& packet);

	virtual bool	buildPacket(PoolBuffer& pBuffer,PacketReader& packet) = 0;

	// TCPClient implementation
	UInt32			onReception(PoolBuffer& pBuffer);
	void			onError(const Exception& ex);
	void			onDisconnection() { Session::kill(); }

	bool			_consumed;
	bool			_decoding;
	std::deque<std::shared_ptr<Decoding>> _decodings;
};



} // namespace Mona
