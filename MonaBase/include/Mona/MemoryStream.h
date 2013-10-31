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

#pragma once

#include "Mona/Mona.h"
#include <streambuf>
#include <iostream>

namespace Mona {


class MemoryStreamBuf;
class ScopedMemoryClip : virtual Object {
public:
	ScopedMemoryClip(MemoryStreamBuf& buffer,UInt32 offset);
	virtual ~ScopedMemoryClip();
private:
	UInt32				_offset;
	MemoryStreamBuf&   _buffer;
};


class MemoryStreamBuf: public std::streambuf {
	friend class ScopedMemoryClip;
public:
	MemoryStreamBuf(char* pBuffer,UInt32 bufferSize);
	MemoryStreamBuf(MemoryStreamBuf&);

	
	void			next(UInt32 count);
	UInt32			written();
	void			written(UInt32 size) { _written = size; }
	UInt32			size() { return _bufferSize; }
	void			resize(UInt32 newSize);
	char*			begin() { return _pBuffer; }
	void			position(UInt32 pos=0);
	char*			gCurrent() { return gptr(); }
	char*			pCurrent() { return pptr(); }
	
	void			clip(UInt32 offset) { clip((Int32)offset); }

private:
	void			clip(Int32 offset);

	virtual int		overflow(int_type c) { return EOF; }
	virtual int		underflow() { return EOF; }
	virtual int		sync() { return 0; }

	UInt32	_written;
	char*	_pBuffer;
	UInt32	_bufferSize;
};

//////////

class MemoryIOS: virtual Object, public virtual std::ios {
public:
	MemoryIOS(char* pBuffer, UInt32 bufferSize) : _buf(pBuffer, bufferSize) {}
		/// Creates the basic stream.
	// MemoryIOS(MemoryIOS&) : :_buf(other._buf) {}

	MemoryStreamBuf* rdbuf() { return &_buf; }
		/// Returns a pointer to the underlying streambuf.

	virtual char*	current()=0;
	void			reset(UInt32 newPos);
	void			resize(UInt32 newSize) { rdbuf()->resize(newSize); }
	char*			begin() { return rdbuf()->begin(); }
	void			next(UInt32 count) { rdbuf()->next(count); }
	UInt32			available();
	void			clip(UInt32 offset) { rdbuf()->clip(offset); }
		
private:
	MemoryStreamBuf _buf;
};

//////////

class MemoryInputStream: virtual Object, public MemoryIOS, public std::istream {
public:
	MemoryInputStream(const char* pBuffer, UInt32 bufferSize) : MemoryIOS(const_cast<char*>(pBuffer), bufferSize), std::istream(rdbuf()) {}
		/// Creates a MemoryInputStream for the given memory area,
		/// ready for reading.
	//MemoryInputStream(MemoryInputStream& other) : MemoryIOS(other), istream(rdbuf()) {}
		/// Destroys the MemoryInputStream.
	char*		current() { return rdbuf()->gCurrent(); }
};


//////////

class MemoryOutputStream: virtual Object, public MemoryIOS, public std::ostream {
public:
	MemoryOutputStream(char* pBuffer, UInt32 bufferSize) : MemoryIOS(pBuffer, bufferSize), std::ostream(rdbuf()) {}
		/// Creates a MemoryOutputStream for the given memory area,
		/// ready for writing.
	// MemoryOutputStream(MemoryOutputStream&) : MemoryIOS(other), ostream(rdbuf()) {}

	UInt32		written() { return rdbuf()->written(); }
	void		written(UInt32 size) { rdbuf()->written(size); }
	char*		current() { return rdbuf()->pCurrent(); }
};

//////////


} // namespace Mona
