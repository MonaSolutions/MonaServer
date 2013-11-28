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

#include "Mona/Peer.h"
#include "Mona/Group.h"
#include "Mona/Handler.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"

using namespace std;


namespace Mona {

class Member : virtual Object {
public:
	Member(UInt32	index,Writer* pWriter) : index(index),pWriter(pWriter){}
	const UInt32	index;
	Writer*			pWriter;
};

Peer::Peer(Handler& handler) :Client(turnPeers), _handler(handler), connected(false), addresses(1), relayable(false) {
}

Peer::Peer(const Peer& peer) : Client(peer), turnPeers(peer.turnPeers), _handler(peer._handler), connected(peer.connected), relayable(false), addresses(peer.addresses) {
	for(auto& it: peer)
		setRaw(it.first,it.second);
}

Peer::~Peer() {
	unsubscribeGroups();
	if(relayable)
		_handler.relay.remove(*this);
	for(auto& it: _ices) {
		((Peer*)it.first)->_ices.erase(this);
		delete it.second;
	}
}

Group& Peer::joinGroup(const UInt8* id,Writer* pWriter) {
	// create group if need
	Entities<Group>::Map::iterator it = _handler._groups.lower_bound(id);
	Group* pGroup = NULL;
	if(it==_handler._groups.end() || memcmp(it->first,id,ID_SIZE)!=0) {
		pGroup = new Group(id);
		_handler._groups.insert(it,pair<const UInt8*,Group*>(pGroup->id,pGroup));
	} else
		pGroup = it->second;
	joinGroup(*pGroup,pWriter);
	return *pGroup;
}

bool Peer::writeId(Group& group,Peer& peer,Writer* pWriter) {
	if(pWriter)
		pWriter->writeMember(peer);
	else {
		map<Group*,Member*>::const_iterator it = peer._groups.find(&group);
		if(it==peer._groups.end()) {
			CRITIC("A peer in a group without have its _groups collection associated")
			return false;
		}
		if(!it->second->pWriter)
			return false;
		it->second->pWriter->writeMember(*this);
	}
	return true;
}

void Peer::joinGroup(Group& group,Writer* pWriter) {
	UInt16 count=5;
	Group::Iterator it0=group.end();
	while(group.size()>0) {
		if(group.begin()==it0)
			break;
		--it0;
		Client& client = **it0;
		if(client==this->id)
			continue;
		if(!writeId(group,(Peer&)client,pWriter))
			continue;
		if(--count==0)
			break;
	}

	// + 1 random!
 	if(it0!=group.end()) {
		Group::Iterator itBegin = group.begin();
		UInt32 distance = Group::Distance(itBegin,it0);
 		if(distance>0) {
 			Group::Advance(itBegin,rand() % distance);
			writeId(group,(Peer&)**itBegin,pWriter);
 		}
 	 }

	map<Group*,Member*>::iterator it = _groups.lower_bound(&group);
	if(it!=_groups.end() && it->first==&group)
		return;

	if(it!=_groups.begin())
		--it;

	UInt32 index = 0;
	if(!group._peers.empty()){
		index = group._peers.rbegin()->first+1;
		if(index<group._peers.rbegin()->first) {
			// max index reached, rewritten index!
			index=0;
			map<UInt32,Peer*>::iterator it1;
			for(it1=group._peers.begin();it1!=group._peers.end();++it1)
				(UInt32&)it1->first = index++;
		}
	}
	group._peers[index] = this;
	_groups.insert(it,pair<Group*,Member*>(&group,new Member(index,pWriter)));
	onJoinGroup(group);
}


void Peer::unjoinGroup(Group& group) {
	map<Group*,Member*>::iterator it = _groups.lower_bound(&group);
	if(it==_groups.end() || it->first!=&group)
		return;
	onUnjoinGroup(it);
}

void Peer::unsubscribeGroups() {
	map<Group*,Member*>::iterator it=_groups.begin();
	while(it!=_groups.end())
		onUnjoinGroup(it++);
}

bool Peer::setName(const string& name) {
	if(connected) {
		// add the new
		if(!_handler._clientsByName.insert(pair<string,Client*>(name,this)).second)
			return false;
		// remove the old
		string nameClient;
		if(getString("name", nameClient))
			_handler._clientsByName.erase(nameClient);
	}
	setString("name",name);
	return true;
}

ICE& Peer::ice(const Peer& peer) {
	map<const Peer*,ICE*>::iterator it = _ices.begin();
	while(it!=_ices.end()) {
		if(it->first == &peer) {
			it->second->setCurrent(*this);
			return *it->second;
		}
		if(it->second->elapsed()>120000) {
			delete it->second;
			_ices.erase(it++);
			continue;
		}
		if(it->first>&peer)
			break;
		++it;
	}
	if(it!=_ices.begin())
		--it;
	ICE& ice = *_ices.insert(it,pair<const Peer*,ICE*>(&peer,new ICE(*this,peer,_handler.relay)))->second; // is offer
	((Peer&)peer)._ices[this] = &ice;
	return ice;
}

/// EVENTS ////////


void Peer::onHandshake(UInt32 attempts,set<SocketAddress>& addresses) {
	string protocol;
	getString("protocol", protocol);
	_handler.onHandshake(protocol, address, path, *this, attempts, addresses);
}

void Peer::onRendezVousUnknown(const UInt8* peerId,set<SocketAddress>& addresses) {
	if (connected) {
		string protocol;
		getString("protocol", protocol);
		_handler.onRendezVousUnknown(protocol, id, addresses);
	} else
		WARN("Rendez-vous using before connection");
}

void Peer::onConnection(Exception& ex, Writer& writer,DataReader& parameters,DataWriter& response) {
	if(!connected) {
		_pWriter = &writer;

		writer.state(Writer::CONNECTING);
		_handler.onConnection(ex, *this,parameters,response);
		if (ex) {
			writer.state(Writer::CONNECTED,true);
			_pWriter = NULL;
			return;
		}
		writer.state(Writer::CONNECTED);
		string name;
		if (getString("name", name)) {
			if (!_handler._clientsByName.insert(pair<string, Client*>(name, this)).second) {
				ex.set(Exception::NETWORK, "Client with a '%s' name exists already", name);
				return;
			}
		}
		(bool&)connected = true;
		if (!_handler._clients.insert(pair<const UInt8*, Client*>(id, this)).second) {
			string hex;
			ERROR("Client ", Util::FormatHex(id, ID_SIZE, hex), " seems already connected!")
		}
	} else {
		string hex;
		ERROR("Client ", Util::FormatHex(id, ID_SIZE, hex), " seems already connected!")
	}
}

void Peer::onDisconnection() {
	if(connected) {
		_pWriter = NULL;
		(bool&)connected = false;
		string name;
		if (getString("name", name))
			_handler._clientsByName.erase(name);
		if (_handler._clients.erase(id) == 0) {
			string hex;
			ERROR("Client ",Util::FormatHex(id,ID_SIZE,hex)," seems already disconnected!")
		}
		_handler.onDisconnection(*this);
	}
}

void Peer::onMessage(Exception& ex, const string& name,DataReader& reader,Mona::DataWriter& writer) {
	if(connected)
		_handler.onMessage(ex, *this, name, reader, writer);
	else
		ERROR("RPC client before connection")
}

void Peer::onJoinGroup(Group& group) {
	if(!connected)
		return;
	_handler.onJoinGroup(*this,group);
}

void Peer::onUnjoinGroup(map<Group*,Member*>::iterator it) {
	Group& group = *it->first;
	map<UInt32,Peer*>::iterator itPeer = group._peers.find(it->second->index);
	group._peers.erase(itPeer++);
	delete it->second;
	_groups.erase(it);

	if(connected)
		_handler.onUnjoinGroup(*this,group);

	if(group.size()==0) {
		_handler._groups.erase(group.id);
		delete &group;
	} else if(itPeer!=group._peers.end()) {
		// if a peer disconnects of one group, give to its following peer the 6th preceding peer
		Peer& followingPeer = *itPeer->second;
		UInt8 count=6;
		while(--count!=0 && itPeer!=group._peers.begin())
			--itPeer;
		if(count==0)
			itPeer->second->writeId(group,followingPeer,NULL);
	}
}

bool Peer::onPublish(const Publication& publication,string& error) {
	if(connected)
		return _handler.onPublish(*this,publication,error);
	WARN("Publication client before connection")
	error = "Client must be connected before publication";
	return false;
}

void Peer::onUnpublish(const Publication& publication) {
	if(connected) {
		_handler.onUnpublish(*this,publication);
		return;
	}
	WARN("Unpublication client before connection")
}

bool Peer::onSubscribe(const Listener& listener,string& error) {
	if(connected)
		return _handler.onSubscribe(*this,listener,error);
	WARN("Subscription client before connection")
	error = "Client must be connected before subscription";
	return false;
}

void Peer::onUnsubscribe(const Listener& listener) {
	if(connected) {
		_handler.onUnsubscribe(*this,listener);
		return;
	}
	WARN("Unsubscription client before connection")
}

bool Peer::onRead(Exception& ex, string& filePath,DataReader& parameters) {

	if(connected)
		return _handler.onRead(ex, *this, filePath,parameters);
	ERROR("Resource '",filePath,"' access by a not connected client")
	return false;
}

void Peer::onDataPacket(const Publication& publication,DataReader& packet) {
	if(connected) {
		_handler.onDataPacket(*this,publication,packet);
		return;
	}
	WARN("DataPacket client before connection")
}

void Peer::onAudioPacket(const Publication& publication,UInt32 time,MemoryReader& packet) {
	if(connected) {
		_handler.onAudioPacket(*this,publication,time,packet);
		return;
	}
	WARN("AudioPacket client before connection")
}

void Peer::onVideoPacket(const Publication& publication,UInt32 time,MemoryReader& packet) {
	if(connected) {
		_handler.onVideoPacket(*this,publication,time,packet);
		return;
	}
	WARN("VideoPacket client before connection")
}

void Peer::onFlushPackets(const Publication& publication) {
	if(connected) {
		_handler.onFlushPackets(*this,publication);
		return;
	}
	WARN("FlushPackets client before connection")
}





} // namespace Mona
