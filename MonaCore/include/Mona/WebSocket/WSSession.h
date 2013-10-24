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
#include "Mona/TCPSession.h"
#include "Mona/WebSocket/WSWriter.h"


namespace Mona {


class WSSession : public TCPSession {
public:

	WSSession(Poco::Net::StreamSocket& socket,Protocol& protocol,Invoker& invoker);
	virtual ~WSSession();


	bool			buildPacket(MemoryReader& data,Poco::UInt32& packetSize);
	void			packetHandler(MemoryReader& packet);
	void			manage();

protected:
	WSWriter&		wsWriter();
	void			kill();
	
private:

	WSWriter			_writer;
	Time		_time;
	Publication*		_pPublication;
	Listener*			_pListener;
	Poco::UInt32		_decoded;
};

inline WSWriter& WSSession::wsWriter() {
	return _writer;
}
	


} // namespace Mona
