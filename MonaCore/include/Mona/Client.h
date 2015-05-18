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
#include "Mona/SocketAddress.h"
#include "Mona/Entity.h"
#include "Mona/Writer.h"
#include "Mona/Parameters.h"
#include "Mona/DataReader.h"
#include <vector>

namespace Mona {

namespace Events {
	struct OnCallProperties : Event<bool(DataReader& reader,std::string& value)> {};
};

class Client : public Entity, public virtual Object,
	public Events::OnCallProperties {
public:
	enum FileAccessType {
		READ,
		WRITE
	};

	Client() : _pData(NULL) {}

	const SocketAddress			address;
	const SocketAddress			serverAddress;

	const std::string			protocol;
	virtual const Parameters&	parameters() const =0;

	 // user data (custom data)
	template <typename DataType>
	DataType* setCustomData(DataType* pData) const { return (DataType*)(_pData = pData); }
	bool	  hasCustomData() const { return _pData != NULL; }
	template<typename DataType>
	DataType* getCustomData() const { return (DataType*)_pData; }

	// Alterable in class children Peer
	
	const std::string			path;
	const std::string			query;
	
	const Time					lastReceptionTime;
	
	virtual UInt16				ping() const = 0;
	virtual const Parameters&	properties() const =0;


	virtual Writer&				writer() = 0;


private:
	mutable void*				_pData;
};


} // namespace Mona
