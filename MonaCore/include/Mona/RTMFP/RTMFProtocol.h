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
#include "Mona/UDProtocol.h"
#include "Mona/RTMFP/RTMFP.h"
#include "Mona/RTMFP/RTMFPHandshake.h"

namespace Mona {

class RTMFProtocol : public UDProtocol  {
public:
	RTMFProtocol(const char* name,const RTMFPParams& params,Gateway& gateway,Invoker& invoker);
	virtual ~RTMFProtocol();

private:
	Poco::SharedPtr<Poco::Buffer<UInt8> >	receive(Poco::Net::SocketAddress& address);
	UInt32								unpack(MemoryReader& packet);
	Session*									session(UInt32 id,MemoryReader& packet);
	void										manage();
	void										check(Session& session);

	RTMFPHandshake*								_pHandshake;
};

inline void RTMFProtocol::manage() {
	_pHandshake->manage();
}

inline UInt32 RTMFProtocol::unpack(MemoryReader& packet) {
	return  RTMFP::Unpack(packet);
}


} // namespace Mona
