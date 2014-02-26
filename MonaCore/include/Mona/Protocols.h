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
#include "Mona/ServerParams.h"
#include "Mona/Protocol.h"


namespace Mona {

class UDProtocol;
class Protocols : virtual Object {
public:
	Protocols(Invoker& invoker) : _invoker(invoker) {}

	void load(Sessions& sessions);
	void unload() { _protocols.clear(); }
	void manage() { for (std::unique_ptr<Protocol>& pProtocol : _protocols) pProtocol->manage(); }

private:
	template<class ProtocolType, class ParamsType,typename ...Args >
	void loadProtocol(const char* name, const ParamsType& params, Sessions& sessions, Args&&... args) {
		if(params.port==0)
			return;
		std::unique_ptr<Protocol> pProtocol(new ProtocolType(name, _invoker, sessions, args ...));
		Exception ex;
		bool success = false;
		EXCEPTION_TO_LOG(success = ((ProtocolType*)pProtocol.get())->load(ex, params), name, " server")
		if (!success)
			return;
		NOTE(name, " server starts on ", params.port, " ", dynamic_cast<UDProtocol*>(pProtocol.get()) ? "UDP" : "TCP", " port");
		_protocols.emplace_back(pProtocol.release());
	}

	std::vector<std::unique_ptr<Protocol>>	_protocols;
	Invoker&								_invoker;
};


} // namespace Mona
