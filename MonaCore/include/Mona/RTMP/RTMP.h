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
#include "Mona/BinaryReader.h"
#include "Mona/AMF.h"
#include "Mona/PoolBuffer.h"
#include "Mona/Crypto.h"
#include <openssl/rc4.h>


namespace Mona {

class RTMPChannel : public virtual Object {
public:
	RTMPChannel(const PoolBuffers& poolBuffers): absoluteTime(0),time(0),bodySize(0),type(AMF::EMPTY),streamId(0),pBuffer(poolBuffers) {}
	UInt32				bodySize;
	UInt32				time;
	UInt32				absoluteTime;
	UInt32				streamId;
	AMF::ContentType	type;
	PoolBuffer			pBuffer;
};


class RTMP : virtual Static {
public:

	enum {
		DEFAULT_CHUNKSIZE =	128,
		DEFAULT_WIN_ACKSIZE = 131072 // TODO default value?
	};

	static UInt16			GetDigestPos(const UInt8* data,bool middle);
	static UInt16			GetDHPos(const UInt8* data,bool middle);
	static const UInt8*		ValidateClient(Crypto& crypto,BinaryReader& reader,bool& middleKey);
	static void				WriteDigestAndKey(Crypto& crypto,UInt8* data,const UInt8* challengeKey,bool middleKey);
	static void				ComputeRC4Keys(Crypto& crypto,const UInt8* pubKey,UInt32 pubKeySize,const UInt8* farPubKey,UInt32 farPubKeySize,const Buffer& sharedSecret,RC4_KEY& decryptKey,RC4_KEY& encryptKey);
private:
	static const UInt8*		ValidateClientScheme(Crypto& crypto,BinaryReader& reader,bool middleKey);
};


} // namespace Mona
