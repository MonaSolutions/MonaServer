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
#include "Mona/Writer.h"
#include "Mona/HTTPPacketWriter.h"
#include "Mona/HTTP/HTTPSender.h"
#include "Mona/StreamSocket.h"

namespace Mona {


class HTTPWriter : public Writer, virtual Object {
public:

	HTTPWriter(StreamSocket& socket);

	State			state(State value=GET,bool minimal=false);
	void			flush(bool full=false);

	DataWriter&		writeInvocation(const std::string& name);
	DataWriter&		writeMessage();
	void			writeRaw(const UInt8* data, UInt32 size) { newWriter().writer.writeRaw(data, size); }

	// TODO ?void			close(int code);


	
private:
	bool				hasToConvert(DataReader& reader) { return dynamic_cast<HTTPPacketWriter*>(&reader) == NULL; }
	HTTPPacketWriter&	newWriter();


	StreamSocket&								_socket;
	std::list<std::shared_ptr<HTTPSender>>		_senders;
};



} // namespace Mona
