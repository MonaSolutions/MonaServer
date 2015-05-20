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
#include "Mona/FlashStream.h"
#include <openssl/rc4.h>


namespace Mona {

class RTMPChannel : public virtual Object {
public:
	RTMPChannel(const PoolBuffers& poolBuffers): absoluteTime(0),time(0),bodySize(0),type(AMF::EMPTY),pBuffer(poolBuffers) {}
	UInt32							bodySize;
	UInt32							time;
	UInt32							absoluteTime;
	std::shared_ptr<FlashStream>	pStream;
	AMF::ContentType				type;
	PoolBuffer						pBuffer;

	void reset(FlashWriter* pWriter) {
		bodySize = time = absoluteTime = 0;
		type = AMF::EMPTY;
		pBuffer.release();
		if (pStream) {
			pStream->disengage(pWriter);
			pStream.reset();
		}
	}
};


class RTMP : virtual Static {
public:

	enum {
		DEFAULT_CHUNKSIZE =	128,
		DEFAULT_WIN_ACKSIZE = 131072 // TODO default value?
	};

	static UInt32			GetDigestPos(const UInt8* data,UInt32 size,bool middle, UInt32& length);
	static UInt32			GetDHPos(const UInt8* data,UInt32 size,bool middle, UInt32& length);
	static const UInt8*		ValidateClient(Crypto::HMAC& hmac,const UInt8* data, UInt32 size,bool& middleKey,UInt32& keySize);
	static bool				WriteDigestAndKey(Exception& ex,Crypto::HMAC& hmac,const UInt8* key,UInt32 keySize,bool middleKey,UInt8* data,UInt32 size);
	static void				ComputeRC4Keys(Crypto::HMAC& hmac,const UInt8* pubKey,UInt32 pubKeySize,const UInt8* farPubKey,UInt32 farPubKeySize,const Buffer& sharedSecret,RC4_KEY& decryptKey,RC4_KEY& encryptKey);
private:
	static const UInt8*		ValidateClientScheme(Crypto::HMAC& hmac,const UInt8* data, UInt32 size,bool middleKey,UInt32& keySize);
};


} // namespace Mona
