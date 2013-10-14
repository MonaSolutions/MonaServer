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

#include "Mona/Protocol.h"
#include "Mona/Invoker.h"

using namespace Poco::Net;

namespace Mona {

Protocol::Protocol(const char* name,Invoker& invoker,Gateway& gateway):invoker(invoker),gateway(gateway),name(name) {
}

Protocol::~Protocol(){
}


bool Protocol::auth(const SocketAddress& address) {
	bool auth = !invoker.isBanned(address.host());
	if(!auth)
		INFO("Data rejected because client %s is banned",address.host().toString().c_str());
	return auth;
}


} // namespace Mona
