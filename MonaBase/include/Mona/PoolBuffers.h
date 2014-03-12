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
#include "Mona/Buffer.h"
#include "Mona/Time.h"
#include <deque>
#include <mutex>

namespace Mona {

class PoolBuffers : virtual Object {
	friend class PoolBuffer;
public:
	PoolBuffers(UInt16 maximumCapacity = 32768);
	virtual ~PoolBuffers();

	void clear() { clear(false); }

private:
	void		clear(bool deleting);
	Buffer*		beginBuffer(UInt32 size=0) const;
	void		endBuffer(Buffer* pBuffer) const;

				
	mutable std::deque<Buffer*>	_buffers;
	mutable std::mutex			_mutex;
	UInt16						_maximumCapacity;
	mutable Time				_lastEmptyTime;
};


} // namespace Mona
