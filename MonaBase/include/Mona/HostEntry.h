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
#include "Mona/IPAddress.h"
#include <set>


namespace Mona {

	/// This class stores information about a host
	/// such as host name, alias names and a list
	/// of IP addresses.
class HostEntry : public virtual Object {
public:
	typedef std::vector<std::string> AliasList;
	typedef std::set<IPAddress>   AddressList;
	
	// Creates an empty HostEntry.
	HostEntry() {}

	// Creates the HostEntry from the data in a hostent structure.
	void set(Exception& ex, const struct hostent* entry);

	// Creates the HostEntry from the data in an addrinfo structure.
	void set(Exception& ex, struct addrinfo* info);


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
