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

#include "Mona/Publication.h"
#include "Mona/MediaCodec.h"
#include "Mona/Logs.h"

using namespace std;



namespace Mona {

Publication::Publication(const string& name,const PoolBuffers& poolBuffers): _audioCodecBuffer(poolBuffers),_videoCodecBuffer(poolBuffers),_new(false),_name(name),_lastTime(0),listeners(_listeners),_pPublisher(NULL) {
	DEBUG("New publication ",_name);
}

Publication::~Publication() {
	// delete _listeners!
	if (!_listeners.empty()) {
		ERROR("Publication ",_name," with subscribers is deleting")
		while (!_listeners.empty())
			removeListener((Peer&)*_listeners.begin()->first);
	}

	DEBUG("Publication ",_name," deleted");
}

void Publication::setBufferTime(UInt32 ms) {
	// TODO?
}

Listener* Publication::addListener(Exception& ex, Peer& peer,Writer& writer) {
	map<Client*,Listener*>::iterator it = _listeners.lower_bound(&peer);
	if(it!=_listeners.end() && it->first==&peer) {
		WARN("Already subscribed for publication ",_name);
		return it->second;
	}
	if(it!=_listeners.begin())
		--it;
	Listener* pListener = new Listener(*this,peer,writer);
	string error;
	if(peer.onSubscribe(*pListener,error)) {
		_listeners.insert(it,pair<Client*,Listener*>(&peer,pListener));
		if (_pPublisher)
			pListener->startPublishing();

		return pListener;
	}
	if(error.empty())
		error = "Not authorized to play " + _name;
	delete pListener;
	ex.set(Exception::APPLICATION, error);
	return NULL;
}

void Publication::removeListener(Peer& peer) {
	map<Client*,Listener*>::iterator it = _listeners.find(&peer);
	if(it==_listeners.end()) {
		WARN("Already unsubscribed of publication ",_name);
		return;
	}
	Listener* pListener = it->second;
	_listeners.erase(it);
	peer.onUnsubscribe(*pListener);
	delete pListener;
}


void Publication::start(Exception& ex, Peer& peer, Type type) {
	if(_pPublisher) { // has already a publisher
		ex.set(Exception::SOFTWARE, _name, " is already publishing");
		return;
	}
	_pPublisher = &peer;
	string error;
	if(!peer.onPublish(*this,error)) {
		if(error.empty())
			error = "Not allowed to publish " + _name;
		_pPublisher=NULL;
		ex.set(Exception::APPLICATION, error);
		return;
	}
	INFO("Publication ", _name, " started")
	for(auto& it : _listeners)
		it.second->startPublishing();
	flush();
}

void Publication::stop(Peer& peer) {
	if(!_pPublisher)
		return; // already done
	if(_pPublisher!=&peer) {
		ERROR("Unpublish '",_name,"' operation with a different publisher");
		return;
	}
	INFO("Publication ", _name, " stopped")
	for(auto& it : _listeners)
		it.second->stopPublishing();
	flush();
	_videoQOS.reset();
	_audioQOS.reset();
	_dataQOS.reset();
	_lastTime=0;
	_pPublisher=NULL;
	_videoCodecBuffer.release();
	_audioCodecBuffer.release();
	peer.onUnpublish(*this);
}

void Publication::flush() {
	if (!_new)
		return;
	_new = false;
	map<Client*,Listener*>::const_iterator it;
	for(it=_listeners.begin();it!=_listeners.end();++it)
		it->second->flush();
	_pPublisher->onFlushPackets(*this);
}

void Publication::pushData(DataReader& reader,UInt32 numberLostFragments) {
	if(!_pPublisher) {
		ERROR("Data packet pushed on '",_name,"' publication which has no publisher");
		return;
	}

	_new = true;
	int pos = reader.packet.position();
	_dataQOS.add(_pPublisher->ping,reader.available()+4,reader.packet.fragments,numberLostFragments); // 4 for time encoded
	auto it = _listeners.begin();
	while(it!=_listeners.end()) {
		(it++)->second->pushDataPacket(reader);   // listener can be removed in this call
		reader.packet.reset(pos);
	}
	_pPublisher->onDataPacket(*this,reader);
}


void Publication::pushAudio(UInt32 time,PacketReader& packet,UInt32 numberLostFragments) {
	if(!_pPublisher) {
		ERROR("Audio packet pushed on '",_name,"' publication which has no publisher");
		return;
	}

//	TRACE("Time Audio ",time)

	if(numberLostFragments>0)
		INFO(numberLostFragments," audio fragments lost on publication ",_name);
	_audioQOS.add(_pPublisher->ping,packet.available()+4,packet.fragments,numberLostFragments); // 4 for time encoded

	// save audio codec packet for future listeners
	if (MediaCodec::AAC::IsCodecInfos(packet)) {
		DEBUG("AAC codec infos received on publication ",_name)
		// AAC codec && settings codec informations
		_audioCodecBuffer->resize(packet.available(),false);
		memcpy(_audioCodecBuffer->data(),packet.current(),packet.available());
	}

	_new = true;
	UInt32 pos = packet.position();
	auto it = _listeners.begin();
	while(it!=_listeners.end()) {
		(it++)->second->pushAudioPacket(time,packet);  // listener can be removed in this call
		packet.reset(pos);
	}
	_pPublisher->onAudioPacket(*this,_lastTime=time,packet);
}

void Publication::pushVideo(UInt32 time,PacketReader& packet,UInt32 numberLostFragments) {
	if(!_pPublisher) {
		ERROR("Video packet pushed on '",_name,"' publication which has no publisher");
		return;
	}

	// string buffer;
	// TRACE("Time Video ",time," => ",Util::FormatHex(packet.current(),16,buffer))
	

	_videoQOS.add(_pPublisher->ping,packet.available()+4,packet.fragments,numberLostFragments); // 4 for time encoded
	if(numberLostFragments>0)
		INFO(numberLostFragments," video fragments lost on publication ",_name);


	// save video codec packet for future listeners
	if (MediaCodec::H264::IsCodecInfos(packet)) {
		DEBUG("H264 codec infos received on publication ",_name)
		// h264 codec && settings codec informations
		_videoCodecBuffer->resize(packet.available(), false);
		memcpy(_videoCodecBuffer->data(), packet.current(), packet.available());
	}


	_new = true;
	UInt32 pos = packet.position();
	auto it = _listeners.begin();
	while(it!=_listeners.end()) {
		(it++)->second->pushVideoPacket(time,packet); // listener can be removed in this call
		packet.reset(pos);
	}
	_pPublisher->onVideoPacket(*this,_lastTime=time,packet);
}


} // namespace Mona
