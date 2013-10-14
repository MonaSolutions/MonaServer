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

#include "Mona/Decoding.h"
#include "Mona/Logs.h"

using namespace Poco;
using namespace Poco::Net;

namespace Mona {

Decoding::Decoding(UInt32 id,TaskHandler& taskHandler,Protocol& protocol,SharedPtr<Buffer<UInt8> >& pBuffer,const SocketAddress& address): id(id),address(address),Task(taskHandler),_pBuffer(pBuffer),_protocol(protocol),_pPacket(new MemoryReader(pBuffer->begin(),pBuffer->size())) {

}

Decoding::Decoding(UInt32 id,TaskHandler& taskHandler,Protocol& protocol,MemoryReader* pPacket,const SocketAddress& address): id(id),address(address),Task(taskHandler),_protocol(protocol),_pPacket(pPacket) {

}

Decoding::~Decoding() {
	delete _pPacket;
}

void Decoding::run() {
	if(decode(*_pPacket))
		waitHandle();
	else
		ERROR("Decoding error on session %u",id);
}

void Decoding::handle() {
	_protocol.receive(*this);
}

Session* Decoding::session() {
	return _protocol.session(id,*_pPacket);
}


} // namespace Mona
