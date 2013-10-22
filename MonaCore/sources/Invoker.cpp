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

#include "Mona/Invoker.h"
#include "Mona/Logs.h"

using namespace std;
using namespace Poco;

namespace Mona {


Invoker::Invoker(UInt32 bufferSize,UInt32 threads) : poolThreads(threads),relay(poolThreads,bufferSize),sockets(*this,poolThreads,bufferSize),clients(_clients,_clientsByName),groups(_groups),publications(_publications),_nextId(0) {
	DEBUG(poolThreads.threadsAvailable()," threads available in the server poolthreads");
		
}


Invoker::~Invoker() {
	// delete groups
	Entities<Group>::Iterator it;
	for(it=_groups.begin();it!=_groups.end();++it)
		delete it->second;
	// delete publications
	Publications::Iterator it2;
	for(it2=_publications.begin();it2!=_publications.end();++it2)
		delete it2->second;
}

UInt32 Invoker::createFlashStream(Peer& peer) {
	AutoPtr<FlashStream> pStream(new FlashStream(*this,peer));
	pair<map<UInt32,AutoPtr<FlashStream> >::iterator, bool> result;
	do {
		result = _streams.insert(pair<UInt32,AutoPtr<FlashStream> >((++_nextId)==0 ? ++_nextId : _nextId,pStream));
	} while(!result.second);
	(UInt32&)pStream->id = _nextId;
	return _nextId;
}

Publication* Invoker::publish(Exception& ex, Peer& peer,const string& name) {
	Publications::Iterator it = createPublication(name);
	Publication * publication = it->second;
	
	publication->start(ex, peer);
	if (ex) {
		if(!publication->publisher() && publication->listeners.count()==0)
			destroyPublication(it);
		return NULL;
	}
	return publication;
}

void Invoker::unpublish(Peer& peer,const string& name) {
	Publications::Iterator it = _publications.find(name);
	if(it == _publications.end()) {
		DEBUG("The publication '",name,"' doesn't exist, unpublish useless");
		return;
	}
	Publication& publication(*it->second);
	publication.stop(peer);
	if(!publication.publisher() && publication.listeners.count()==0)
		destroyPublication(it);
}

Listener* Invoker::subscribe(Exception& ex, Peer& peer,const string& name,Writer& writer,double start) {
	Publications::Iterator it = createPublication(name);
	Publication& publication(*it->second);
	Listener* pListener = publication.addListener(ex, peer,writer,start==-3000 ? true : false);
	if (ex) {
		if(!publication.publisher() && publication.listeners.count()==0)
			destroyPublication(it);
	}
	return pListener;
}

void Invoker::unsubscribe(Peer& peer,const string& name) {
	Publications::Iterator it = _publications.find(name);
	if(it == _publications.end()) {
		DEBUG("The publication '",name,"' doesn't exists, unsubscribe useless");
		return;
	}
	Publication& publication(*it->second);
	publication.removeListener(peer);
	if(!publication.publisher() && publication.listeners.count()==0)
		destroyPublication(it);
}

Publications::Iterator Invoker::createPublication(const string& name) {
	Publications::Iterator it = _publications.lower_bound(name);
	if(it!=_publications.end() && it->first==name)
		return it;
	if(it!=_publications.begin())
		--it;
	return _publications.insert(it,pair<string,Publication*>(name,new Publication(name)));
}

void Invoker::destroyPublication(const Publications::Iterator& it) {
	delete it->second;
	_publications.erase(it);
}



} // namespace Mona
