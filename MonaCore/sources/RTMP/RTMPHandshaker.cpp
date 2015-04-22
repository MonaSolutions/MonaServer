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

#include "Mona/RTMP/RTMPHandshaker.h"
#include "Mona/DiffieHellman.h"
#include "Mona/Session.h"
#include "Mona/RTMP/RTMP.h"

using namespace std;


namespace Mona {

RTMPHandshaker::RTMPHandshaker(bool encrypted, const SocketAddress& address,PoolBuffer& pBuffer) : encrypted(encrypted), failed(false),_pBuffer(pBuffer.poolBuffers), TCPSender("RTMPHandshaker"),_address(address),_writer(pBuffer.poolBuffers) {
	_pBuffer.swap(pBuffer);
}

bool RTMPHandshaker::compute(Exception& ex) {
	Crypto::HMAC hmac;
	bool middle;
	UInt32 keySize;
	const UInt8* key = RTMP::ValidateClient(hmac,_pBuffer->data(),_pBuffer->size(),middle,keySize); // size = HMAC_KEY_SIZE

	if (!key) {
		if (encrypted) {
			ex.set(Exception::PROTOCOL,"Unable to validate client");
			return false;
		}

		/// Simple Handshake ///
		
		_writer.write8(3);
		//generate random data
		_writer.writeRandom(1536);
		// write data
		_writer.write(_pBuffer->data()+1,_pBuffer->size()-1);
	} else {

		/// Complexe Handshake ///

		if(encrypted) {
			pDecryptKey.reset(new RC4_KEY);
			pEncryptKey.reset(new RC4_KEY);
		}

		// encrypted flag
		_writer.write8(encrypted ? 6 : 3);

		//timestamp
		_writer.write32((UInt32)Time::Now());

		//version
		_writer.write32(0);

		//generate random data
		_writer.writeRandom(3064);

		if (encrypted) {

			UInt32 farPubKeySize;
			const UInt8* farPubKey = _pBuffer->data() + RTMP::GetDHPos(_pBuffer->data(),_pBuffer->size(), middle,farPubKeySize);

			//compute DH key position
			UInt32 serverDHSize;
			UInt32 serverDHPos = RTMP::GetDHPos(_writer.data(), _writer.size(), middle, serverDHSize);

			PoolBuffer pSecret(_pBuffer.poolBuffers);
			//generate DH key
			DiffieHellman dh;
			int publicKeySize;
			do {
				if (ex || !dh.initialize(ex, true))
					return false;
				dh.computeSecret(ex, farPubKey,farPubKeySize, *pSecret);
			} while (!ex && (pSecret->size() != DiffieHellman::SIZE || dh.privateKeySize(ex) != DiffieHellman::SIZE || (publicKeySize=dh.publicKeySize(ex)) != DiffieHellman::SIZE));
			if (ex)
				return false;
			RTMP::ComputeRC4Keys(hmac,dh.readPublicKey(ex, _writer.data() + serverDHPos), DiffieHellman::SIZE, farPubKey, farPubKeySize, *pSecret, *pDecryptKey, *pEncryptKey);
		}

		//generate the digest
		if (!RTMP::WriteDigestAndKey(ex, hmac, key, keySize, middle, _writer.data(), _writer.size()))
			return false;
	}

	Session::DumpResponse(encrypted ? "RTMPE" : "RTMP", data(), size(), _address,true);
	return TCPSender::run(ex);
}


} // namespace Mona
