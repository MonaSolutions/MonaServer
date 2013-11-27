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
#include "Mona/Protocol.h"
#include "Mona/UDPSocket.h"
#include "Mona/Logs.h"

namespace Mona {

class UDProtocol : public Protocol, public UDPSocket, virtual Object {
public:
	bool load(Exception& ex, const ProtocolParams& params);

protected:
	UDProtocol(const char* name, Invoker& invoker, Sessions& sessions) : UDPSocket(invoker.sockets), Protocol(name, invoker, sessions) {}
	
private:
	void		onReception(const std::shared_ptr<Buffer<UInt8>>& pData, const SocketAddress& address);
	void		onError(const std::string& error) { WARN("Protocol ",name,", ", error); }

	virtual void onPacket(const std::shared_ptr<Buffer<UInt8>>& pData, const SocketAddress& address) = 0;
};

inline bool UDProtocol::load(Exception& ex, const ProtocolParams& params) {
	SocketAddress address;
	if (!address.set(ex, params.host, params.port))
		return false;
	return bind(ex, address);
}


inline void	UDProtocol::onReception(const std::shared_ptr<Buffer<UInt8>>& pData, const SocketAddress& address) {
	if (!pData) {
		WARN("Protocol ", name, ", onReception of null data");
		return;
	}
	if(!auth(address))
		return;
	onPacket(pData, address);
}


} // namespace Mona
