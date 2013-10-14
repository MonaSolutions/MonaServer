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
#include "Mona/Logs.h"
#include "Poco/Buffer.h"
#include "Poco/SharedPtr.h"

namespace Mona {


class Protocol {
public:
	virtual ~Protocol();

	virtual Poco::SharedPtr<Poco::Buffer<Mona::UInt8> >	receive(Poco::Net::SocketAddress& address);
	virtual Mona::UInt32								unpack(MemoryReader& packet){return 0;}
	virtual Session*									session(Mona::UInt32 id,MemoryReader& packet){return NULL;}
	void												receive(Decoding& decoding);
	virtual void										check(Session& session){}
	virtual void										manage(){}
	bool												auth(const Poco::Net::SocketAddress& address);
	
	const std::string		name;
protected:
	Protocol(const char* name,Invoker& invoker,Gateway& gateway);
	

	Invoker&		invoker;
	Gateway&		gateway;
};

inline Poco::SharedPtr<Poco::Buffer<Mona::UInt8> > Protocol::receive(Poco::Net::SocketAddress& address){
	ERROR("Protocol::receive called without treatment for %s",name.c_str());
	return NULL;
}

inline void Protocol::receive(Decoding& decoding) {
	gateway.receive(decoding);
}


} // namespace Mona
