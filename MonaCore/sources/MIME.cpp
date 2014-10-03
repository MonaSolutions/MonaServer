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

#include "Mona/MIME.h"
#include "Mona/JSONReader.h"
#include "Mona/JSONWriter.h"
#include "Mona/XMLRPCReader.h"
#include "Mona/XMLRPCWriter.h"
#include "Mona/AMFReader.h"
#include "Mona/AMFWriter.h"


using namespace std;


namespace Mona {


bool MIME::CreateDataReader(const char* type,PacketReader& packet,const PoolBuffers& poolBuffers,unique_ptr<DataReader>& pReader) {
	if (String::ICompare(type, EXPAND("json")) == 0) {
		pReader.reset(new JSONReader(packet, poolBuffers));
		if (((JSONReader&)*pReader).isValid())
			return true;
		packet.reset();
	} else if (String::ICompare(type, EXPAND("xml")) == 0) {
		pReader.reset(new XMLRPCReader(packet, poolBuffers));
		if (((XMLRPCReader&)*pReader).isValid())
			return true;
		packet.reset();
	} else if (String::ICompare(type, EXPAND("amf")) == 0 || String::ICompare(type, EXPAND("x-amf")) == 0) {
		pReader.reset(new AMFReader(packet));
		return true;
	}
	return false;
}

bool MIME::CreateDataWriter(const char* type,const PoolBuffers& poolBuffers,unique_ptr<DataWriter>& pWriter) {
	if (String::ICompare(type, EXPAND("json")) == 0)
		pWriter.reset(new JSONWriter(poolBuffers));
	else if (String::ICompare(type, EXPAND("xml")) == 0)
		pWriter.reset(new XMLRPCWriter(poolBuffers));
	else if (String::ICompare(type, EXPAND("amf")) == 0 || String::ICompare(type, EXPAND("x-amf")) == 0)
		pWriter.reset(new AMFWriter(poolBuffers));
	else
		return false;
	return true;
}


} // namespace Mona
