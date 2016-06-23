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

#include "Mona/Invoker.h"
#include "Mona/Logs.h"

using namespace std;


namespace Mona {


Invoker::Invoker(UInt32 socketBufferSize,UInt16 threads) :clients(), groups(),relayer(poolBuffers,poolThreads,socketBufferSize),sockets(*this,poolBuffers,poolThreads,socketBufferSize), poolThreads(threads), defaultRoom(nullptr) {
	DEBUG(poolThreads.threadsAvailable()," threads available in the server poolthreads");
		
}
Invoker::~Invoker() {
	// delete groups
	for(auto& it : groups)
		delete it.second;
}

} // namespace Mona
