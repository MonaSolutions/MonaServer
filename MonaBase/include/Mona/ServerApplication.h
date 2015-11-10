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
#include "Mona/Application.h"
#include "Mona/TerminateSignal.h"

namespace Mona {

class ServerApplication : public Application, public virtual Object {
public:
	ServerApplication() : _isInteractive(true) { _PThis = this; }

	bool	isInteractive() const { return _isInteractive; }

    int		run(int argc, const char* argv[]);

protected:
	void defineOptions(Exception& ex, Options& options);

private:
    virtual int main(TerminateSignal& terminateSignal) = 0;
	int			main() { return Application::EXIT_OK; }

	bool		_isInteractive;

    static ServerApplication*	 _PThis;
	static TerminateSignal		 _TerminateSignal;

#if defined(_WIN32)
	static int  __stdcall ConsoleCtrlHandler(unsigned long ctrlType);
	static void __stdcall ServiceMain(unsigned long argc, char** argv);
	static void __stdcall ServiceControlHandler(unsigned long control);

	bool hasConsole();
	bool isService();
	void beService();
	bool registerService(Exception& ex);
	bool unregisterService(Exception& ex);

	std::string _displayName;
	std::string _description;
	std::string _startup;

#else
	void handlePidFile(Exception& ex,const std::string& value);
    bool isDaemon(int argc, const char** argv);
	void beDaemon();

	std::string _pidFile;
#endif
};


} // namespace Mona
