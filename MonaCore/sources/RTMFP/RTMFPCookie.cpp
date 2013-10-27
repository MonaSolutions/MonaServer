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

#include "Mona/RTMFP/RTMFPCookie.h"
#include "Mona/RTMFP/RTMFP.h"
#include "Mona/Logs.h"
#include <cstring>

using namespace std;
using namespace Poco;

namespace Mona {

RTMFPCookie::RTMFPCookie(RTMFPHandshake& handshake,Invoker& invoker,const string& tag,const string& queryUrl) : peerId(),_invoker(invoker), _pComputingThread(NULL),_pCookieComputing(new RTMFPCookieComputing(handshake,invoker)),tag(tag),id(0),farId(0),queryUrl(queryUrl),_writer(_buffer,sizeof(_buffer)),_initiatorNonce(0),decryptKey(),encryptKey() {
	_pComputingThread = invoker.poolThreads.enqueue<RTMFPCookieComputing>(_pCookieComputing, _pComputingThread);
}

void RTMFPCookie::finalize() {
	if(_writer.length()==0) {
		// It's our key public part
		int size = _pCookieComputing->diffieHellman.publicKeySize();
		_writer.write7BitLongValue(size+11);
		UInt8* nonce = _writer.begin() + _writer.position();
		_writer.writeRaw("\x03\x1A\x00\x00\x02\x1E\x00",7);
		UInt8 byte2 = DH_KEY_SIZE-size;
		if(byte2>2) {
			CRITIC("Generation DH key with less of 126 bytes!");
			byte2=2;
		}
		_writer.write8(0x81);
		_writer.write8(2-byte2);
		_writer.write8(0x0D);
		_writer.write8(0x02);
		_pCookieComputing->diffieHellman.writePublicKey(&nonce[11]);

		// Compute Keys
		RTMFP::ComputeAsymetricKeys(_pCookieComputing->sharedSecret,&_initiatorNonce[0],_initiatorNonce.size(),nonce,size+11,(UInt8*)decryptKey,(UInt8*)encryptKey);

		_writer.next(size);
		_writer.write8(0x58);
	}
}

UInt16 RTMFPCookie::read(MemoryWriter& writer) {
	writer.write32(id);
	writer.writeRaw(_writer.begin(),_writer.length());
	return _writer.length();
}


void RTMFPCookie::computeSecret(Exception& ex, const UInt8* initiatorKey,UInt32 sizeKey,const UInt8* initiatorNonce,UInt32 sizeNonce) {
	_pCookieComputing->initiatorKey.resize(sizeKey);
	memcpy(&_pCookieComputing->initiatorKey[0],initiatorKey,sizeKey);
	_initiatorNonce.resize(sizeNonce);
	memcpy(&_initiatorNonce[0],initiatorNonce,sizeNonce);

	_pCookieComputing->weak = _pCookieComputing;
	_pComputingThread = _invoker.poolThreads.enqueue<RTMFPCookieComputing>(_pCookieComputing, _pComputingThread);
}


} // namespace Mona
