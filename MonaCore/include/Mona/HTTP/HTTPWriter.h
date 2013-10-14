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
#include "Mona/Writer.h"
#include "Mona/HTTPPacketWriter.h"
#include "Mona/SocketHandler.h"
#include "Mona/HTTP/HTTPSender.h"
#include "Poco/Net/StreamSocket.h"

namespace Mona {


class HTTPWriter : public Writer {
public:

	HTTPWriter(SocketHandler<Poco::Net::StreamSocket>& handler);
	virtual ~HTTPWriter();

	State			state(State value=GET,bool minimal=false);
	void			flush(bool full=false);

	DataWriter&		writeInvocation(const std::string& name);
	DataWriter&		writeMessage();
	void			writeRaw(const Mona::UInt8* data,Mona::UInt32 size);

	// TODO ?void			close(int code);


	
private:
	bool				hasToConvert(DataReader& reader);
	HTTPPacketWriter&	newWriter();


	SocketHandler<Poco::Net::StreamSocket>&		_handler;
	std::list<Poco::AutoPtr<HTTPSender>>		_senders;
};

inline void HTTPWriter::writeRaw(const Mona::UInt8* data,Mona::UInt32 size) {
	newWriter().writer.writeRaw(data,size);
}

inline bool HTTPWriter::hasToConvert(DataReader& reader) {
	return dynamic_cast<HTTPPacketWriter*>(&reader)==NULL;
}


} // namespace Mona
