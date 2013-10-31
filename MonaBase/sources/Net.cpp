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

#include "Mona/Net.h"

using namespace std;

namespace Mona {

#if defined(_WIN32)
static volatile bool	Initialized(false);
static mutex			Mutex;

bool Net::InitializeNetwork(Exception& ex) {
	lock_guard<mutex> lock(Mutex);
	if (Initialized)
		return true;
	WORD    version = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(version, &data) != 0) {
		ex.set(Exception::NETWORK, "Failed to initialize network subsystem");
		return false;
	}
	return Initialized = true;
}

class NetUninitializer {
public:
	~NetUninitializer() {
		if (!Initialized)
			return;
		WSACleanup();
	}
};


NetUninitializer uninitializer;
#endif 

} // namespace Mona
