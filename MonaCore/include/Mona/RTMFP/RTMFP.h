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
#include "Mona/PacketReader.h"
#include "Mona/PacketWriter.h"
#include "Mona/Time.h"
#include <openssl/evp.h>
#include <math.h>

namespace Mona {

#define RTMFP_DEFAULT_KEY	(UInt8*)"Adobe Systems 02"
#define RTMFP_KEY_SIZE		0x10

#define RTMFP_HEADER_SIZE		11
#define RTMFP_MIN_PACKET_SIZE	(RTMFP_HEADER_SIZE+1)
#define RTMFP_MAX_PACKET_SIZE	1192
#define RTMFP_TIMESTAMP_SCALE	4


class RTMFPKey : virtual Object {
public:
	RTMFPKey(const UInt8* key) {memcpy(_key, key, RTMFP_KEY_SIZE);}
	const UInt8* value() { return _key; }

private:
	UInt8 _key[RTMFP_KEY_SIZE];	
};

class RTMFPEngine : virtual NullableObject {
public:
	enum Direction {
		DECRYPT=0,
		ENCRYPT
	};
	enum Type {
		NORMAL=0,
		DEFAULT
	};
	RTMFPEngine(const std::shared_ptr<RTMFPKey>& pKey,Direction direction);
	virtual ~RTMFPEngine();

	void		  process(const UInt8* in,UInt8* out,int size);

	Type		  type;
private:
	Direction						_direction;
	const std::shared_ptr<RTMFPKey> _pKey;
	EVP_CIPHER_CTX					_context;

	static const std::shared_ptr<RTMFPKey>	_pDefaultKey;
	static RTMFPEngine						_DefaultDecrypt;
	static RTMFPEngine						_DefaultEncrypt;
};


class RTMFP : virtual Static {
public:
	static UInt32				Unpack(PacketReader& packet);
	static void					Pack(PacketWriter& packet,UInt32 farId);

	static bool					ReadCRC(PacketReader& packet);
	static void					WriteCRC(PacketWriter& packet);
	static bool					Decode(Exception& ex,RTMFPEngine& aesDecrypt,PacketReader& packet);
	static void					Encode(RTMFPEngine& aesEncrypt,PacketWriter& packet);
	

	static void					ComputeAsymetricKeys(const Buffer& sharedSecret,
														const UInt8* initiatorNonce,UInt16 initNonceSize,
														const UInt8* responderNonce,UInt16 respNonceSize,
														 UInt8* requestKey,
														 UInt8* responseKey);

	static UInt16				TimeNow() { return Time(Mona::Time()); }
	static UInt16				Time(Int64 timeVal) { return (UInt32)round(timeVal / (1000.0*RTMFP_TIMESTAMP_SCALE)); }

private:
	static UInt16				CheckSum(PacketReader& packet);
};

}  // namespace Mona
