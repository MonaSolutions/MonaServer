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
#include "Mona/MemoryReader.h"
#include "Mona/AMF.h"
#include "Mona/Buffer.h"
#include <openssl/rc4.h>


namespace Mona {

class RTMPChannel : virtual Object {
public:
	RTMPChannel(): absoluteTime(0),time(0),bodySize(0),type(AMF::EMPTY),streamId(0){}
	UInt32				bodySize;
	UInt32				time;
	UInt32				absoluteTime;
	UInt32				streamId;
	AMF::ContentType	type;
};


class RTMP : virtual Static {
public:

	static enum {
		DEFAULT_CHUNKSIZE =	128,
		DEFAULT_WIN_ACKSIZE = 131072 // TODO default value?
	};

	static UInt32			GetDigestPos(const UInt8* data,bool middle);
	static UInt32			GetDHPos(const UInt8* data,bool middle);
	static const UInt8*		ValidateClient(MemoryReader& packet,bool& middleKey);
	static const UInt8*		ValidateClientScheme(MemoryReader& packet,bool middleKey);
	static void				WriteDigestAndKey(UInt8* data,const UInt8* challengeKey,bool middleKey);
	static void				ComputeRC4Keys(const UInt8* pubKey,UInt32 pubKeySize,const UInt8* farPubKey,UInt32 farPubKeySize,const Buffer<UInt8>& sharedSecret,RC4_KEY& decryptKey,RC4_KEY& encryptKey);

};


} // namespace Mona
