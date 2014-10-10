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
#include "Mona/ParameterWriter.h"
#include "Mona/SplitWriter.h"
#include "Mona/Peer.h"
#include "Mona/Logs.h"

using namespace std;



namespace Mona {

Publication::Publication(const string& name,const PoolBuffers& poolBuffers): _propertiesWriter(poolBuffers),_running(false),poolBuffers(poolBuffers),_audioCodecBuffer(poolBuffers),_videoCodecBuffer(poolBuffers),_new(false),_name(name),_lastTime(0),listeners(_listeners) {
	DEBUG("New publication ",_name);
}

Publication::~Publication() {
	// delete _listeners!
	if (!_listeners.empty()) {
		ERROR("Publication ",_name," with subscribers is deleting")
		while (!_listeners.empty())
			removeListener((Peer&)*_listeners.begin()->first);
	}
	if (_running)
		ERROR("Publication ",_name," running is deleting")
	DEBUG("Publication ",_name," deleted");
}


Listener* Publication::addListener(Exception& ex, Client& client,Writer& writer) {
	map<Client*,Listener*>::iterator it = _listeners.lower_bound(&client);
	if(it!=_listeners.end() && it->first==&client) {
		WARN("Already subscribed for publication ",_name);
		return it->second;
	}
	if(it!=_listeners.begin())
		--it;
	Listener* pListener = new Listener(*this,client,writer);
	if(((Peer&)client).onSubscribe(ex,*pListener)) {
		_listeners.insert(it,pair<Client*,Listener*>(&client,pListener));
		if (_running)
			pListener->startPublishing();
		return pListener;
	}
	if(!ex)
		ex.set(Exception::APPLICATION,"Not authorized to play ",_name);
	delete pListener;
	return NULL;
}

void Publication::removeListener(Client& client) {
	map<Client*,Listener*>::iterator it = _listeners.find(&client);
	if(it==_listeners.end()) {
		WARN("Already unsubscribed of publication ",_name);
		return;
	}
	Listener* pListener = it->second;
	_listeners.erase(it);
	((Peer&)client).onUnsubscribe(*pListener);
	delete pListener;
}


void Publication::start(Type type) {
	if (_running)
		return;
	INFO("Publication ", _name, " started")
	_running = true;
	for(auto& it : _listeners)
		it.second->startPublishing();
	flush();
}

void Publication::stop() {
	if(!_running)
		return; // already done
	INFO("Publication ", _name, " stopped")
	for(auto& it : _listeners)
		it.second->stopPublishing();
	flush();
	_properties.clear();
	_propertiesWriter.clear();
	_videoQOS.reset();
	_audioQOS.reset();
	_dataQOS.reset();
	_lastTime=0;
	_running=false;
	_videoCodecBuffer.release();
	_audioCodecBuffer.release();
}

void Publication::flush() {
	if (!_new)
		return;
	_new = false;
	map<Client*,Listener*>::const_iterator it;
	for(it=_listeners.begin();it!=_listeners.end();++it)
		it->second->flush();
	OnFlush::raise(*this);
}

void Publication::pushData(DataReader& reader, UInt16 ping, double lostRate) {
	if(!_running) {
		ERROR("Data packet pushed on '",_name,"' publication stopped");
		return;
	}

	_new = true;
	_dataQOS.add(reader.available()+4,ping,lostRate); // 4 for time encoded
	auto it = _listeners.begin();
	while(it!=_listeners.end()) {
		(it++)->second->pushData(reader);   // listener can be removed in this call
		reader.reset();
	}
	OnData::raise(*this, reader);
}


void Publication::pushAudio(UInt32 time,PacketReader& packet, UInt16 ping, double lostRate) {
	if(!_running) {
		ERROR("Audio packet pushed on '",_name,"' publication stopped");
		return;
	}

//	TRACE("Time Audio ",time)

	if(lostRate)
		INFO((UInt8)(lostRate*100),"% of audio information lost on publication ",_name);
	_audioQOS.add(packet.available()+4,ping,lostRate); // 4 for time encoded

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
		(it++)->second->pushAudio(time,packet);  // listener can be removed in this call
		packet.reset(pos);
	}
	OnAudio::raise(*this, _lastTime=time, packet);
}

void Publication::pushVideo(UInt32 time,PacketReader& packet, UInt16 ping, double lostRate) {
	if(!_running) {
		ERROR("Video packet pushed on '",_name,"' publication stopped");
		return;
	}

	// string buffer;
	// TRACE("Time Video ",time," => ",Util::FormatHex(packet.current(),16,buffer))
	

	_videoQOS.add(packet.available()+4,ping,lostRate); // 4 for time encoded
	if(lostRate)
		INFO((UInt8)(lostRate*100),"% video fragments lost on publication ",_name);


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
		(it++)->second->pushVideo(time,packet); // listener can be removed in this call
		packet.reset(pos);
	}
	OnVideo::raise(*this, _lastTime=time, packet);
}

void Publication::clearProperties()  {
	if (!_running) {
		ERROR("Properties can't be writing on ",_name," publication stopped");
		return;
	}
	_properties.clear();
	_propertiesWriter.clear();
	INFO("Clear ",_name," publication properties")
	OnProperties::raise(*this,_properties);
}

void Publication::writeProperties(DataReader& reader)  {
	if (!_running) {
		ERROR("Properties can't be writing on ",_name," publication stopped");
		return;
	}

	_properties.clear();
	_propertiesWriter.clear();
	
	_propertiesWriter.amf0 = true;
	if (reader.nextType() != DataReader::STRING)
		_propertiesWriter.writeString(EXPAND("onMetaData"));
	else
		reader.read(_propertiesWriter, 1);
	_propertiesWriter.amf0 = false;

	// flat all
	ParameterWriter writer(_properties);
	SplitWriter writers(writer,_propertiesWriter);
	reader.read(writers,1);

	auto it = _listeners.begin();
	while (it != _listeners.end())
		(it++)->second->updateProperties();  // listener can be removed in this call

	INFO("Write ",_name," publication properties")
	OnProperties::raise(*this,_properties);
}


} // namespace Mona
