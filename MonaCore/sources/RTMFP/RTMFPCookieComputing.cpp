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

#include "Mona/RTMFP/RTMFPCookieComputing.h"
#include "Mona/RTMFP/RTMFPHandshake.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"

using namespace std;


namespace Mona {

RTMFPCookieComputing::RTMFPCookieComputing(RTMFPHandshake& handshake,Invoker& invoker): WorkThread("RTMFPCookieComputing"),_handshake(handshake),Task(invoker),packet(invoker.poolBuffers) {
	Util::Random(value, COOKIE_SIZE);
}

bool RTMFPCookieComputing::run(Exception& ex) {
	// First execution is for the DH computing if pDH == null, else it's to compute Diffie-Hellman keys
	if (!_diffieHellman.initialized())
		return _diffieHellman.initialize(ex);

	// Compute Diffie-Hellman secret
	_diffieHellman.computeSecret(ex,initiatorKey.data(),initiatorKey.size(),_sharedSecret);
	if (ex)
		return false;

	if (packet.size() > 0) {
		ex.set(Exception::CRYPTO, "RTMFPCookieComputing already executed");
		return false;
	}

	string hex;
	DEBUG("Shared Secret : ", Util::FormatHex(_sharedSecret.data(), _sharedSecret.size(), hex));

	// It's our key public part
	int size = _diffieHellman.publicKeySize(ex);
	if (ex)
		return false;
	packet.write7BitLongValue(size+11);
	UInt32 noncePos = packet.size();
	packet.writeRaw(EXPAND_DATA_SIZE("\x03\x1A\x00\x00\x02\x1E\x00"));
	UInt8 byte2 = DH_KEY_SIZE-size;
	if(byte2>2) {
		CRITIC("Generation DH key with less of 126 bytes!");
		byte2=2;
	}
	packet.write8(0x81);
	packet.write8(2-byte2);
	packet.write8(0x0D);
	packet.write8(0x02);
	_diffieHellman.readPublicKey(ex,packet.buffer(size));
	packet.write8(0x58);

	// Compute Keys
	RTMFP::ComputeAsymetricKeys(_sharedSecret,initiatorNonce.data(),initiatorNonce.size(),packet.data()+noncePos,size+11,decryptKey,encryptKey);
	
	waitHandle();
	return true;
}


void RTMFPCookieComputing::handle(Exception& ex) {
	RTMFPSession* pSession = _handshake.createSession(value);
	if (pSession)
		pSession->pRTMFPCookieComputing = weak.lock();
}


} // namespace Mona
