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
#include "Mona/Protocol.h"
#include <functional>

namespace Mona {

class UDProtocol;
class Protocols : public virtual Object {
public:
	Protocols(Invoker& invoker) : _invoker(invoker) {}

	void load(Sessions& sessions);
	void unload() { _protocols.clear(); }
	void manage() { for (auto& it : _protocols) it.second->manage(); }

private:
	template<class ProtocolType, typename ...Args >
	void loadProtocol(const char* name, UInt16 port, Sessions& sessions, Args&&... args) {

		// check that name protocol is unique!
		const auto& it(_protocols.lower_bound(name));
		if (it != _protocols.end() && it->first == name) {
			ERROR(name, " protocol already exists, use a unique name for each protocol");
			return;
		}

		std::string buffer;
		if (!_invoker.getNumber(String::Format(buffer, name, ".port"), port))
			_invoker.setNumber(buffer, port);
		if(port==0)
			return; // not fill parameters, so if "publicAddress" parameter doesn't exist it means that protocol is disabled!

		std::unique_ptr<ProtocolType> pProtocol(new ProtocolType(name, _invoker, sessions, args ...));

		std::string host("0.0.0.0");
		_invoker.getString("host", host);
		std::string   publicHost;
	
		// copy configs prefixed to protocol params! (duplicated to get easy access with protocol object)
		Parameters::ForEach forEach;
		forEach = [&publicHost,&pProtocol](const std::string& key, const std::string& value) -> void {
			pProtocol->setString(key, value);
			if (key == "publicHost")
				publicHost = value;
		};
		_invoker.iterate(String::Format(buffer,name,"."),forEach);


		pProtocol->getString("host",host);

		Exception ex;
		
		bool success;
		SocketAddress address;
		EXCEPTION_TO_LOG(success = address.setWithDNS(ex, host, port), name, " server")
		if (!success)
			return;
		ex = NULL;

		// resolve every protocol public server address
		if ((publicHost.empty() && !_invoker.getString("publicHost", publicHost)) || !((SocketAddress&)pProtocol->publicAddress).setWithDNS(ex, publicHost, port)) {
			if (ex) {
				WARN("Impossible to resolve ",publicHost," publicHost, should be written in an IP form");
				ex.set(Exception::NIL);
			}
			((SocketAddress&)pProtocol->publicAddress).set(address);	
		}
		if (!pProtocol->publicAddress.host())
			((SocketAddress&)pProtocol->publicAddress).set(IPAddress::Loopback(),port);
		pProtocol->setString("publicAddress",pProtocol->publicAddress.toString());
	
		// load protocol
		EXCEPTION_TO_LOG(success = pProtocol->load(ex,address), name, " server")
		if (!success)
			return;

		// copy protocol params to invoker params! (to get defaults protocol params + configs params on invoker)
		for (auto& it : *pProtocol)
			_invoker.setString(String::Format(buffer,name,".",it.first), it.second);

		NOTE(name, " server started on ",host,":",port, dynamic_cast<const UDProtocol*>(pProtocol.get()) ? " (UDP)" : " (TCP)");
		_protocols.emplace_hint(it, std::piecewise_construct,std::forward_as_tuple(name),std::forward_as_tuple(pProtocol.release()));
	}


	std::map<std::string,std::unique_ptr<Protocol>>	_protocols;
	Invoker&										_invoker;
};


} // namespace Mona
