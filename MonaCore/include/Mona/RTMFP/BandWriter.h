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
#include "Mona/PacketWriter.h"

namespace Mona {

class RTMFPWriter;
class BandWriter : public virtual Object {
public:
	virtual const PoolBuffers&				poolBuffers() = 0;
	virtual void							initWriter(const std::shared_ptr<RTMFPWriter>& pWriter)=0;
	virtual std::shared_ptr<RTMFPWriter>	changeWriter(RTMFPWriter& writer) = 0;

	virtual bool						failed() const = 0;
	virtual bool						canWriteFollowing(RTMFPWriter& writer)=0;
	virtual UInt32						availableToWrite()=0;
	virtual BinaryWriter&				writeMessage(UInt8 type,UInt16 length,RTMFPWriter* pWriter=NULL)=0;
	virtual void						flush()=0;
	virtual UInt16						ping() const = 0;
	
};


} // namespace Mona
