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
#include "Mona/IPAddress.h"
#include <list>


namespace Mona {

	/// This class stores information about a host
	/// such as host name, alias names and a list
	/// of IP addresses.
class HostEntry : virtual Object {
public:
	typedef std::list<std::string> AliasList;
	typedef std::list<IPAddress>   AddressList;
	
	// Creates an empty HostEntry.
	HostEntry();

	// Creates the HostEntry from the data in a hostent structure.
	bool set(Exception& ex, const struct hostent* entry);

#if defined(POCO_HAVE_ADDRINFO)
	// Creates the HostEntry from the data in an addrinfo structure.
	bool set(Exception& ex, struct addrinfo* info);
#endif

#if defined(POCO_VXWORKS)
	bool set(Exception& ex,const std::string& name, const void* addr);
#endif

	// Returns the canonical host name.
	const std::string& name() const {return _name;}
	// Returns a vector containing alias names for the host name
	const AliasList& aliases() const {return _aliases;}
	// Returns a vector containing the IPAddresses for the host
	const AddressList& addresses() const { return _addresses;}

private:
	std::string _name;
	AliasList   _aliases;
	AddressList _addresses;
};


} // namespace Mona
