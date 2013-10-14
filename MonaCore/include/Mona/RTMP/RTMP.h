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
#include "Mona/MemoryReader.h"
#include "Mona/AMF.h"
#include "Poco/Buffer.h"
#include <openssl/rc4.h>


namespace Mona {

#define DEFAULT_CHUNKSIZE	128


class RTMPChannel {
public:
	RTMPChannel():headerSize(0),time(0),bodySize(0),type(AMF::EMPTY),streamId(0){}
	virtual ~RTMPChannel(){}

	Mona::UInt16		headerSize;
	Mona::UInt32		bodySize;
	Mona::UInt32		time;
	Mona::UInt32		streamId;
	AMF::ContentType	type;
};


class RTMP {
public:

	static Mona::UInt32			GetDigestPos(const Mona::UInt8* data,bool middle);
	static Mona::UInt32			GetDHPos(const Mona::UInt8* data,bool middle);
	static const Mona::UInt8*	ValidateClient(MemoryReader& packet,bool& middleKey);
	static const Mona::UInt8*	ValidateClientScheme(MemoryReader& packet,bool middleKey);
	static void					WriteDigestAndKey(Mona::UInt8* data,const Mona::UInt8* challengeKey,bool middleKey);
	static void					ComputeRC4Keys(const Mona::UInt8* pubKey,Mona::UInt32 pubKeySize,const Mona::UInt8* farPubKey,Mona::UInt32 farPubKeySize,const Poco::Buffer<Mona::UInt8>& sharedSecret,RC4_KEY& decryptKey,RC4_KEY& encryptKey);

};


} // namespace Mona
