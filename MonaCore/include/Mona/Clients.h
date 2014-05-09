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
#include "Mona/Entities.h"
#include "Mona/Client.h"

namespace Mona {

class Clients : public Entities<Client>, public virtual Object {
public:

	Client* operator()(const UInt8* id) const { return Entities<Client>::operator()(id); }
	Client* operator()(const std::string& name) const {
		std::map<std::string,Client*>::const_iterator it = _clientsByName.find(name);
		if(it==_clientsByName.end())
			return NULL;
		return it->second;
	}
	bool add(Exception& ex,Client& client) {
		if (client.properties().getString("name", (std::string&)client.name) && !client.name.empty()) {
			if (!_clientsByName.emplace(client.name, &client).second) {
				ex.set(Exception::PROTOCOL, "Client named ",client.name," exists already");
				return false;
			}
		}
		if (Entities<Client>::add(client))
			return true;
		std::string hex;
		ex.set(Exception::PROTOCOL, "Client ",Util::FormatHex(client.id, ID_SIZE, hex)," exists already");
		return false;
	}

	bool remove(Client& client) {
		if (!client.name.empty())
			_clientsByName.erase(client.name);
		return Entities<Client>::remove(client);
	}
	

private:
	std::map<std::string,Client*>	_clientsByName;
};



} // namespace Mona
