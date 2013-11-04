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


#include "Mona/DNS.h"
#include "Mona/SocketAddress.h"
#include "Mona/Socket.h"
#include <cstring>

using namespace std;


namespace Mona {


bool DNS::HostByName(Exception& ex, const string& hostname, HostEntry& host) {
	if (!Net::InitializeNetwork(ex))
		return false;

	struct addrinfo* pAI;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
	int rc = getaddrinfo(hostname.c_str(), NULL, &hints, &pAI); 
	if (rc == 0) {
		host.set(ex,pAI);
		freeaddrinfo(pAI);
		return true;
	}
	SetAIError(ex,rc, hostname);
	return false;
}


bool DNS::HostByAddress(Exception& ex,const IPAddress& address, HostEntry& host) {
	if (!Net::InitializeNetwork(ex))
		return false;
	SocketAddress sa;
	sa.set(address, 0);
	static char fqname[1024];
	int rc = getnameinfo(sa.addr(), sizeof(*sa.addr()), fqname, sizeof(fqname), NULL, 0, NI_NAMEREQD);
	if (rc == 0) {
		struct addrinfo* pAI;
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
		rc = getaddrinfo(fqname, NULL, &hints, &pAI);
		if (rc == 0) {
			host.set(ex, pAI);
			freeaddrinfo(pAI);
			return true;
		}
	}
	SetAIError(ex, rc, address.toString());
	return false;
}


bool DNS::Resolve(Exception& ex, const string& address, HostEntry& host) {
	IPAddress ip;
	Exception ignore;
	if (ip.set(ignore, address))
		return HostByAddress(ex,ip,host);
	return HostByName(ex, address, host);
}


bool DNS::ThisHost(Exception& ex, HostEntry& host) {
	string name;
	if (!HostName(ex, name))
		return false;
	return HostByName(ex,name, host);
}


bool DNS::HostName(Exception& ex,string& host) {
	if(!Net::InitializeNetwork(ex))
		return false;
	char buffer[256];
	int rc = gethostname(buffer, sizeof(buffer));
	if (rc == 0) {
		host.assign(buffer);
		return true;
	}
	ex.set(Exception::NETADDRESS,"Cannot get host name");
	return false;
}


void DNS::SetAIError(Exception& ex, int error, const string& argument) {
	string message(gai_strerror(error)); // TODO tester!!!
	if (!argument.empty()) {
		message.append(" (");
		message.append(argument);
		message.append(")");
	}
	ex.set(Exception::NETADDRESS, message);
}

} // namespace Mona
