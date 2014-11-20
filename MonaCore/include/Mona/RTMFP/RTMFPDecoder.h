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
#include "Mona/Decoder.h"
#include "Mona/RTMFP/RTMFP.h"

namespace Mona {

class RTMFPDecoder : public Decoder<BinaryReader>, public virtual Object {
public:
	RTMFPDecoder(Invoker& invoker, const UInt8* decryptKey) : Decoder(invoker, "RTMFPDecoder"), _pDecoder(new RTMFPEngine(decryptKey, RTMFPEngine::DECRYPT)) {}
	RTMFPDecoder(Invoker& invoker, const std::shared_ptr<RTMFPEngine>& pDecoder) : Decoder(invoker, "RTMFPDecoder"), _pDecoder(pDecoder) {}

private:
	UInt32 decoding(Exception& ex, UInt8* data,UInt32 size);
	const std::shared_ptr<RTMFPEngine>	  _pDecoder;
};



} // namespace Mona
