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
	RTMFPDecoding(Invoker& invoker,const UInt8* data,UInt32 size,const RTMFPEngine& engine,RTMFPEngine::Type type) : Decoding("RTMFPDecoding",invoker,data,size),_decoder(engine,type) {}

private:
	bool		  decode(Exception& ex, MemoryReader& reader, UInt32 times) { if (times) return false;  reader.next(4); return RTMFP::Decode(ex, _decoder, reader); }

	RTMFPEngine	  _decoder;
};



} // namespace Mona
