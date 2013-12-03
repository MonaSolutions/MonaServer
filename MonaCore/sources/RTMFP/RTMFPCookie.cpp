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

#include "Mona/RTMFP/RTMFPCookie.h"
#include "Mona/RTMFP/RTMFP.h"
#include "Mona/Logs.h"
#include <cstring>

using namespace std;


namespace Mona {

RTMFPCookie::RTMFPCookie(RTMFPHandshake& handshake,Invoker& invoker,const string& tag,const string& queryUrl) : _invoker(invoker), _pComputingThread(NULL),_pCookieComputing(new RTMFPCookieComputing(handshake,invoker)),tag(tag),id(0),farId(0),queryUrl(queryUrl),_writer(_buffer,sizeof(_buffer)),_initiatorNonce(0) {
	
}


bool RTMFPCookie::finalize(Exception& ex) {
	if (_writer.length() > 0)
		return true;
	// It's our key public part
	int size = _pCookieComputing->diffieHellman.publicKeySize(ex);
	if (ex)
		return false;
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
	_pCookieComputing->diffieHellman.readPublicKey(ex,&nonce[11]);

	// Compute Keys
	RTMFP::ComputeAsymetricKeys(_pCookieComputing->sharedSecret,&_initiatorNonce[0],_initiatorNonce.size(),nonce,size+11,decryptKey,encryptKey);

	_writer.next(size);
	_writer.write8(0x58);
	return true;
}

UInt16 RTMFPCookie::read(MemoryWriter& writer) {
	writer.write32(id);
	writer.writeRaw(_writer.begin(),_writer.length());
	return _writer.length();
}


bool RTMFPCookie::computeSecret(Exception& ex, const UInt8* initiatorKey,UInt32 sizeKey,const UInt8* initiatorNonce,UInt32 sizeNonce) {
	_pCookieComputing->initiatorKey.resize(sizeKey,false);
	memcpy(&_pCookieComputing->initiatorKey[0],initiatorKey,sizeKey);
	_initiatorNonce.resize(sizeNonce,false);
	memcpy(&_initiatorNonce[0],initiatorNonce,sizeNonce);
	_pCookieComputing->weak = _pCookieComputing;
	_pComputingThread = _invoker.poolThreads.enqueue<RTMFPCookieComputing>(ex,_pCookieComputing, _pComputingThread);
	return !ex;
}


} // namespace Mona
