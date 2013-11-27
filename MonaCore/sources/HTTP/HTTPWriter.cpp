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

#include "Mona/HTTP/HTTPWriter.h"
#include "Mona/HTTP/HTTP.h"
#include "Mona/String.h"

using namespace std;

namespace Mona {

HTTPWriter::HTTPWriter(StreamSocket& socket) : _socket(socket) {
	
}


HTTPPacketWriter& HTTPWriter::newWriter() {
	HTTPSender* pSender = new HTTPSender();
	_senders.emplace_back(pSender);
	return pSender->writer;
}

void HTTPWriter::flush(bool full) {
	if(_senders.empty())
		return;
	// TODO _qos.add(ping,_sent);
	FLUSH_SENDERS("HTTPSender flush", HTTPSender, _senders)
}


HTTPWriter::State HTTPWriter::state(State value,bool minimal) {
	State state = Writer::state(value,minimal);
	if(state==CONNECTED && minimal)
		_senders.clear();
	return state;
}



DataWriter& HTTPWriter::writeInvocation(const std::string& name) {
	DataWriter& writer = writeMessage();
	string header("HTTP/1.1 ");
	Exception ex;
	UInt16 code(0);
	if (String::ToNumber<UInt16>(name, code)) {
		string message;
		header.append(name);
		header.append(" ");
		HTTP::CodeToMessage(code,message);
		if(!message.empty())
			header.append(message);
	} else
		header.append(name);
	writer.writeString(header);
	return writer;
}

DataWriter& HTTPWriter::writeMessage() {
	if(state()==CLOSED)
        return DataWriter::Null;
	return newWriter();
}



} // namespace Mona
