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
#include "Mona/BinaryWriter.h"
#include "Mona/AMFWriter.h"
#include "Mona/BinaryStream.h"
#include "Mona/BinaryReader.h"
#include "Mona/MemoryStream.h"
#include "Poco/Buffer.h"
#include <list>

namespace Mona {


class RTMFPMessage {
public:

	RTMFPMessage(std::istream& istr,bool repeatable);
	virtual ~RTMFPMessage();

	BinaryReader&			reader(UInt32& size);
	BinaryReader&			reader(UInt32 fragment,UInt32& size);

	virtual UInt32	length();
	UInt32			elapsed();

	void					addFragment(UInt32 size,UInt64 stage);

	std::map<UInt32,UInt64>		fragments;
	const bool								repeatable;
	bool									canceled;
	
private:
	virtual	UInt32	init(UInt32 position)=0;
	BinaryReader			_reader;
	Time			_time;
};

inline UInt32 RTMFPMessage::elapsed() {
	return (UInt32)(_time.elapsed()/1000);
}


class RTMFPMessageUnbuffered : public RTMFPMessage {
public:
	RTMFPMessageUnbuffered(const UInt8* data,UInt32 size);
	virtual ~RTMFPMessageUnbuffered();

private:
	UInt32				init(UInt32 position);
	UInt32				length();

	MemoryInputStream			_stream;
	UInt32				_size;
};

inline UInt32 RTMFPMessageUnbuffered::length() {
	return _size;
}


class RTMFPMessageBuffered : public RTMFPMessage {
public:
	RTMFPMessageBuffered(bool repeatable=true);
	virtual ~RTMFPMessageBuffered();

	AMFWriter			writer;

private:
	UInt32		init(UInt32 position);

};

class RTMFPMessageNull : public RTMFPMessageBuffered {
public:
	RTMFPMessageNull();
};


} // namespace Mona
