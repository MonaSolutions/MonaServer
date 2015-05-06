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
#include "Mona/AMFWriter.h"


namespace Mona {


class RTMFPMessage : public virtual Object {
public:

	RTMFPMessage(bool repeatable) : repeatable(repeatable),_frontSize(0) {}
	RTMFPMessage(AMF::ContentType type, UInt32 time, bool repeatable) :   _frontSize(type==AMF::EMPTY ? 0 : (type==AMF::DATA_AMF3 ? 6 : 5)), repeatable(repeatable) {
		if (type == AMF::EMPTY)
			return;
		_front[0] = type;
		BinaryWriter(&_front[1], 4).write32(time);
		if (type == AMF::DATA_AMF3)
			_front[5] = 0;
	}

	const UInt8*	front()  const { return _front;  }
	UInt8			frontSize() const { return _frontSize; }

	virtual const UInt8*	body()  const = 0;
	virtual UInt32			bodySize() const = 0;

	UInt32					size() const { return frontSize()+bodySize(); }

	std::map<UInt32,UInt64>	fragments;
	const bool				repeatable;
private:
	UInt8					_front[6];
	UInt8					_frontSize;
};


class RTMFPMessageUnbuffered : public RTMFPMessage, public virtual Object {
public:
	RTMFPMessageUnbuffered(const UInt8* data, UInt32 size) : _data(data), _size(size),RTMFPMessage(false) {}
	RTMFPMessageUnbuffered(AMF::ContentType type, UInt32 time,const UInt8* data, UInt32 size) : _data(data), _size(size),RTMFPMessage(type,time,false) {}

private:
	const UInt8*	body() const { return _data; }
	UInt32			bodySize() const { return _size; }

	UInt32			_size;
	const UInt8*	_data;
};



class RTMFPMessageBuffered: public RTMFPMessage, virtual public NullableObject {
public:
	RTMFPMessageBuffered(const PoolBuffers& poolBuffers,bool repeatable) : _pWriter(new AMFWriter(poolBuffers)),RTMFPMessage(repeatable) {}
	RTMFPMessageBuffered() : _pWriter(&AMFWriter::Null),RTMFPMessage(false) {}
	
	virtual ~RTMFPMessageBuffered() { if (*_pWriter) delete _pWriter; }

	AMFWriter&		writer() { return *_pWriter; }

	operator bool() const { return *_pWriter; }

private:

	const UInt8*	body() const { return _pWriter->packet.data(); }
	UInt32			bodySize() const { return _pWriter->packet.size(); }

	AMFWriter*		_pWriter;

};


} // namespace Mona
