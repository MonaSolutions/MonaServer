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
#include "Mona/Decoding.h"

namespace Mona {


class RTMFPDecoding : public Decoding, virtual Object {
public:
	RTMFPDecoding(Invoker& invoker,PoolBuffer& pBuffer,const std::shared_ptr<RTMFPKey>& pDecryptKey,RTMFPEngine::Type type) : Decoding("RTMFPDecoding",invoker,pBuffer),_decoder(pDecryptKey,RTMFPEngine::DECRYPT) {
		_decoder.type = type;
	}

private:
	bool		  decode(Exception& ex, PacketReader& packet, UInt32 times) { if (times) return false;  packet.next(4); return RTMFP::Decode(ex, _decoder, packet); }

	RTMFPEngine	  _decoder;
};



} // namespace Mona
