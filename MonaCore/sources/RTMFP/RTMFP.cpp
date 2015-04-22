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

#include "Mona/RTMFP/RTMFP.h"
#include "Mona/Crypto.h"


using namespace std;


namespace Mona {



BinaryWriter& RTMFP::WriteAddress(BinaryWriter& writer,const SocketAddress& address,AddressType type) {
	const IPAddress& host = address.host();
	if (host.family() == IPAddress::IPv6)
		writer.write8(type | 0x80);
	else
		writer.write8(type);
	NET_SOCKLEN size(host.size());
	const UInt8* bytes = (const UInt8*)host.addr();
	for(NET_SOCKLEN i=0;i<size;++i)
		writer.write8(bytes[i]);
	return writer.write16(address.port());
}

UInt32 RTMFP::Unpack(BinaryReader& reader) {
	reader.reset();
	UInt32 id=0;
	for(int i=0;i<3;++i)
		id ^= reader.read32();
	reader.reset(4);
	return id;
}

void RTMFP::Pack(BinaryWriter& writer,UInt32 farId) {
	BinaryReader reader(writer.data()+4,writer.size()-4);
	BinaryWriter(writer.data(),4).write32(reader.read32()^reader.read32()^farId);
}


void RTMFP::ComputeAsymetricKeys(const Buffer& sharedSecret, const UInt8* initiatorNonce,UInt16 initNonceSize,
														    const UInt8* responderNonce,UInt16 respNonceSize,
														    UInt8* requestKey,UInt8* responseKey) {
	UInt8 mdp1[Crypto::HMAC::SIZE];
	UInt8 mdp2[Crypto::HMAC::SIZE];
	Crypto::HMAC hmac;

	// doing HMAC-SHA256 of one side
	hmac.compute(EVP_sha256(),responderNonce,respNonceSize,initiatorNonce,initNonceSize,mdp1);
	// doing HMAC-SHA256 of the other side
	hmac.compute(EVP_sha256(),initiatorNonce,initNonceSize,responderNonce,respNonceSize,mdp2);

	// now doing HMAC-sha256 of both result with the shared secret DH key
	hmac.compute(EVP_sha256(),sharedSecret.data(),sharedSecret.size(),mdp1,Crypto::HMAC::SIZE,requestKey);
	hmac.compute(EVP_sha256(),sharedSecret.data(),sharedSecret.size(),mdp2,Crypto::HMAC::SIZE,responseKey);
}



}  // namespace Mona
