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


#include "Mona/HostEntry.h"

using namespace std;

namespace Mona {

HostEntry::HostEntry() {
}

	
bool HostEntry::set(Exception& ex,const struct hostent* entry) {
	ASSERT_RETURN(entry != NULL,false)
	
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
			_addresses.emplace_back();
			_addresses.back().set(ex, *address, entry->h_length);
			if (ex) {
				_addresses.pop_back();
				return false;
			}
			++address;
		}
	}
	return true;
}


#if defined(POCO_HAVE_ADDRINFO)


bool HostEntry::set(Exception& ex, struct addrinfo* ainfo) {
	ASSERT_RETURN(ainfo != NULL,false)
	for (struct addrinfo* ai = ainfo; ai; ai = ai->ai_next) {
		if (ai->ai_canonname)
			_name.assign(ai->ai_canonname);
		if (ai->ai_addrlen && ai->ai_addr) {
			_addresses.emplace_back();
			if (ai->ai_addr->sa_family == AF_INET6)
				_addresses.back().set(ex, &reinterpret_cast<struct sockaddr_in6*>(ai->ai_addr)->sin6_addr, reinterpret_cast<struct sockaddr_in6*>(ai->ai_addr)->sin6_scope_id);
			else if (ai->ai_addr->sa_family == AF_INET)
				_addresses.back().set(ex, &reinterpret_cast<struct sockaddr_in*>(ai->ai_addr)->sin_addr);
			else
				ex.set(Exception::NETADDRESS, "Unknown address family ", ai->ai_addr->sa_family);
			if (ex) {
				_addresses.pop_back();
				return false;
			}
		}
	}
	return true;
}


#endif // POCO_HAVE_ADDRINFO


#if defined(POCO_VXWORKS)

bool HostEntry::set(Exception& ex,const string& name, const void* addr):_name(name) {
	_addresses.emplace_back();
	_addresses.back().set(ex, &addr);
	if (ex) {
		_addresses.pop_back();
		return false;
	}
	return true;
}

#endif // POCO_VXWORKS



} // namespace Mona
