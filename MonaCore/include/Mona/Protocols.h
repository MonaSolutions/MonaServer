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
#include "Mona/ServerParams.h"
#include "Mona/Protocol.h"
#include <list>


namespace Mona {

class Protocols : virtual Object {
public:
	Protocols(Invoker& invoker) : _invoker(invoker) {}

	void load(Gateway& gateway);
	void unload() { _protocols.clear(); }
	void manage() { for (std::shared_ptr<Protocol>& pProtocol : _protocols) pProtocol->manage(); }

private:
	template<class ProtocolType, class ParamsType>
	void loadProtocol(const char* name,const ParamsType& params,Gateway& gateway) {
		if(params.port==0)
			return;
		std::shared_ptr<Protocol> pProtocol(new ProtocolType(name, _invoker,gateway));
		Exception ex;
		bool success = false;
		EXCEPTION_TO_LOG(success=pProtocol->load(ex, (ParamsType&)params), name, " server")
		if (success) {
			_protocols.emplace_back(pProtocol);
			NOTE(name, " server starts on ", params.port, " ", dynamic_cast<UDProtocol*>(pProtocol.get()) ? "UDP" : "TCP", " port");
		}
	}

	std::list<std::shared_ptr<Protocol>>	_protocols;
	Invoker&								_invoker;
};


} // namespace Mona
