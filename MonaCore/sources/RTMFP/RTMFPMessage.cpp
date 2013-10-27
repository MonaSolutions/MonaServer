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

#include "Mona/RTMFP/RTMFPMessage.h"
#include <cstring>

using namespace std;
using namespace Poco;


namespace Mona {

RTMFPMessage::RTMFPMessage(istream& istr,bool repeatable) : _reader(istr),repeatable(repeatable),canceled(false) {
	
}


BinaryReader& RTMFPMessage::reader(UInt32& size) {
	map<UInt32,UInt64>::const_iterator it = fragments.begin();
	size =  init(it==fragments.end() ? 0 : it->first);
	return _reader;
}

BinaryReader& RTMFPMessage::reader(UInt32 fragment,UInt32& size) {
	size =  init(fragment);
	return _reader;
}

void RTMFPMessage::addFragment(UInt32 size,UInt64 stage) {
	((map<UInt32,UInt64>&)fragments)[size] = stage;
	_time.update();
}

UInt32 RTMFPMessage::length() {
	UInt32 length;
	reader(length);
	return length;
}

RTMFPMessageBuffered::RTMFPMessageBuffered(bool repeatable) : RTMFPMessage(writer.stream,repeatable) {
}


UInt32 RTMFPMessageBuffered::init(UInt32 position) {
	writer.stream.resetReading(position);
	return writer.stream.size();
}

RTMFPMessageUnbuffered::RTMFPMessageUnbuffered(const UInt8* data,UInt32 size) : _stream((const char*)data,size),RTMFPMessage(_stream,false),_size(size) {
}

UInt32 RTMFPMessageUnbuffered::init(UInt32 position) {
	_stream.reset(position);
	return _stream.available();
}

RTMFPMessageNull::RTMFPMessageNull() {
	writer.stream.setstate(ios_base::eofbit);
}



} // namespace Mona
