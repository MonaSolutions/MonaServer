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
#include "Mona/SocketAddress.h"
#include "Mona/BinaryReader.h"
#include "Mona/BinaryWriter.h"
#include "Mona/Time.h"
#include <openssl/evp.h>

namespace Mona {

#define RTMFP_DEFAULT_KEY	(UInt8*)"Adobe Systems 02"
#define RTMFP_KEY_SIZE		0x10

#define RTMFP_HEADER_SIZE		11
#define RTMFP_MIN_PACKET_SIZE	(RTMFP_HEADER_SIZE+1)
#define RTMFP_MAX_PACKET_SIZE	1192
#define RTMFP_TIMESTAMP_SCALE	4

class RTMFPEngine : public virtual Object {
public:
	enum Direction {
		DECRYPT=0,
		ENCRYPT
	};
	RTMFPEngine(const UInt8* key, Direction direction) : _direction(direction), _context(EVP_CIPHER_CTX_new()) {
		memcpy(_key, key, RTMFP_KEY_SIZE);
		EVP_CIPHER_CTX_init(_context);
	}
	virtual ~RTMFPEngine() {
		EVP_CIPHER_CTX_cleanup(_context);
		EVP_CIPHER_CTX_free(_context);
	}

	void process(UInt8* data, int size) {
		static UInt8 IV[RTMFP_KEY_SIZE];
		EVP_CipherInit_ex(_context, EVP_aes_128_cbc(), NULL, _key, IV,_direction);
		EVP_CipherUpdate(_context, data, &size, data, size);
	}

private:
	Direction				_direction;
	UInt8					_key[RTMFP_KEY_SIZE];
	EVP_CIPHER_CTX*			_context;
};


class RTMFP : virtual Static {
public:
	enum AddressType {
		ADDRESS_UNSPECIFIED=0,
		ADDRESS_LOCAL=1,
		ADDRESS_PUBLIC=2,
		ADDRESS_REDIRECTION=3
	};

	static BinaryWriter&		WriteAddress(BinaryWriter& writer, const SocketAddress& address, AddressType type=ADDRESS_UNSPECIFIED);

	static UInt32				Unpack(BinaryReader& reader);
	static void					Pack(BinaryWriter& writer,UInt32 farId);

	static void					ComputeAsymetricKeys(const Buffer& sharedSecret,
														const UInt8* initiatorNonce,UInt16 initNonceSize,
														const UInt8* responderNonce,UInt16 respNonceSize,
														 UInt8* requestKey,
														 UInt8* responseKey);

	static UInt16				TimeNow() { return Time(Mona::Time::Now()); }
	static UInt16				Time(Int64 timeVal) { return (timeVal / RTMFP_TIMESTAMP_SCALE)&0xFFFF; }

};

}  // namespace Mona
