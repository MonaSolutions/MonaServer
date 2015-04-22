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
#include "Mona/HostEntry.h"


namespace Mona {

	/// This class provides an interface to the
	/// domain name service.
	///
	/// An internal DNS cache is used to speed up name lookups.

class DNS : virtual Static {
public:

	// Returns a HostEntry object containing the DNS information for the host with the given name
	static bool HostByName(Exception& ex, const std::string& hostname, HostEntry& host) { return HostByName(ex, hostname.data(), host); }
	static bool HostByName(Exception& ex, const char* hostname, HostEntry& host);
		
	// Returns a HostEntry object containing the DNS information for the host with the given IP address
	// BEWARE blocking method!!
	static bool HostByAddress(Exception& ex, const IPAddress& address, HostEntry& host);

	// Returns a HostEntry object containing the DNS information for the host with the given IP address or host name
	// BEWARE blocking method!!
	static bool Resolve(Exception& ex, const std::string& address, HostEntry& host)  { return Resolve(ex, address.data(), host); }
	static bool Resolve(Exception& ex, const char* address, HostEntry& host);
		
	// Returns a HostEntry object containing the DNS information for this host
	// BEWARE blocking method!!
	static bool ThisHost(Exception& ex,HostEntry& host);

	// Returns the host name of this host
	static bool HostName(Exception& ex, std::string& host);

private:

	// Set the exception according to the getaddrinfo() error code
	template <typename ...Args>
	static void SetAIError(Exception& ex, int error, Args&&... args) {
		ex.set(Exception::NETIP, gai_strerror(error), args ...);
	}

};


} // namespace Mona
