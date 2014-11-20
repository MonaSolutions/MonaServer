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

RTMPHandshaker::RTMPHandshaker(const SocketAddress& address,PoolBuffer& pBuffer) : failed(false),_pBuffer(pBuffer.poolBuffers), TCPSender("RTMPHandshaker"),_address(address),_writer(pBuffer.poolBuffers) {
	_pBuffer.swap(pBuffer);
}

bool RTMPHandshaker::compute(Exception& ex) {
	BinaryReader packet(_pBuffer->data(),_pBuffer->size());

	UInt8 handshakeType = packet.read8();
	if(handshakeType!=3 && handshakeType!=6) {
		ex.set(Exception::PROTOCOL,"RTMP Handshake type '",handshakeType,"' unknown");
		return false;
	}

	Crypto::HMAC hmac;
	bool encrypted(handshakeType == 6);
	bool middle;
	const UInt8* challengeKey = RTMP::ValidateClient(hmac,packet,middle); // size = HMAC_KEY_SIZE

	if (!challengeKey) {
		if (encrypted) {
			ex.set(Exception::PROTOCOL,"Unable to validate client");
			return false;
		}

		/// Simple Handshake ///
		
		_writer.write8(3);
		//generate random data
		_writer.writeRandom(1536);
		// write data
		_writer.write(packet.current(),packet.available());
	} else {

		/// Complexe Handshake ///

		if(encrypted) {
			pDecryptKey.reset(new RC4_KEY);
			pEncryptKey.reset(new RC4_KEY);
		}
		packet.reset();

		const UInt8* farPubKey = packet.current() + RTMP::GetDHPos(packet.current(), middle); // size = DH_KEY_SIZE

		// encrypted flag
		_writer.write8(encrypted ? 6 : 3);

		//timestamp
		_writer.write32((UInt32)Time::Now());

		//version
		_writer.write32(0);

		//generate random data
		_writer.writeRandom(3064);

		if (encrypted) {

			//compute DH key position
			UInt32 serverDHPos = RTMP::GetDHPos(_writer.data(), middle);

			PoolBuffer pSecret(_pBuffer.poolBuffers);
			//generate DH key
			DiffieHellman dh;
			int publicKeySize;
			do {
				if (ex || !dh.initialize(ex, true))
					return false;
				dh.computeSecret(ex, farPubKey,DH_KEY_SIZE, *pSecret);
			} while (!ex && (pSecret->size() != DH_KEY_SIZE || dh.privateKeySize(ex) != DH_KEY_SIZE || (publicKeySize=dh.publicKeySize(ex)) != DH_KEY_SIZE));
		
			if (ex)
				return false;

			dh.readPublicKey(ex,(UInt8*)_writer.data() + serverDHPos);
			if (ex)
				return false;

			RTMP::ComputeRC4Keys(hmac,_writer.data() + serverDHPos, publicKeySize, farPubKey, DH_KEY_SIZE, *pSecret, *pDecryptKey, *pEncryptKey);
		}

		//generate the digest
		RTMP::WriteDigestAndKey(hmac,(UInt8*)_writer.data(),challengeKey,middle);
	}

	Session::DumpResponse(data(), size(), _address,true);
	return TCPSender::run(ex);
}


} // namespace Mona
