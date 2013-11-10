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

#include "MonaServer.h"
#include "Mona/ServerApplication.h"


using namespace std;
using namespace Mona;


class ServerMona : public ServerApplication  {

private:

///// MAIN
	int main(TerminateSignal& terminateSignal) {
		// starts the server
		int bufferSize(0), threads(0), serversPort(0);
		getNumber("bufferSize", bufferSize);
		getNumber("threads", threads);
		string serversTargets;
		getNumber("servers.port", serversPort);
		getString("servers.targets", serversTargets);
		MonaServer server(terminateSignal, bufferSize, threads, serversPort, serversTargets);
		if (server.start(*this)) {
			terminateSignal.wait();
			// Stop the server
			server.stop();
		}
		return Application::EXIT_OK;
	}

};

int main(int argc, const char* argv[]) {
	return ServerMona().run(argc, argv);
}
