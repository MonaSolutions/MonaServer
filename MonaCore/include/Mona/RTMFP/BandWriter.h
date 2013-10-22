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
#include "Mona/Exceptions.h"
#include "Mona/MemoryWriter.h"

namespace Mona {

class RTMFPWriter;
class BandWriter {
public:
	BandWriter() {}
	virtual ~BandWriter() {}

	virtual void			initWriter(RTMFPWriter& writer)=0;
	virtual void			resetWriter(RTMFPWriter& writer)=0;
	virtual void			close()=0;

	virtual bool			failed() const = 0;
	virtual bool			canWriteFollowing(RTMFPWriter& writer)=0;
	virtual MemoryWriter&	writer()=0;
	virtual MemoryWriter&	writeMessage(Exception& ex, Mona::UInt8 type,Mona::UInt16 length,RTMFPWriter* pWriter=NULL)=0;
	virtual void			flush(Exception& ex, bool full=true)=0;
	
};


} // namespace Mona
