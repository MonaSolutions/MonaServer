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

#include "Mona/RTMFP/RTMFProtocol.h"


using namespace std;


namespace Mona {


bool RTMFProtocol::load(Exception& ex, const RTMFPParams& params) {
	if (!load(ex, params))
		return false;
	(UInt16&)params.keepAliveServer *= 1000;
	(UInt16&)params.keepAlivePeer *= 1000;
	_pHandshake.reset(new RTMFPHandshake(*this, gateway, invoker));
	return true;
}

bool RTMFProtocol::receive(Exception& ex,shared_ptr<Buffer<UInt8>> &pBuffer,SocketAddress& address) {
	pBuffer.reset(new Buffer<UInt8>(RTMFP_PACKET_RECV_SIZE));
	int size = receiveFrom(ex,pBuffer->data(),pBuffer->size(),address);
	if (ex)
		return false;
	if(size<RTMFP_MIN_PACKET_SIZE) {
		ERROR("Invalid RTMFP packet");
		return false;
	}
	pBuffer->resize(size);
	return true;
}

Session* RTMFProtocol::session(UInt32 id,MemoryReader& packet) {
	if(id==0)
		return _pHandshake.get();
	return NULL;
}

void RTMFProtocol::check(Session& session) {
	double ptr = 0;
	if (!session.peer.getNumber("&RTMFPCookieComputing", ptr))
		return;
	RTMFPCookieComputing* pCookieComputing = reinterpret_cast<RTMFPCookieComputing*>(static_cast<unsigned>(ptr));
	if(!pCookieComputing)
		return;
	_pHandshake->commitCookie(pCookieComputing->value);
	session.peer.setNumber("&RTMFPCookieComputing",0.);
	delete pCookieComputing;
}




} // namespace Mona
