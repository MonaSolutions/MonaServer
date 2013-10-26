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

#include "Mona/RTMP/RTMPHandshaker.h"
#include "Mona/DiffieHellman.h"
#include "Mona/Logs.h"
#include "Mona/RTMP/RTMP.h"
#include "Poco/RandomStream.h"
#include "Mona/Time.h"
#include <cstring>

using namespace std;
using namespace Poco;


namespace Mona {

RTMPHandshaker::RTMPHandshaker(const UInt8* data, UInt32 size): TCPSender(NULL,true),_middle(false),_writer(_buffer,sizeof(_buffer)),_farPubKey(0) {
	_writer.write8(3);
	//generate random data
	RandomInputStream().read((char*)_writer.begin()+_writer.position(),1536);
	_writer.next(1536);
	_writer.writeRaw(data,size);
}

RTMPHandshaker::RTMPHandshaker(const UInt8* farPubKey,const UInt8* challengeKey,bool middle,const Poco::SharedPtr<RC4_KEY>& pDecryptKey,const Poco::SharedPtr<RC4_KEY>& pEncryptKey): TCPSender(NULL,true),_pDecryptKey(pDecryptKey),_pEncryptKey(pEncryptKey),_middle(middle),_writer(_buffer,sizeof(_buffer)),_farPubKey(DH_KEY_SIZE) {
	memcpy(_farPubKey.data(),farPubKey,_farPubKey.size());
	memcpy(_challengeKey,challengeKey,sizeof(_challengeKey));
}


bool RTMPHandshaker::run(Exception& ex) {
	if(_writer.length()==0)
		runComplex();
	dump(true);
	return run(ex);
}

void RTMPHandshaker::runComplex() {

	bool encrypted = (!_pDecryptKey.isNull() && !_pEncryptKey.isNull());

	// encrypted flag
	_writer.write8(encrypted ? 6 : 3);

	//timestamp
	_writer.write32((UInt32)Time());

	//version
	_writer.write32(0);

	//generate random data
	RandomInputStream().read((char*)_writer.begin()+_writer.position(),3064);
	_writer.clear(3073);

	
	if (encrypted) {

		//compute DH key position
		UInt32 serverDHPos = RTMP::GetDHPos(_writer.begin(), _middle);

		Buffer<UInt8> secret(0);
		//generate DH key
		DiffieHellman dh;
		do {
			dh.initialize(true);
			dh.computeSecret(_farPubKey,secret);
		} while(secret.size()!=DH_KEY_SIZE || dh.privateKeySize()!=DH_KEY_SIZE || dh.publicKeySize()!=DH_KEY_SIZE);
		
		dh.writePublicKey(_writer.begin()+serverDHPos);

		RTMP::ComputeRC4Keys(_writer.begin()+serverDHPos,dh.publicKeySize(),_farPubKey.data(),_farPubKey.size(),secret,*_pDecryptKey,*_pEncryptKey);
	}

	//generate the digest
	RTMP::WriteDigestAndKey(_writer.begin(),_challengeKey,_middle);
}


} // namespace Mona
