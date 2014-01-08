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
#include <openssl/hmac.h>
#include <math.h>


using namespace std;


namespace Mona {

#define RTMFP_KEY_SIZE	0x20


RTMFPEngine RTMFPEngine::_DefaultDecrypt(RTMFP_SYMETRIC_KEY,RTMFPEngine::DECRYPT);
RTMFPEngine RTMFPEngine::_DefaultEncrypt(RTMFP_SYMETRIC_KEY,RTMFPEngine::ENCRYPT);

RTMFPEngine::RTMFPEngine() : type(EMPTY),_direction(DECRYPT) {
}

RTMFPEngine::RTMFPEngine(const UInt8* key,Direction direction) : type(key ? DEFAULT : EMPTY),_direction(direction) {
	if(!key)
		return;
	if(_direction==DECRYPT)
		AES_set_decrypt_key(key, 0x80,&_key);
	else
		AES_set_encrypt_key(key, 0x80,&_key);
}

RTMFPEngine::RTMFPEngine(const RTMFPEngine& engine,Type type) : type(engine.type==EMPTY ? EMPTY : type),_key(engine._key),_direction(engine._direction) {
}

RTMFPEngine::RTMFPEngine(const RTMFPEngine& engine) : type(engine.type),_key(engine._key),_direction(engine._direction) {
}

void RTMFPEngine::set(const RTMFPEngine& engine,Type type) {
	(Type&)this->type = type;
	_key = engine._key;
	_direction = engine._direction;
}


void RTMFPEngine::process(const UInt8* in,UInt8* out,UInt32 size) {
	if(type==EMPTY)
		return;
	if(type==SYMMETRIC) {
		if(_direction==DECRYPT)
			_DefaultDecrypt.process(in,out,size);
		else
			_DefaultEncrypt.process(in,out,size);
		return;
	}
	UInt8	iv[RTMFP_KEY_SIZE];
	memset(iv,0,sizeof(iv));
	AES_cbc_encrypt(in, out,size,&_key,iv, _direction);
}




UInt16 RTMFP::CheckSum(PacketReader& packet) {
	int sum = 0;
	int pos = packet.position();
	while(packet.available()>0)
		sum += packet.available()==1 ? packet.read8() : packet.read16();
	packet.reset(pos);

  /* add back carry outs from top 16 bits to low 16 bits */
  sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
  sum += (sum >> 16);                     /* add carry */
  return ~sum; /* truncate to 16 bits */
}


bool RTMFP::Decode(Exception& ex,RTMFPEngine& aesDecrypt,PacketReader& packet) {
	// Decrypt
	aesDecrypt.process(packet.current(),(UInt8*)packet.current(),packet.available());
	bool result = ReadCRC(packet);
	if (!result)
		ex.set(Exception::CRYPTO, "Bad RTMFP CRC sum computing");
	return result;
}

bool RTMFP::ReadCRC(PacketReader& packet) {
	// Check the first 2 CRC bytes 
	packet.reset(4);
	UInt16 sum = packet.read16();
	return (sum == CheckSum(packet));
}


void RTMFP::Encode(RTMFPEngine& aesEncrypt,PacketWriter& packet) {
	if(aesEncrypt.type != RTMFPEngine::EMPTY) {
		// paddingBytesLength=(0xffffffff-plainRequestLength+5)&0x0F
		int paddingBytesLength = (0xFFFFFFFF-packet.size()+5)&0x0F;
		// Padd the plain request with paddingBytesLength of value 0xff at the end
		while (paddingBytesLength-->0)
			packet.write8(0xFF);
	}

	WriteCRC(packet);
	
	// Encrypt the resulted request
	aesEncrypt.process(packet.data()+4,(UInt8*)packet.data()+4,packet.size()-4);
}

void RTMFP::WriteCRC(PacketWriter& packet) {
	// Compute the CRC and add it at the beginning of the request
	PacketReader reader(packet.data(),packet.size());
	reader.next(6);
	UInt16 sum = CheckSum(reader);
	BinaryWriter(packet,4).write16(sum);
}

UInt32 RTMFP::Unpack(PacketReader& packet) {
	packet.reset();
	UInt32 id=0;
	for(int i=0;i<3;++i)
		id ^= packet.read32();
	packet.reset(4);
	return id;
}

void RTMFP::Pack(PacketWriter& packet,UInt32 farId) {
	PacketReader reader(packet.data(),packet.size());
	reader.next(4);
	BinaryWriter(packet).write32(reader.read32()^reader.read32()^farId);
}


void RTMFP::ComputeAsymetricKeys(const Buffer& sharedSecret, const UInt8* initiatorNonce,UInt16 initNonceSize,
														    const UInt8* responderNonce,UInt16 respNonceSize,
														    UInt8* requestKey,UInt8* responseKey) {
	UInt8 mdp1[HMAC_KEY_SIZE];
	UInt8 mdp2[HMAC_KEY_SIZE];

	// doing HMAC-SHA256 of one side
	HMAC(EVP_sha256(),responderNonce,respNonceSize,initiatorNonce,initNonceSize,mdp1,NULL);
	// doing HMAC-SHA256 of the other side
	HMAC(EVP_sha256(),initiatorNonce,initNonceSize,responderNonce,respNonceSize,mdp2,NULL);

	// now doing HMAC-sha256 of both result with the shared secret DH key
	HMAC(EVP_sha256(),sharedSecret.data(),sharedSecret.size(),mdp1,HMAC_KEY_SIZE,requestKey,NULL);
	HMAC(EVP_sha256(),sharedSecret.data(),sharedSecret.size(),mdp2,HMAC_KEY_SIZE,responseKey,NULL);
}



}  // namespace Mona
