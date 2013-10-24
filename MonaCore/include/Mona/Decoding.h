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
#include "Mona/Task.h"
#include "Mona/WorkThread.h"
#include "Mona/Protocol.h"
#include "Poco/SharedPtr.h"


namespace Mona {


class Session;
class Decoding : public WorkThread, private Task {
public:
	Decoding(UInt32 id,TaskHandler& taskHandler,Protocol& protocol,MemoryReader* pPacket,const SocketAddress& address);
	virtual ~Decoding();

	const UInt32				id;
	const SocketAddress	address;

	MemoryReader&		packet();
	Session*			session();

protected:
	Decoding(UInt32 id,TaskHandler& taskHandler,Protocol& protocol,Poco::SharedPtr<Poco::Buffer<UInt8> >& pBuffer,const SocketAddress& address);

private:
	virtual bool				decode(MemoryReader& packet){return false;}

	void						handle();
	void						run();

	MemoryReader*								_pPacket;
	Poco::SharedPtr<Poco::Buffer<UInt8> >	_pBuffer;
	Protocol&									_protocol;
};

inline MemoryReader& Decoding::packet() {
	return *_pPacket;
}


} // namespace Mona
