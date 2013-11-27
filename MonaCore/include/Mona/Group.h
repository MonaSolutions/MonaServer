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
#include "Mona/Peer.h"
#include <map>
#include <cstring>


namespace Mona {


class GroupIterator : virtual Object {
	friend class Group;
public:
	GroupIterator():_pPeers(NULL){}
	GroupIterator(const GroupIterator& other) :_pPeers(other._pPeers),_it(other._it) {}
	GroupIterator(std::map<UInt32,Peer*>& peers,bool end=false) : _pPeers(&peers),_it(end ? peers.end() : peers.begin()) {}
	bool		  operator !=(const GroupIterator& other) { return _it!=other._it; }
	bool		  operator ==(const GroupIterator& other) { return _it==other._it; }
    GroupIterator operator ++() { ++_it; return *this; }
    GroupIterator operator --() { --_it; return *this; }
    Client*		  operator *() { if(_pPeers && _it!=_pPeers->end()) return _it->second; return NULL; }
private:
	std::map<UInt32,Peer*>*				_pPeers; 
	std::map<UInt32,Peer*>::const_iterator _it;
};


class Group : public Entity, virtual Object {
	friend class Peer;
public:
	Group(const UInt8* id) {
		std::memcpy((UInt8*)this->id,id,ID_SIZE);
	}
	virtual ~Group(){}

	typedef GroupIterator Iterator;

	Iterator begin() { return GroupIterator(_peers); }
	Iterator end() { return GroupIterator(_peers, true); }
	UInt32  size() { return _peers.size(); }

	static UInt32 Distance(Iterator& it0, Iterator& it1) { return distance(it0._it, it1._it); }
	static void Advance(Iterator& it, UInt32 count) { advance(it._it, count); }

private:
	std::map<UInt32,Peer*> 	_peers;
};



} // namespace Mona
