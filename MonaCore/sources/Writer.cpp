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

#include "Mona/Writer.h"
#include "Mona/Client.h"
#include "Mona/Logs.h"


using namespace std;

namespace Mona {

DataWriterNull      DataWriter::Null;
Writer				Writer::Null(true);

Writer::Writer(WriterHandler* pHandler) : _isNull(false),reliable(true),_state(CONNECTED) {
	if (pHandler)
		_handlers.insert(pHandler);
}

Writer::Writer(Writer& writer) : _isNull(writer._isNull),reliable(writer.reliable),_state(writer._state) {
}

Writer::Writer(bool isNull) : _isNull(isNull), reliable(true), _state(CONNECTED) {
}

Writer::~Writer(){
	close();
}

Writer::State Writer::state(State value, bool minimal) {
	if (value == GET)
		return _state;
	return _state = value;
}

void Writer::close(int code) {
	if(_state==CLOSED)
		return;
	state(CLOSED);
	for (WriterHandler* pHandler : _handlers)
		pHandler->close(*this, code);
	_handlers.clear();
	flush();
}

bool Writer::writeMedia(MediaType type,UInt32 time,PacketReader& packet) {
	ERROR("writeMedia method not supported by this protocol for ",Format<UInt8>("%.2x",(UInt8)type)," type")
	return true;
}

bool Writer::writeMember(const Client& client){
	ERROR("writeMember method not supported by ",client.protocol," protocol")
	return false;
}


} // namespace Mona
