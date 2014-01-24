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

#include "Mona/Crypto.h"

using namespace std;

namespace Mona {


UInt8* Crypto::hmac(const EVP_MD* evpMD, const void* key, int keySize, const UInt8* data, size_t size, UInt8* value) {
	HMAC_Init_ex(&_hmacCTX,key, keySize, evpMD, NULL);
	HMAC_Update(&_hmacCTX, data, size);
	HMAC_Final(&_hmacCTX, value,NULL);
	return value;
}

} // namespace Mona
