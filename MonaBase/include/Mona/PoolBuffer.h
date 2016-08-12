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
#include "Mona/PoolBuffers.h"


namespace Mona {

class PoolBuffer : virtual public NullableObject, public Binary {
public:
	PoolBuffer(const PoolBuffers& poolBuffers,UInt32 size=0) : poolBuffers(poolBuffers),_pBuffer(NULL),_size(size) {}
	virtual ~PoolBuffer() { release(); }

	operator bool() const { return _pBuffer!=NULL;  }
	bool			empty() const { return !_pBuffer || _pBuffer->size()==0; }

	Buffer* operator->() const { if (!_pBuffer) _pBuffer=poolBuffers.beginBuffer(_size);  return _pBuffer; }
	Buffer& operator*() const { if (!_pBuffer) _pBuffer=poolBuffers.beginBuffer(_size);  return *_pBuffer; }

	
	PoolBuffer&	swap(PoolBuffer& pBuffer) { std::swap(pBuffer._pBuffer, _pBuffer); return *this; }
	void		release() { if (!_pBuffer) return; poolBuffers.endBuffer(_pBuffer); _pBuffer = NULL; }

	const PoolBuffers&	poolBuffers;

	// just to be a Binary object
	const UInt8*	data() const { return _pBuffer ? _pBuffer->data() : NULL; }
	UInt32			size() const { return _pBuffer ? _pBuffer->size() : 0; }
private:
	mutable Buffer*		_pBuffer;
	UInt32				_size;

};


} // namespace Mona
