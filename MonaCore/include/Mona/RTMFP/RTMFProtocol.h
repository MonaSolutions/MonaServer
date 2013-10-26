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

class RTMFProtocol : public UDProtocol, virtual Object  {
public:
	RTMFProtocol(const char* name, Invoker& invoker, Gateway& gateway) : UDProtocol(name, invoker, gateway) {}
	
private:
	Poco::SharedPtr<Buffer<UInt8> >		receive(Exception& ex,SocketAddress& address);
	UInt32										unpack(MemoryReader& packet) { return  RTMFP::Unpack(packet); }
	Session*									session(UInt32 id,MemoryReader& packet);
	void										manage() { _pHandshake->manage(); }
	void										check(Session& session);
	virtual bool								load(Exception& ex, const RTMFPParams& params);

	std::unique_ptr<RTMFPHandshake>				_pHandshake;
};


} // namespace Mona
