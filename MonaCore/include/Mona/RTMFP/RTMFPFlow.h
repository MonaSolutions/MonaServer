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
#include "Mona/FlashMainStream.h"
#include "Mona/PoolBuffers.h"
#include "Mona/RTMFP/RTMFPWriter.h"


namespace Mona {

class RTMFPPacket;
class RTMFPFragment;
class RTMFPFlow : public virtual Object {
public:
	RTMFPFlow(UInt64 id,const std::string& signature,Peer& peer,Invoker& invoker, BandWriter& band, const std::shared_ptr<FlashMainStream>& pMainStream);
	RTMFPFlow(UInt64 id,const std::string& signature,const std::shared_ptr<FlashStream>& pStream, Peer& peer,Invoker& invoker, BandWriter& band);
	virtual ~RTMFPFlow();

	const UInt64		id;

	bool critical() const { return _pWriter->critical; }

	void				receive(UInt64 stage,UInt64 deltaNAck,PacketReader& fragment,UInt8 flags);
	
	void				commit();

	void				fail(const std::string& error);

	bool				consumed() { return _completed; }
	
private:
	void				onFragment(UInt64 stage,PacketReader& fragment,UInt8 flags);

	void				complete();

	Peer&							_peer;
	Group*							_pGroup;

	bool							_completed;
	BandWriter&						_band;
	std::shared_ptr<RTMFPWriter>	_pWriter;
	const UInt64					_stage;
	std::shared_ptr<FlashStream>	_pStream;

	// Receiving
	RTMFPPacket*					_pPacket;
	std::map<UInt64,RTMFPFragment>	_fragments;
	UInt32							_numberLostFragments;
	const PoolBuffers&				_poolBuffers;
};


} // namespace Mona
