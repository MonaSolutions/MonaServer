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

#include "Mona/Publication.h"
#include "Mona/Logs.h"

using namespace std;



namespace Mona {

Publication::Publication(const string& name):_name(name),_droppedFrames(0),_firstKeyFrame(false),listeners(_listeners),_pPublisher(NULL) {
	DEBUG("New publication ",_name);
}

Publication::~Publication() {
	// delete _listeners!
	map<Client*,Listener*>::iterator it;
	for(it=_listeners.begin();it!=_listeners.end();++it)
		delete it->second;

	DEBUG("Publication ",_name," deleted");
}

void Publication::setBufferTime(UInt32 ms) {
	// TODO?
}

Listener* Publication::addListener(Exception& ex, Peer& peer,Writer& writer,bool unbuffered) {
	map<Client*,Listener*>::iterator it = _listeners.lower_bound(&peer);
	if(it!=_listeners.end() && it->first==&peer) {
		WARN("Already subscribed for publication ",_name);
		return it->second;
	}
	if(it!=_listeners.begin())
		--it;
	Listener* pListener = new Listener(*this,peer,writer,unbuffered);
	string error;
	if(peer.onSubscribe(*pListener,error)) {
		_listeners.insert(it,pair<Client*,Listener*>(&peer,pListener));
		if(_pPublisher)
			pListener->startPublishing();
		return pListener;
	}
	if(error.empty())
		error = "Not authorized to play " + _name;
	delete pListener;
	ex.set(Exception::NETWORK, error);
	return NULL;
}

void Publication::removeListener(Peer& peer) {
	map<Client*,Listener*>::iterator it = _listeners.find(&peer);
	if(it==_listeners.end()) {
		WARN("Already unsubscribed of publication ",_name);
		return;
	}
	Listener* pListener = it->second;
	peer.onUnsubscribe(*pListener);
	_listeners.erase(it);
	delete pListener;
}


void Publication::start(Exception& ex, Peer& peer) {
	if(_pPublisher) { // has already a publisher
		ex.set(Exception::NETWORK, _name, " is already publishing");
		return;
	}
	_pPublisher = &peer;
	string error;
	if(!peer.onPublish(*this,error)) {
		if(error.empty())
			error = "Not allowed to publish " + _name;
		_pPublisher=NULL;
		ex.set(Exception::NETWORK, error);
		return;
	}
	_firstKeyFrame=false;
	map<Client*,Listener*>::const_iterator it;
	for(it=_listeners.begin();it!=_listeners.end();++it)
		it->second->startPublishing();
	flush();
}

void Publication::stop(Peer& peer) {
	if(!_pPublisher)
		return; // already done
	if(_pPublisher!=&peer) {
		ERROR("Unpublish '",_name,"' operation with a different publisher");
		return;
	}
	map<Client*,Listener*>::const_iterator it;
	for(it=_listeners.begin();it!=_listeners.end();++it)
		it->second->stopPublishing();
	flush();
	peer.onUnpublish(*this);
	_videoQOS.reset();
	_audioQOS.reset();
	_dataQOS.reset();
	_droppedFrames=0;
	_pPublisher=NULL;
	return;
}

void Publication::flush() {
	map<Client*,Listener*>::const_iterator it;
	for(it=_listeners.begin();it!=_listeners.end();++it)
		it->second->flush();
	_pPublisher->onFlushPackets(*this);
}

void Publication::pushData(DataReader& packet,UInt32 numberLostFragments) {
	if(!_pPublisher) {
		ERROR("Data packet pushed on '",_name,"' publication which has no publisher");
		return;
	}
	int pos = packet.reader.position();
	_dataQOS.add(_pPublisher->ping,packet.available()+4,packet.reader.fragments,numberLostFragments); // 4 for time encoded
	map<Client*,Listener*>::const_iterator it;
	for(it=_listeners.begin();it!=_listeners.end();++it) {
		it->second->pushDataPacket(packet);
		packet.reader.reset(pos);
	}
	_pPublisher->onDataPacket(*this,packet);
}


void Publication::pushAudio(MemoryReader& packet,UInt32 time,UInt32 numberLostFragments) {
	if(!_pPublisher) {
		ERROR("Audio packet pushed on '",_name,"' publication which has no publisher");
		return;
	}

	int pos = packet.position();
	if(numberLostFragments>0)
		INFO(numberLostFragments," audio fragments lost on publication ",_name);
	_audioQOS.add(_pPublisher->ping,packet.available()+4,packet.fragments,numberLostFragments); // 4 for time encoded
	map<Client*,Listener*>::const_iterator it;
	for(it=_listeners.begin();it!=_listeners.end();++it) {
		it->second->pushAudioPacket(packet,time);
		packet.reset(pos);
	}
	_pPublisher->onAudioPacket(*this,time,packet);
}

void Publication::pushVideo(MemoryReader& packet,UInt32 time,UInt32 numberLostFragments) {
	if(!_pPublisher) {
		ERROR("Video packet pushed on '",_name,"' publication which has no publisher");
		return;
	}
	

	// if some lost packet, it can be a keyframe, to avoid break video, we must wait next key frame
	if(numberLostFragments>0)
		_firstKeyFrame=false;

	// is keyframe?
	if(((*packet.current())&0xF0) == 0x10)
		_firstKeyFrame = true;

	_videoQOS.add(_pPublisher->ping,packet.available()+4,packet.fragments,numberLostFragments); // 4 for time encoded
	if(numberLostFragments>0)
		INFO(numberLostFragments," video fragments lost on publication ",_name);

	if(!_firstKeyFrame) {
		DEBUG("No key frame available on publication ",_name,", frame dropped to wait first key frame");
		++_droppedFrames;
		return;
	}

	int pos = packet.position();
	map<Client*,Listener*>::const_iterator it;
	for(it=_listeners.begin();it!=_listeners.end();++it) {
		it->second->pushVideoPacket(packet,time);
		packet.reset(pos);
	}
	_pPublisher->onVideoPacket(*this,time,packet);
}


} // namespace Mona
