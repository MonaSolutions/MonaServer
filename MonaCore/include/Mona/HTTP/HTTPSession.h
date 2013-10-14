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
#include "Mona/WebSocket/WSSession.h"
#include "Mona/HTTP/HTTPWriter.h"


namespace Mona {


class HTTPSession :  public WSSession {
public:

	HTTPSession(Poco::Net::StreamSocket& socket,Protocol& protocol,Invoker& invoker);
	virtual ~HTTPSession();
	
private:
	void			manage();

	bool			buildPacket(MemoryReader& data,Poco::UInt32& packetSize);
	void			packetHandler(MemoryReader& packet);

	void			processGet(const std::string& fileName);

	HTTPWriter		_writer;
	bool			_isWS;
};


} // namespace Mona
