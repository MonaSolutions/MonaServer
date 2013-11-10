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
#include "Mona/Entity.h"
#include "Mona/Writer.h"
#include "Mona/Entities.h"
#include "Mona/MapParameters.h"

namespace Mona {

class Client : public Entity, public MapParameters, virtual Object {
public:
	Client(Entities<Client>::Map& turnClients):turnClients(turnClients),_pWriter(NULL),ping(0){}
	Client(const Client& o) :
		Entity(o),address(o.address), serverAddress(o.serverAddress),
		path(o.path), ping(o.ping), turnClients(o.turnClients), _pWriter(o._pWriter) {
	}

	const SocketAddress			address;
	const SocketAddress			serverAddress;

	const std::string			path;
	const UInt16				ping;
	Entities<Client>			turnClients;

	virtual bool				setName(const std::string& name)=0;

	Writer&						writer() { return _pWriter ? *_pWriter : Writer::Null; }
protected:
	Writer*						_pWriter;
};


} // namespace Mona
