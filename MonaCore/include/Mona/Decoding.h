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

namespace Mona {


namespace Events {
	struct OnDecoded : Event<void(PacketReader&,const SocketAddress& address)> {};
};

class Decoding : public WorkThread, private Task, public virtual Object,
	public Events::OnDecoded {
public:
	Decoding(const char* name,Invoker& invoker,const UInt8* data,UInt32 size);
	Decoding(const char* name,Invoker& invoker,PoolBuffer& pBuffer);

	SocketAddress	address;

private:
	// If return true, packet is pass to the session.
	// If ex is raised on true returned value, it displays a WARN
	// If ex is raised on false returned value, it displays a ERROR
	virtual const UInt8*	decodeRaw(Exception& ex, PoolBuffer& pBuffer, UInt32 times,const UInt8* data,UInt32& size);
	virtual bool			decode(Exception& ex, PacketReader& packet, UInt32 times) { return false; }

	bool			run(Exception& ex);
	void			handle(Exception& ex);

	PoolBuffer						_pBuffer;
	UInt32							_size;
	const UInt8*					_current;
};


} // namespace Mona
