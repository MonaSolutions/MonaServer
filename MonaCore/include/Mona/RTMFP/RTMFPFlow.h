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
#include "Mona/FlashStream.h"
#include "Mona/RTMFP/RTMFPWriter.h"
#include "Poco/AutoPtr.h"

namespace Mona {

class RTMFPPacket;
class RTMFPFragment;
class RTMFPFlow : virtual Object {
public:
	RTMFPFlow(UInt64 id,const std::string& signature,Peer& peer,Invoker& invoker,BandWriter& band);
	virtual ~RTMFPFlow();

	const UInt64		id;

	void				fragmentHandler(UInt64 stage,UInt64 deltaNAck,MemoryReader& fragment,UInt8 flags);
	
	void				commit();

	void				fail(const std::string& error);

	bool				consumed() { return _completed; }
	void				complete();
	
private:
	void				fragmentSortedHandler(UInt64 stage,MemoryReader& fragment,UInt8 flags);
	
	AMF::ContentType	unpack(MemoryReader& reader);

	bool							_completed;
	BandWriter&						_band;
	std::shared_ptr<RTMFPWriter>	_pWriter;
	const UInt64					_stage;
	std::shared_ptr<FlashStream>	_pStream;

	// Receiving
	RTMFPPacket*					_pPacket;
	std::map<UInt64,RTMFPFragment*>	_fragments;
	UInt32							_numberLostFragments;
};


} // namespace Mona
