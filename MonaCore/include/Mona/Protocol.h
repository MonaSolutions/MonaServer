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
#include "Mona/Invoker.h"
#include "Mona/Sessions.h"
#include "Mona/Logs.h"

namespace Mona {


class Protocol : public virtual Object, public MapParameters {
public:
	const std::string   name;
	const SocketAddress publicAddress;

	virtual void	manage() {}

protected:
	Protocol(const char* name, Invoker& invoker, Sessions& sessions) : invoker(invoker), sessions(sessions), name(name) {}
	
	bool			auth(const SocketAddress& address);

	Invoker&		invoker;
	Sessions&		sessions;

private:
	bool	load(Exception& ex,const SocketAddress& address) { return true; }
};

inline bool Protocol::auth(const SocketAddress& address) {
	bool auth = !invoker.isBanned(address.host());
	if (!auth)
		INFO("Data rejected because client ", address.host().toString(), " is banned");
	return auth;
}


} // namespace Mona
