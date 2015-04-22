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
#include "Mona/TCPSender.h"
#include "Mona/PacketWriter.h"
#include <openssl/rc4.h>

namespace Mona {

class RTMPHandshaker : public TCPSender, public virtual Object {
public:
	RTMPHandshaker(bool encrypted, const SocketAddress& address,PoolBuffer& pBuffer);

	const bool encrypted;
	volatile bool failed;

	std::shared_ptr<RC4_KEY>	pEncryptKey;
	std::shared_ptr<RC4_KEY>    pDecryptKey;

private:

	bool run(Exception& ex) {return !(failed = !compute(ex)); }
	bool compute(Exception& ex);

	const UInt8*	data() const { return _writer.data(); }
	UInt32			size() const { return _writer.size(); }

	PacketWriter			_writer;

	SocketAddress				_address;
	PoolBuffer					_pBuffer;
};


} // namespace Mona
