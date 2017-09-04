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
#include "Mona/Exceptions.h"
#include "Mona/Buffer.h"
#include <openssl/dh.h>


namespace Mona {

class DiffieHellman : public virtual Object {
public:
	enum { SIZE = 0x80 };

	DiffieHellman();
	virtual ~DiffieHellman();

	bool	initialized() { return _pDH != NULL; }
	bool	initialize(Exception& ex,bool reset=false);

	int		publicKeySize(Exception& ex) { return _publicKeySize; }
	UInt8*	readPublicKey(Exception& ex, UInt8* key);

	int		privateKeySize(Exception& ex) { return _privateKeySize; }
	UInt8*	readPrivateKey(Exception& ex, UInt8* key);

	Buffer&	computeSecret(Exception& ex, const UInt8* farPubKey, UInt32 farPubKeySize, Buffer& sharedSecret);

private:
	UInt8*	readKey(const BIGNUM *pKey, UInt8* key) const { BN_bn2bin(pKey, key); return key; }

	UInt8	_publicKeySize;
	UInt8	_privateKeySize;

	DH*			_pDH;
};


} // namespace Mona
