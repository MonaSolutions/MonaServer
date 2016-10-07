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


#include "Mona/HostEntry.h"

using namespace std;

namespace Mona {

	
void HostEntry::set(Exception& ex,const struct hostent* entry) {
	ASSERT(entry != NULL)

	_name = entry->h_name;
	char** alias = entry->h_aliases;
	if (alias) {
		while (*alias) {
			_aliases.emplace_back(*alias);
			++alias;
		}
	}

	char** address = entry->h_addr_list;
	if (address) {
		while (*address) {
			if (entry->h_length == sizeof(struct in_addr))
				_addresses.emplace(*reinterpret_cast<struct in_addr*>(*address));
			else if (entry->h_length == sizeof(struct in6_addr))
				_addresses.emplace(*reinterpret_cast<struct in6_addr*>(*address));
			else
				ex.set(Exception::NETIP, "Unvalid host ip entry");
			++address;
		}
	}
}


void HostEntry::set(Exception& ex, struct addrinfo* ainfo) {
	ASSERT(ainfo != NULL)
	for (struct addrinfo* ai = ainfo; ai; ai = ai->ai_next) {
		if (ai->ai_canonname)
			_name.assign(ai->ai_canonname);
		if (ai->ai_addrlen && ai->ai_addr) {
			if (ai->ai_addr->sa_family == AF_INET6)
				_addresses.emplace(reinterpret_cast<struct sockaddr_in6*>(ai->ai_addr)->sin6_addr, reinterpret_cast<struct sockaddr_in6*>(ai->ai_addr)->sin6_scope_id);
			else if (ai->ai_addr->sa_family == AF_INET)
				_addresses.emplace(reinterpret_cast<struct sockaddr_in*>(ai->ai_addr)->sin_addr);
			else
				ex.set(Exception::NETIP, "Unknown ip family ", ai->ai_addr->sa_family);
		}
	}
}



} // namespace Mona
