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
#include "Mona/TCPSender.h"
#include "Mona/MemoryWriter.h"
#include "Poco/Buffer.h"
#include "Poco/SharedPtr.h"
#include <openssl/rc4.h>

namespace Mona {

class RTMPHandshaker : public TCPSender {
public:
	RTMPHandshaker(const UInt8* data, UInt32 size,SocketHandler<Poco::Net::StreamSocket>& handler);
	RTMPHandshaker(const UInt8* farPubKey,const UInt8* challengeKey,bool middle,const Poco::SharedPtr<RC4_KEY>& pDecryptKey,const Poco::SharedPtr<RC4_KEY>& pEncryptKey,SocketHandler<Poco::Net::StreamSocket>& handler);
	virtual ~RTMPHandshaker();

private:
	
	bool flush();
	void flushComplex();

	const UInt8*	begin(bool displaying=false);
	UInt32		size(bool displaying=false);

	UInt8					_buffer[3073];
	MemoryWriter				_writer;
	Poco::Buffer<UInt8>	_farPubKey;
	UInt8					_challengeKey[HMAC_KEY_SIZE];	
	bool						_middle;
	Poco::SharedPtr<RC4_KEY>	_pEncryptKey;
	Poco::SharedPtr<RC4_KEY>	_pDecryptKey;
};

inline const UInt8* RTMPHandshaker::begin(bool displaying) {
	return _writer.begin();
}

inline UInt32 RTMPHandshaker::size(bool displaying) {
	return _writer.length();
}


} // namespace Mona
