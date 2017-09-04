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


#include "Mona/DiffieHellman.h"


using namespace std;

namespace Mona {

UInt8 DH1024p[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


DiffieHellman::DiffieHellman() : _pDH(NULL), _publicKeySize(0), _privateKeySize(0) {
	
}

DiffieHellman::~DiffieHellman() {
	if(_pDH)
		DH_free(_pDH);
}

UInt8*	DiffieHellman::readPublicKey(Exception& ex, UInt8* key) {
	if (!initialize(ex)) 
		return NULL; 
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	return readKey(_pDH->pub_key, key);
#else
	const BIGNUM *pubKey;
	DH_get0_key(_pDH, &pubKey, NULL);
	return readKey(pubKey, key);
#endif
}

UInt8*	DiffieHellman::readPrivateKey(Exception& ex, UInt8* key) { 
	if (!initialize(ex)) 
		return NULL; 
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	return readKey(_pDH->priv_key, key);
#else
	const BIGNUM *privKey;
	DH_get0_key(_pDH, NULL, &privKey);
	return readKey(privKey, key);
#endif
}

bool DiffieHellman::initialize(Exception& ex,bool reset) {
	if(!reset && _pDH)
		return true;
	if(_pDH)
		DH_free(_pDH);
	_pDH = DH_new();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	BIGNUM* p = _pDH->p = BN_new();
	BIGNUM* g = _pDH->g = BN_new();
#else
	BIGNUM* p = BN_new();
	BIGNUM* g = BN_new();
	DH_set0_pqg(_pDH, p, NULL, g);
#endif

	//3. initialize p, g and key length
	BN_set_word(g, 2); //group DH 2
	BN_bin2bn(DH1024p, SIZE, p); //prime number

	//4. Generate private and public key
	if (!DH_generate_key(_pDH)) {
		ex.set(Exception::MATH,"Generation DH key failed");
		DH_free(_pDH);
		_pDH = NULL;
	}

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	const BIGNUM *pubKey = _pDH->pub_key, *privKey = _pDH->priv_key;
#else
	const BIGNUM *pubKey, *privKey;
	DH_get0_key(_pDH, &pubKey, &privKey);
#endif
	_publicKeySize = BN_num_bytes(pubKey);
	_privateKeySize = BN_num_bytes(privKey);
	return !ex;
}


Buffer& DiffieHellman::computeSecret(Exception& ex, const UInt8* farPubKey, UInt32 farPubKeySize, Buffer& sharedSecret) {
	if (!initialize(ex))
		return sharedSecret;
	BIGNUM *bnFarPubKey = BN_bin2bn(farPubKey,farPubKeySize,NULL);
	sharedSecret.resize(SIZE,false);
	int size = DH_compute_key(sharedSecret.data(), bnFarPubKey, _pDH);
	if (size <= 0)
		ex.set(Exception::MATH, "Diffie Hellman exchange failed, DH compute key error");
	else if(size!=SIZE)
		sharedSecret.resize(size,true);
	BN_free(bnFarPubKey);
	return sharedSecret;
}



}  // namespace Mona
