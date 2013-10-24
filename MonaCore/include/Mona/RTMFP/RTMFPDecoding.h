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
#include "Mona/Decoding.h"

namespace Mona {


class RTMFPDecoding : public Decoding {
public:
	RTMFPDecoding(UInt32 id,TaskHandler& taskHandler,Protocol& protocol,Poco::SharedPtr<Poco::Buffer<UInt8> >& pBuffer,const Poco::Net::SocketAddress& address): Decoding(id,taskHandler,protocol,pBuffer,address){}
	virtual ~RTMFPDecoding(){}

	RTMFPEngine					decoder;

private:
	bool						decode(MemoryReader& packet);

};

inline bool RTMFPDecoding::decode(MemoryReader& packet) {
	packet.next(4);
	return RTMFP::Decode(decoder,packet);
}



} // namespace Mona
