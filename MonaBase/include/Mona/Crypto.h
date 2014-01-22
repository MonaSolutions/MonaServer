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
#include <openssl/hmac.h>
#include <mutex>
#include <deque>

namespace Mona {

class Crypto : virtual Object {
public:

	Crypto() { HMAC_CTX_init(&_hmacCTX);}
	~Crypto() { HMAC_CTX_cleanup(&_hmacCTX); }

	UInt8* hmac(const EVP_MD* evpMD, const void* key, int keySize, const UInt8* data, size_t size, UInt8* value);

private:
	HMAC_CTX _hmacCTX;
};


} // namespace Mona
