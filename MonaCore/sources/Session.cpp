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

#include "Mona/Session.h"
#include "Mona/Decoding.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"
#include "Poco/Format.h"
#include "Poco/NumberFormatter.h"
#include "Poco/RandomStream.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

Session::Session(Protocol& protocol,Invoker& invoker, const Peer& peer,const char* name) :
	protocol(protocol),name(name ? name : ""),invoker(invoker),_pDecodingThread(NULL),died(false),checked(false),failed(false),id(0),peer(peer) {
	(*this->peer.addresses.begin())= peer.address;
	(string&)this->peer.protocol = protocol.name;
	if(memcmp(this->peer.id,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",ID_SIZE)==0)
		RandomInputStream().read((char*)this->peer.id,ID_SIZE);
	DEBUG("peer.id: %s",Util::FormatHex(this->peer.id,ID_SIZE).c_str());
}
	
Session::Session(Protocol& protocol,Invoker& invoker, const char* name) :
	protocol(protocol),name(name ? name : ""),invoker(invoker),_pDecodingThread(NULL),died(false),checked(false),failed(false),id(0),peer((Handler&)invoker) {
	(string&)peer.protocol = protocol.name;
	if(memcmp(peer.id,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",ID_SIZE)==0)
		RandomInputStream().read((char*)peer.id,ID_SIZE);
	DEBUG("peer.id: %s",Util::FormatHex(this->peer.id,ID_SIZE).c_str());
}


Session::~Session() {
	kill();
}

const string& Session::reference() {
	if(name.empty())
		(string&)name = NumberFormatter::format(id);
	return name;
}

void Session::kill() {
	if(died)
		return;
	peer.onDisconnection();
	(bool&)died=true;
}

void Session::decode(Decoding* pDecoding) {
	try {
		_pDecodingThread = invoker.poolThreads.enqueue(pDecoding,_pDecodingThread);
	} catch(Exception& ex) {
		WARN("Decoding message impossible on session %s, %s",reference().c_str(),ex.message().c_str());
	}
}

void Session::receive(MemoryReader& packet) {
	if(!checked) {
		protocol.check(*this);
		(bool&)checked = true;
	}
	if(died)
		return;
	DUMP(packet,format("Request from %s",peer.address.toString()).c_str())
	packetHandler(packet);
}




} // namespace Mona
