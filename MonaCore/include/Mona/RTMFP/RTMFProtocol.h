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
#include "Mona/UDProtocol.h"
#include "Mona/RTMFP/RTMFP.h"
#include "Mona/RTMFP/RTMFPHandshake.h"

namespace Mona {

class RTMFProtocol : public UDProtocol, public virtual Object  {
public:
	RTMFProtocol(const char* name, Invoker& invoker, Sessions& sessions);
	~RTMFProtocol();
	
	bool		load(Exception& ex,const SocketAddress& address);
private:
	void		manage() { if (_pHandshake) _pHandshake->manage(); }
	UDProtocol::OnPacket::Type onPacket;

	std::unique_ptr<RTMFPHandshake>	_pHandshake;
};


} // namespace Mona
