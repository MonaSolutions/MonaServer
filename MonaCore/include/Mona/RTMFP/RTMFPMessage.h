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


class RTMFPMessage : virtual Object {
public:

	RTMFPMessage(bool repeatable) : repeatable(repeatable) {}

	virtual const UInt8*	data()=0;
	virtual UInt32			size()=0;

	std::map<UInt32,UInt64>	fragments;
	const bool				repeatable;

	Time					sendingTime;
};


class RTMFPMessageUnbuffered : public RTMFPMessage, virtual Object {
public:
	RTMFPMessageUnbuffered(const UInt8* data, UInt32 size) : _data(data), _size(size),RTMFPMessage(false) {}
	
private:
	const UInt8*	data() { return _data; }
	UInt32			size() { return _size; }

	UInt32			_size;
	const UInt8*	_data;
};



class RTMFPMessageBuffered: public RTMFPMessage, virtual NullableObject {
public:
	RTMFPMessageBuffered(const PoolBuffers& poolBuffers,bool repeatable) : _pWriter(new AMFWriter(poolBuffers)),RTMFPMessage(repeatable) {}
	RTMFPMessageBuffered() : _pWriter(&AMFWriter::Null),RTMFPMessage(false) {}
	
	virtual ~RTMFPMessageBuffered() { if (_pWriter != &AMFWriter::Null) delete _pWriter; }

	AMFWriter&		writer() { return *_pWriter; }

	operator bool() const { return *_pWriter; }

private:

	const UInt8*	data() { return _pWriter->packet.data(); }
	UInt32			size() { return _pWriter->packet.size(); }

	AMFWriter*		_pWriter;

};


} // namespace Mona
