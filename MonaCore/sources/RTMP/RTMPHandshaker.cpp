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
#include "Mona/Writer.h"
#include "Mona/Logs.h"
#include "Mona/RTMP/RTMP.h"
#include "Mona/Util.h"
#include "Mona/Time.h"
#include <cstring>

using namespace std;



namespace Mona {

RTMPHandshaker::RTMPHandshaker(const PoolBuffers& poolBuffers,const SocketAddress& address, const UInt8* data, UInt32 size) : _poolBuffers(poolBuffers),_pFarPubKey(poolBuffers), TCPSender("RTMPHandshaker"),_address(address), _middle(false),_writer(poolBuffers) {
	_writer.write8(3);
	//generate random data
	_writer.writeRandom(1536);
	// write data
	_writer.writeRaw(data,size);
}

RTMPHandshaker::RTMPHandshaker(const PoolBuffers& poolBuffers,const SocketAddress& address, const UInt8* farPubKey, const UInt8* challengeKey, bool middle, const std::shared_ptr<RC4_KEY>& pDecryptKey, const std::shared_ptr<RC4_KEY>& pEncryptKey) : _poolBuffers(poolBuffers),_pFarPubKey(poolBuffers,DH_KEY_SIZE),TCPSender("RTMPHandshaker"),_address(address), _pDecryptKey(pDecryptKey), _pEncryptKey(pEncryptKey), _middle(middle), _writer(poolBuffers) {
	memcpy(_pFarPubKey->data(),farPubKey,_pFarPubKey->size());
	memcpy(_challengeKey,challengeKey,sizeof(_challengeKey));
}


bool RTMPHandshaker::run(Exception& ex) {
	if (_writer.size() == 0) {
		if (!runComplex(ex))
			return false;
	}
	Writer::DumpResponse(data(), size(), _address,true);
	return TCPSender::run(ex);
}

bool RTMPHandshaker::runComplex(Exception& ex) {

	bool encrypted = (_pDecryptKey && _pEncryptKey);

	// encrypted flag
	_writer.write8(encrypted ? 6 : 3);

	//timestamp
	_writer.write32((UInt32)Time());

	//version
	_writer.write32(0);

	//generate random data
	_writer.writeRandom(3064);

	if (encrypted) {

		//compute DH key position
		UInt32 serverDHPos = RTMP::GetDHPos(_writer.data(), _middle);

		PoolBuffer pSecret(_poolBuffers);
		//generate DH key
		DiffieHellman dh;
		int publicKeySize;
		do {
			if (ex || !dh.initialize(ex, true))
				return false;
			dh.computeSecret(ex, *_pFarPubKey, *pSecret);
		} while (!ex && (pSecret->size() != DH_KEY_SIZE || dh.privateKeySize(ex) != DH_KEY_SIZE || (publicKeySize=dh.publicKeySize(ex)) != DH_KEY_SIZE));
		
		if (ex)
			return false;

		dh.readPublicKey(ex,(UInt8*)_writer.data() + serverDHPos);
		if (ex)
			return false;

		RTMP::ComputeRC4Keys(_writer.data() + serverDHPos, publicKeySize, _pFarPubKey->data(), _pFarPubKey->size(), *pSecret, *_pDecryptKey, *_pEncryptKey);
	}

	//generate the digest
	RTMP::WriteDigestAndKey((UInt8*)_writer.data(),_challengeKey,_middle);
	return true;
}


} // namespace Mona
