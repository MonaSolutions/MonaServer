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
#include "Mona/Gateway.h"
#include "Mona/SocketManager.h"
#include "Mona/ServerParams.h"
#include "Mona/Invoker.h"

namespace Mona {


class Protocol : virtual Object {
public:
	virtual bool		load(Exception& ex, const ProtocolParams& params) { return true; }
	virtual bool		receive(Exception& ex, std::shared_ptr<Buffer<UInt8>>& pBuffer ,SocketAddress& address);
	virtual UInt32		unpack(MemoryReader& packet){return 0;}
	virtual Session*	session(UInt32 id,MemoryReader& packet){return NULL;}
	void				receive(Decoding& decoding) { gateway.receive(decoding); }
	virtual void		check(Session& session){}
	virtual void		manage(){}
	bool				auth(const SocketAddress& address);
	
	const std::string	name;
protected:
	Protocol(const char* name,Invoker& invoker,Gateway& gateway);
	

	Invoker&		invoker;
	Gateway&		gateway;
};


} // namespace Mona
