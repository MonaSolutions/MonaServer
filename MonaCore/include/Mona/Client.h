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

namespace Mona {

namespace Events {
	struct OnCallProperties : Event<bool(std::vector<std::string>&)> {};
};

class Client : public Entity, public virtual Object,
	public Events::OnCallProperties {
public:
	Client() : _pWriter(NULL),ping(0),_pUserData(NULL) {}

	const SocketAddress			address;
	const std::string			protocol;
	virtual const Parameters&	parameters() const =0;
	const std::string			name;

	template <typename DataType>
	DataType*					getUserData() const { return (DataType*)_pUserData; }

	template <typename DataType>
	DataType&					setUserData(DataType& data) { _pUserData = &data; return data; }

	// Alterable in class children Peer
	
	const std::string			path;
	const std::string			query;
	const std::string			serverAddress;
	const UInt16				ping;
	virtual const Parameters&	properties() const =0;
	void						properties(std::vector<std::string>& items) {
		if (!OnCallProperties::subscribed() || OnCallProperties::raise<false>(items)) {
			for (std::string& item : items)
				if (!properties().getString(item, item)) item.assign("null");
		}
	}

	Writer&						writer() { return _pWriter ? *_pWriter : Writer::Null; }
protected:
	Writer*						_pWriter;
private:
	void*						_pUserData;
};


} // namespace Mona
