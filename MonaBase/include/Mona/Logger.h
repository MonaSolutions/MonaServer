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
#include "Poco/Thread.h"

namespace Mona {


class Logger
{
public:
	enum Priority
	{
		PRIO_FATAL = 1,   /// A fatal error. The application will most likely terminate. This is the highest priority.
		PRIO_CRITIC,    /// A critical error. The application might not be able to continue running successfully.
		PRIO_ERROR,       /// An error. An operation did not complete successfully, but the application as a whole is not affected.
		PRIO_WARN,     /// A warning. An operation completed with an unexpected result.
		PRIO_NOTE,      /// A notice, which is an information with just a higher priority.
		PRIO_INFO, /// An informational message, usually denoting the successful completion of an operation.
		PRIO_DEBUG,       /// A debugging message.
		PRIO_TRACE        /// A tracing message. This is the lowest priority.
	};

	Logger() {}
	virtual ~Logger() {}

	virtual void logHandler(Poco::Thread::TID threadId,const std::string& threadName,Priority priority,const char *filePath,long line, const char *text)=0;
	virtual void dumpHandler(const Mona::UInt8* data,Mona::UInt32 size){}

private:
	
};

} // namespace Mona
