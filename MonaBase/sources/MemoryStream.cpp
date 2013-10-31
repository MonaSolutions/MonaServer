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

#include "Mona/MemoryStream.h"


using namespace std;


namespace Mona {

ScopedMemoryClip::ScopedMemoryClip(MemoryStreamBuf& buffer,UInt32 offset) : _offset(offset),_buffer(buffer) {
	if(_offset>=_buffer._bufferSize)
		_offset = _buffer._bufferSize-1;
	if(_offset<0)
		_offset=0;
	_buffer.clip(_offset);
}
ScopedMemoryClip::~ScopedMemoryClip() {
	_buffer.clip(-(Int32)_offset);
}


MemoryStreamBuf::MemoryStreamBuf(char* pBuffer, UInt32 bufferSize): _pBuffer(pBuffer),_bufferSize(bufferSize),_written(0) {
	setg(_pBuffer, _pBuffer,_pBuffer + _bufferSize);
	setp(_pBuffer, _pBuffer + _bufferSize);
}

MemoryStreamBuf::MemoryStreamBuf(MemoryStreamBuf& other): _pBuffer(other._pBuffer),_bufferSize(other._bufferSize),_written(other._written) {
	setg(_pBuffer,other.gCurrent(),_pBuffer + _bufferSize);
	setp(_pBuffer,_pBuffer + _bufferSize);
	pbump((int)(other.pCurrent()-_pBuffer));
}

void MemoryStreamBuf::clip(Int32 offset) {
	if(offset==0)
		return;
	char* gpos = gCurrent();

	_pBuffer += offset;
	_bufferSize -= offset;
	
	int ppos = pCurrent()-_pBuffer;

	setg(_pBuffer,gpos,_pBuffer + _bufferSize);

	setp(_pBuffer,_pBuffer + _bufferSize);
	pbump(ppos);

	if(_written<offset)
		_written=0;
	else
		_written-=offset;
	if(_written>_bufferSize)
		_written=_bufferSize;
}


void MemoryStreamBuf::next(UInt32 count) {
	if(count==0)
		return;
	UInt32 max = (_pBuffer+_bufferSize)-pCurrent();
	if(count>max)
		count = max;
	pbump(count);
	max = (_pBuffer+_bufferSize)-gCurrent();
	if(count>max)
		count = max;
	gbump(count);
}

void MemoryStreamBuf::position(UInt32 pos) {
	written(); // Save nb char written
	setp(_pBuffer,_pBuffer + _bufferSize);
	if(pos>_bufferSize)
		pos = _bufferSize;
	pbump((int)pos);
	setg(_pBuffer,_pBuffer+pos,_pBuffer + _bufferSize);
}

void MemoryStreamBuf::resize(UInt32 newSize) {
	if(newSize==_bufferSize)
		return;
	_bufferSize = newSize;
	int pos = gCurrent()-_pBuffer;
	if(pos>_bufferSize)
		pos = _bufferSize;
	setg(_pBuffer,_pBuffer+pos,_pBuffer + _bufferSize);
	pos = pCurrent()-_pBuffer;
	if(pos>_bufferSize)
		pos = _bufferSize;
	setp(_pBuffer,_pBuffer + _bufferSize);
	pbump(pos);
}

UInt32 MemoryStreamBuf::written() {
	int written = pCurrent()-begin();
	if(written<0)
		written=0;
	if(written>_written) 
		_written = (UInt32)written;
	return _written;
}

void MemoryIOS::reset(UInt32 newPos) {
	if(newPos>=0)
		rdbuf()->position(newPos);
	clear();
}

UInt32 MemoryIOS::available() {
	int result = rdbuf()->size() - (current()-begin());
	if(result<0)
		return 0;
	return (UInt32)result;
}


} // namespace Mona
