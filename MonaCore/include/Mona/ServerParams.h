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

struct ProtocolParams {
	ProtocolParams(UInt16 port) : port(port) {}
	UInt16	port;
};

struct HTTPParams : ProtocolParams {
	HTTPParams() : ProtocolParams(80) {}
};

struct RTMPParams : ProtocolParams {
	RTMPParams() : ProtocolParams(1935) {}
};


struct RTMFPParams : ProtocolParams {
	RTMFPParams() : ProtocolParams(1935),keepAlivePeer(10),keepAliveServer(15) {}

	Poco::UInt16				keepAlivePeer;
	Poco::UInt16				keepAliveServer;
};



struct ServerParams {
	ServerParams() : threadPriority(Poco::Thread::PRIO_HIGH) {}
	Poco::Thread::Priority		threadPriority;
	RTMFPParams					RTMFP;
	RTMPParams					RTMP;
	HTTPParams					HTTP;
};


} // namespace Mona
