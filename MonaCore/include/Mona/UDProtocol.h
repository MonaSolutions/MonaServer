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
#include "Mona/Protocol.h"
#include "Mona/SocketHandler.h"

namespace Mona {


class UDProtocol : public SocketHandler<Poco::Net::DatagramSocket>,public Protocol {
protected:
	UDProtocol(const char* name,Invoker& invoker,Gateway& gateway) : SocketHandler<Poco::Net::DatagramSocket>(invoker.sockets),Protocol(name,invoker,gateway) {}
	virtual ~UDProtocol(){}

private:
	void			onReadable();
	void			onError(const std::string& error);
};

inline void UDProtocol::onError(const std::string& error) {
	WARN("Protocol %s, %s",error.c_str(),name.c_str());
}

inline void UDProtocol::onReadable() {
	gateway.readable(*this);
}

} // namespace Mona
