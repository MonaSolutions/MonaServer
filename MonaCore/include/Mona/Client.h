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
#include "Mona/Entity.h"
#include "Mona/Writer.h"
#include "Mona/Entities.h"
#include "Mona/MapParameters.h"

namespace Mona {

class Client : public Entity, virtual Object {
public:
	Client() : _pWriter(NULL),ping(0),timesBeforeTurn(0),_pUserData(NULL) {}

	const SocketAddress			address;
	const std::string			protocol;

	const std::string			name;

	template <typename DataType>
	DataType*					getUserData() const { return (DataType*)_pUserData; }

	template <typename DataType>
	DataType&					setUserData(DataType& data) { _pUserData = &data; return data; }

	// Alterable in class children Peer
	
	const std::string			path;
	const std::string			query;
	const MapParameters			properties;
	const std::string			serverAddress;
	const UInt16				ping;


	UInt32						timesBeforeTurn;
	const Entities<Client>		turnClients;


	Writer&						writer() { return _pWriter ? *_pWriter : Writer::Null; }
protected:
	Writer*						_pWriter;
private:
	void*						_pUserData;
};


} // namespace Mona
