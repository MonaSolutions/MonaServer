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
#include "ApplicationKiller.h"
#include "Mona/ServerApplication.h"


#if defined(POCO_OS_FAMILY_UNIX)
#include <signal.h>
#endif

using namespace Mona;
using namespace Poco;
using namespace std;



class ServerMona : public ServerApplication, private Logger, private ApplicationKiller  {
public:
	ServerMona() {
	}

private:

	void kill() {
		terminate();
	}



///// MAIN
	int main() {

#if defined(POCO_OS_FAMILY_UNIX) // TODO remonter d'un niveau?
		sigset_t sset;
		sigemptyset(&sset);
		if (!getenv("POCO_ENABLE_DEBUGGER"))
			sigaddset(&sset, SIGINT);
		sigaddset(&sset, SIGQUIT);
		sigaddset(&sset, SIGTERM);
		sigprocmask(SIG_BLOCK, &sset, NULL);
#endif

		// starts the server
		int bufferSize(0), threads(0), serversPort(0);
		getNumber("bufferSize", bufferSize);
		getNumber("threads", threads);
		string serversTargets;
		getNumber("servers.port", serversPort);
		getString("servers.targets", serversTargets);
		MonaServer server(*this, bufferSize, threads, serversPort, serversTargets);
		server.start(*this);

		// wait for CTRL-C or kill
#if defined(POCO_OS_FAMILY_UNIX)
		int sig;
		sigwait(&sset, &sig);		
#else
		waitForTerminationRequest();
#endif

		// Stop the server
		server.stop();
		return Application::EXIT_OK;
	}

};

int main(int argc, char* argv[]) {
	return ServerMona().run(argc, argv);
}
