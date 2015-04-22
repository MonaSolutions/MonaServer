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

#include "Mona/Exceptions.h"


namespace Mona {

using namespace std;

map<Exception::Code, const char*> Exception::_CodeMessages ({
	{ Exception::APPLICATION, "Application error" },
	{ Exception::SOFTWARE, "Software error" },
	{ Exception::FILE, "File error" },
	{ Exception::ARGUMENT, "Argument error" },
	{ Exception::OPTION, "Option error" },
	{ Exception::SERVICE, "Service error" },
	{ Exception::REGISTRY, "Registry error" },
	{ Exception::PROTOCOL, "Protocol error" },
	{ Exception::NETWORK, "Network error" },
	{ Exception::SOCKET, "Socket error" },
	{ Exception::NETIP, "Net ip error" },
	{ Exception::NETPORT, "Net port error" },
	{ Exception::FORMATTING, "Formatting error" },
	{ Exception::THREAD, "Thread error" },
	{ Exception::MEMORY, "Memory error" },
	{ Exception::SYSTEM, "System error" },
	{ Exception::MATH, "Math error" },
	{ Exception::CRYPTO, "Crypto error" },
	{ Exception::PERMISSION, "Permission error" },
	{ Exception::ASSERT, "Assert error" }
});



} // namespace Mona
