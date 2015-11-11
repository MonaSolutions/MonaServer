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
#include "Mona/TCPSender.h"
#include "Mona/TCPSession.h"
#include "Mona/AMFWriter.h"
#include "Mona/RTMP/RTMP.h"


namespace Mona {

class RTMPSender : public TCPSender, public virtual Object {
public:
	RTMPSender(const PoolBuffers& poolBuffers) : _writer(poolBuffers),sizePos(0),headerSize(0),TCPSender("RTMPSender") {}

	UInt32				sizePos;
	UInt8				headerSize;

	const UInt8*		data() const { return _writer.packet.data(); }
	UInt32				size() const { return _writer.packet.size(); }


	void				dump(bool encrypted, RTMPChannel& channel, const SocketAddress& address) { pack(channel); Session::DumpResponse(encrypted ? "RTMPE" : "RTMP", data(), size(), address); }

	AMFWriter&			writer(RTMPChannel& channel) { pack(channel); return _writer; }
private:
	void				pack(RTMPChannel& channel);

	AMFWriter			_writer;
};


} // namespace Mona
