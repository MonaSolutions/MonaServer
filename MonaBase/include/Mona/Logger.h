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
#include <mutex>
#include <thread>

namespace Mona {


class Logger : public virtual Object {
public:
	
	enum Level {
		LEVEL_FATAL = 1, /// A fatal error. The application will most likely terminate. This is the highest priority.
		LEVEL_CRITIC,    /// A critical error. The application might not be able to continue running successfully.
		LEVEL_ERROR,    /// An error. An operation did not complete successfully, but the application as a whole is not affected.
		LEVEL_WARN,     /// A warning. An operation completed with an unexpected result.
		LEVEL_NOTE,      /// A notice, which is an information with just a higher priority.
		LEVEL_INFO,		/// An informational message, usually denoting the successful completion of an operation.
		LEVEL_DEBUG,    /// A debugging message.
		LEVEL_TRACE      /// A tracing message. This is the lowest priority.
	};


    virtual void log(THREAD_ID threadId, Level level, const char *filePath, std::string& shortFilePath, long line, std::string& message);
	virtual void dump(const std::string& header, const UInt8* data, UInt32 size);

private:
	static std::mutex	_Mutex;
};

} // namespace Mona
