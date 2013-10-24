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
#include "Poco/StreamUtil.h"
#include <streambuf>
#include <iostream>

namespace Mona {


class MemoryStreamBuf;
class ScopedMemoryClip {
public:
	ScopedMemoryClip(MemoryStreamBuf& buffer,UInt32 offset);
	~ScopedMemoryClip();
private:
	UInt32		_offset;
	MemoryStreamBuf&   _buffer;
};


class MemoryStreamBuf: public std::streambuf {
	friend class ScopedMemoryClip;
public:
	MemoryStreamBuf(char* pBuffer,UInt32 bufferSize);
	MemoryStreamBuf(MemoryStreamBuf&);
	~MemoryStreamBuf();

	
	void			next(UInt32 count);
	UInt32	written();
	void			written(UInt32 size);
	UInt32	size();
	void			resize(UInt32 newSize);
	char*			begin();
	void			position(UInt32 pos=0);
	char*			gCurrent();
	char*			pCurrent();
	
	void			clip(UInt32 offset);

private:
	void			clip(Int32 offset);

	virtual int overflow(int_type c);
	virtual int underflow();
	virtual int sync();

	UInt32	_written;
	char*			_pBuffer;
	UInt32	_bufferSize;

	MemoryStreamBuf();
	MemoryStreamBuf& operator = (const MemoryStreamBuf&);
};

inline void MemoryStreamBuf::written(UInt32 size) {
	_written=size;
}

inline int MemoryStreamBuf::overflow(int_type c) {
	return EOF;
}

inline int MemoryStreamBuf::underflow() {
	return EOF;
}

inline int MemoryStreamBuf::sync() {
	return 0;
}

inline void MemoryStreamBuf::clip(UInt32 offset) {
	clip((Int32)offset);
}

/// inlines

inline UInt32 MemoryStreamBuf::size() {
	return _bufferSize;
}
inline char* MemoryStreamBuf::begin() {
	return _pBuffer;
}
inline char* MemoryStreamBuf::gCurrent() {
	return gptr();
}
inline char* MemoryStreamBuf::pCurrent() {
	return pptr();
}
//////////

class MemoryIOS: public virtual std::ios
	/// The base class for MemoryInputStream and MemoryOutputStream.
	///
	/// This class is needed to ensure the correct initialization
	/// order of the stream buffer and base classes.
{
public:
	MemoryIOS(char* pBuffer,UInt32 bufferSize);
		/// Creates the basic stream.
	MemoryIOS(MemoryIOS&);
	~MemoryIOS();
		/// Destroys the stream.

	MemoryStreamBuf* rdbuf();
		/// Returns a pointer to the underlying streambuf.

	virtual char*	current()=0;
	void			reset(UInt32 newPos);
	void			resize(UInt32 newSize);
	char*			begin();
	void			next(UInt32 count);
	UInt32	available();
	void			clip(UInt32 offset);
		
private:
	MemoryStreamBuf _buf;
};

/// inlines
inline char* MemoryIOS::begin() {
	return rdbuf()->begin();
}
inline void MemoryIOS::clip(UInt32 offset) {
	rdbuf()->clip(offset);
}
inline void MemoryIOS::resize(UInt32 newSize) {
	rdbuf()->resize(newSize);
}
inline void MemoryIOS::next(UInt32 count) {
	rdbuf()->next(count);
}
inline MemoryStreamBuf* MemoryIOS::rdbuf() {
	return &_buf;
}
//////////

class MemoryInputStream: public MemoryIOS, public std::istream
	/// An input stream for reading from a memory area.
{
public:
	MemoryInputStream(const char* pBuffer,UInt32 bufferSize);
		/// Creates a MemoryInputStream for the given memory area,
		/// ready for reading.
	MemoryInputStream(MemoryInputStream&);
	~MemoryInputStream();
		/// Destroys the MemoryInputStream.
	char*			current();
};



inline char* MemoryInputStream::current() {
	return rdbuf()->gCurrent();
}
//////////

class MemoryOutputStream: public MemoryIOS, public std::ostream
	/// An input stream for reading from a memory area.
{
public:
	MemoryOutputStream(char* pBuffer,UInt32 bufferSize);
		/// Creates a MemoryOutputStream for the given memory area,
		/// ready for writing.
	MemoryOutputStream(MemoryOutputStream&);
	~MemoryOutputStream();
		/// Destroys the MemoryInputStream.
	
	UInt32	written();
	void			written(UInt32 size);
	char*			current();
};

/// inlines
inline UInt32 MemoryOutputStream::written() {
	return rdbuf()->written();
}
inline void MemoryOutputStream::written(UInt32 size) {
	rdbuf()->written(size);
}
inline char* MemoryOutputStream::current() {
	return rdbuf()->pCurrent();
}
//////////


} // namespace Mona
