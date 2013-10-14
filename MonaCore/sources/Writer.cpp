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

#include "Mona/Writer.h"
#include "Mona/Logs.h"

using namespace Poco;

namespace Mona {

DataWriterNull	Writer::DataWriterNull;
Writer			Writer::Null;

Writer::Writer(WriterHandler* pHandler) : reliable(true),_pHandler(pHandler),_state(CONNECTED) {
}

Writer::Writer(Writer& writer) : reliable(writer.reliable),_pHandler(writer._pHandler),_state(writer._state) {
}

Writer::~Writer(){
	close();
}

void Writer::close(int code) {
	if(_state==CLOSED)
		return;
	state(CLOSED);
	if(_pHandler)
		_pHandler->close(*this,code);
	flush();
}

bool Writer::writeMedia(MediaType type,UInt32 time,MemoryReader& data) {
	ERROR("writeMedia method not supported by this protocol for %.2x type",type)
	return true;
}

void Writer::writeMember(const Peer& peer){
	ERROR("writeMember method not supported by this protocol")
}


} // namespace Mona
