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


Invoker::Invoker(UInt32 socketBufferSize,UInt16 threads) : poolThreads(threads),relayer(poolBuffers,poolThreads,socketBufferSize),sockets(*this,poolBuffers,poolThreads,socketBufferSize),publications(_publications), clients(), groups() {
	DEBUG(poolThreads.threadsAvailable()," threads available in the server poolthreads");
		
}


Invoker::~Invoker() {
	// delete groups
	for(auto& it : groups)
		delete it.second;
}


Publication* Invoker::publish(Exception& ex, const string& name, Publication::Type type, Peer* pPeer) {
	MAP_FIND_OR_EMPLACE(_publications, it, name, name,poolBuffers);
	Publication& publication(it->second);

	if(publication.running()) { // is already publishing
		ex.set(Exception::SOFTWARE, name, " is already publishing");
		return NULL;
	}

	if(pPeer ? pPeer->onPublish(ex,publication) : onPublish(ex,publication)) {
		publication.start(type);
		return &publication;
	}

	if(!ex)
		ex.set(Exception::APPLICATION, "Not allowed to publish ",name);
	if (publication.listeners.count() == 0)
		_publications.erase(it);
	return NULL;
}


void Invoker::unpublish(const string& name,Peer* pPeer) {

	auto it = _publications.find(name);
	if(it == _publications.end()) {
		DEBUG("The publication '",name,"' doesn't exist, unpublish useless");
		return;
	}
	Publication& publication(it->second);
	if (publication.running()) {
		if (pPeer)
			pPeer->onUnpublish(publication);
		else
			onUnpublish(publication);
		publication.stop();
	}
	
	if(publication.listeners.count()==0)
		_publications.erase(it);
}

Listener* Invoker::subscribe(Exception& ex, Peer& peer,string& name,Writer& writer) {
	string query;
	publicationName(name, query);
	Listener* pListener(subscribe(ex, peer, name, writer, query.c_str()));
	if (pListener)
		Util::UnpackQuery(query, *pListener);
	return pListener;
}

Listener* Invoker::subscribe(Exception& ex, Peer& peer,const string& name,Writer& writer, const char* queryParams) {
	MAP_FIND_OR_EMPLACE(_publications, it, name, name,poolBuffers);
	Publication& publication(it->second);
	Listener* pListener = publication.addListener(ex, peer,writer, queryParams);
	if (!pListener) {
		if(!publication.running() && publication.listeners.count()==0)
			_publications.erase(it);
	}
	return pListener;
}

void Invoker::unsubscribe(Peer& peer,const string& name) {
	auto it = _publications.find(name);
	if(it == _publications.end()) {
		DEBUG("The publication '",name,"' doesn't exists, unsubscribe useless");
		return;
	}
	Publication& publication(it->second);
	publication.removeListener(peer);
	if(!publication.running() && publication.listeners.count()==0)
		_publications.erase(it);
}

string& Invoker::publicationName(string& name,string& query) {
	size_t found(name.find('?'));
	if (found != string::npos) {
		query = (&name[found]+1);
		name.assign(name,0,found);
	} else
		query.clear();
	return name;
}


} // namespace Mona
