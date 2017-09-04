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

UInt8* Crypto::HMAC::compute(const EVP_MD* evpMD, const void* key, int keySize, const UInt8* data, size_t size, UInt8* value) {
	HMAC_Init_ex(get(),key, keySize, evpMD, NULL);
	HMAC_Update(get(), data, size);
	HMAC_Final(get(), value,NULL);
	return value;
}

UInt16 Crypto::ComputeCRC(BinaryReader& reader) {
	int sum = 0;
	UInt32 pos(reader.position());
	while(reader.available()>0)
		sum += reader.available()==1 ? reader.read8() : reader.read16();
	reader.reset(pos);

  /* add back carry outs from top 16 bits to low 16 bits */
  sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
  sum += (sum >> 16);                     /* add carry */
  return ~sum; /* truncate to 16 bits */
}

} // namespace Mona
