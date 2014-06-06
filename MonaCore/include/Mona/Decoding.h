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
#include "Mona/Invoker.h"
#include "Mona/PoolBuffer.h"
#include "Mona/Expirable.h"

namespace Mona {


class Session;
class Decoding : public WorkThread, private Task, public virtual Object {
public:

	Decoding(Invoker& invoker, const Session& session, const SocketAddress& address, const char* name,const UInt8* data,UInt32 size);
	Decoding(Invoker& invoker, const Session& session, const SocketAddress& address, const char* name, PoolBuffer& pBuffer);
	Decoding(Invoker& invoker, const Session& session, const char* name,const UInt8* data,UInt32 size);
	Decoding(Invoker& invoker, const Session& session, const char* name,PoolBuffer& pBuffer);


private:
	// If return true, packet is pass to the session.
	// If ex is raised on true returned value, it displays a WARN
	// If ex is raised on false returned value, it displays a ERROR
	virtual const UInt8*	decodeRaw(Exception& ex, PoolBuffer& pBuffer, UInt32 times,const UInt8* data,UInt32& size);
	virtual bool			decode(Exception& ex, PacketReader& packet, UInt32 times) { return false; }

	bool			run(Exception& ex);
	void			handle(Exception& ex);

	SocketAddress					_address;
	PoolBuffer						_pBuffer;
	Expirable<Session>				_expirableSession;
	UInt32							_size;
	const UInt8*					_current;
};


} // namespace Mona
