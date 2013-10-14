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

class Client : public Entity, public MapParameters {
public:
	Client(Entities<Client>::Map& turnClients):turnClients(turnClients),_pWriter(NULL),ping(0){}
	virtual ~Client(){}

	const Poco::Net::SocketAddress				address;
	const Poco::Net::SocketAddress				serverAddress;

	const std::string							path;
	const Mona::UInt16							ping;
	const std::string							protocol;
	Entities<Client>							turnClients;

	virtual bool		setName(const std::string& name)=0;

	Writer&										writer();			
protected:
	Writer*										_pWriter;
};

inline Writer&	Client::writer() {
	return _pWriter ? *_pWriter : Writer::Null;
}


} // namespace Mona
