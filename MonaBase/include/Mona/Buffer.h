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
#include <cstring>

namespace Mona {

template<typename ElementType>
class Buffer : virtual Object {
public:
	Buffer(UInt32 size = 0) : _pBuffer(new ElementType[size]()), _size(size),_originalSize(size) {}
	virtual ~Buffer() { delete [] _pBuffer; }

	UInt32	size() const { return _size; }

	void resetSize() { _size = _originalSize; }

	void resize(UInt32 size,bool preserveContent = true) {
		if (size == _originalSize)
			return;
		if (size<_originalSize) {
			_size = size;
			return;
		}
		ElementType* pBuffer = new ElementType[size]();
		if (preserveContent)
			std::memcpy(pBuffer, _pBuffer, size > _originalSize ? _originalSize : size);
		delete [] _pBuffer;
		_pBuffer = pBuffer;
		_originalSize = _size = size;
	}

	const ElementType* data() const { return _originalSize > 0 ? &_pBuffer[0] : NULL; }
	ElementType* data() { return _originalSize > 0 ? &_pBuffer[0] : NULL; }

	ElementType& operator [] (UInt32 index) {
		FATAL_ASSERT(index < _originalSize)
		return _pBuffer[index];
	}

	const ElementType& operator [] (UInt32 index) const {
		FATAL_ASSERT(index < _originalSize)
		return _pBuffer[index];
	}

private:
	ElementType*	_pBuffer;
	UInt32			_size;
	UInt32			_originalSize;
};


} // namespace Mona
