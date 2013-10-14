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
class RTMFPFlow {
public:
	RTMFPFlow(Mona::UInt64 id,const std::string& signature,Peer& peer,Invoker& invoker,BandWriter& band);
	virtual ~RTMFPFlow();

	const Mona::UInt64		id;

	void				fragmentHandler(Mona::UInt64 stage,Mona::UInt64 deltaNAck,MemoryReader& fragment,Mona::UInt8 flags);
	
	void				commit();

	void				fail(const std::string& error);

	bool				consumed();
	void				complete();
	
private:
	void				fragmentSortedHandler(Mona::UInt64 stage,MemoryReader& fragment,Mona::UInt8 flags);
	
	AMF::ContentType	unpack(MemoryReader& reader);

	bool						_completed;
	BandWriter&					_band;
	Poco::AutoPtr<RTMFPWriter>	_pWriter;
	const Mona::UInt64			_stage;
	Poco::AutoPtr<FlashStream>	_pStream;

	// Receiving
	RTMFPPacket*								_pPacket;
	std::map<Mona::UInt64,RTMFPFragment*>	_fragments;
	Mona::UInt32							_numberLostFragments;
};

inline bool RTMFPFlow::consumed() {
	return _completed;
}


} // namespace Mona
