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

#include "Mona/WebSocket/WS.h"
#include "Mona/Logs.h"
#include "Poco/Base64Encoder.h"
#include <openssl/evp.h>
#include <sstream>

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

void WS::ComputeKey(string& key) {
	key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11"); // WEBSOCKET_GUID
	UInt8 temp[20];
	EVP_Digest(key.c_str(),key.size(),temp,NULL,EVP_sha1(),NULL);
	ostringstream ostr;
	Base64Encoder(ostr).write((const char*)temp, sizeof(temp));
	key.assign(ostr.str());
}



void WS::Unmask(MemoryReader& data) {
	UInt32 i;
	UInt8 mask[4];
	data.readRaw(mask,sizeof(mask));
	UInt8* bytes = data.current();
	for(i=0;i<data.available();++i)
		bytes[i] ^= mask[i%4];
}

UInt8 WS::WriteHeader(UInt8 type,UInt32 size,BinaryWriter& writer) {
	if(type==0)
		return 0;
	writer.write8(type|0x80);
	if (size < 126) {
		writer.write8(size);
		return 2;
	}
	if (size < 65536) {
		writer.write8(126);
		writer.write16(size);
		return 4;
	}
	writer.write8(127);
	writer.write64(size);
	return 10;
}

UInt8 WS::HeaderSize(UInt32 size) {
	if (size < 126)
		return 2;
	if (size < 65536)
		return 4;
	return 10;
}




	




} // namespace Mona
