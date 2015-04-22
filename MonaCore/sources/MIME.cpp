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
#include "Mona/QueryReader.h"
#include "Mona/QueryWriter.h"


using namespace std;


namespace Mona {

MIME::Type MIME::DataType(const char* type) {
	if (String::ICompare(type, EXPAND("json")) == 0)
		return JSON;
	if (String::ICompare(type, EXPAND("xml")) == 0)
		return XMLRPC;
	if (String::ICompare(type, EXPAND("amf")) == 0 || String::ICompare(type, EXPAND("x-amf")) == 0)
		return AMF;
	if (String::ICompare(type, EXPAND("x-www-form-urlencoded")) == 0)
		return QUERY;
	return UNKNOWN;
}

MIME::Type MIME::DataType(DataWriter& writer) {
	if (typeid(writer).name() == typeid(AMFWriter).name())
		return AMF;
	if (typeid(writer).name() == typeid(JSONWriter).name())
		return JSON;
	if (typeid(writer).name() == typeid(XMLRPCWriter).name())
		return XMLRPC;
	if (typeid(writer).name() == typeid(QueryWriter).name())
		return QUERY;
	return UNKNOWN;
}

MIME::Type MIME::DataType(DataReader& reader) {
	if (typeid(reader).name() == typeid(AMFReader).name())
		return AMF;
	if (typeid(reader).name() == typeid(JSONReader).name())
		return JSON;
	if (typeid(reader).name() == typeid(XMLRPCReader).name())
		return XMLRPC;
	if (typeid(reader).name() == typeid(QueryReader).name())
		return QUERY;
	return UNKNOWN;
}

bool MIME::CreateDataReader(Type type,PacketReader& packet,const PoolBuffers& poolBuffers,unique_ptr<DataReader>& pReader) {
	switch (type) {
		case JSON:
			pReader.reset(new JSONReader(packet, poolBuffers));
			if (((JSONReader&)*pReader).isValid())
				return true;
			break;
		case XMLRPC:
			pReader.reset(new XMLRPCReader(packet, poolBuffers));
			if (((XMLRPCReader&)*pReader).isValid())
				return true;
			break;
		case AMF:
			pReader.reset(new AMFReader(packet));
			return true;
		case QUERY:
			pReader.reset(new QueryReader(packet));
			return true;
		case UNKNOWN:
			break;
	}
	pReader.reset();
	packet.reset();
	return false;
}

bool MIME::CreateDataWriter(Type type,const PoolBuffers& poolBuffers,unique_ptr<DataWriter>& pWriter) {
	switch (type) {
		case JSON:
			pWriter.reset(new JSONWriter(poolBuffers));
			return true;
		case XMLRPC:
			pWriter.reset(new XMLRPCWriter(poolBuffers));
			return true;
		case AMF:
			pWriter.reset(new AMFWriter(poolBuffers));
			return true;
		case QUERY:
			pWriter.reset(new QueryWriter(poolBuffers));
			return true;
		case UNKNOWN:
			break;
	}
	return false;
}


} // namespace Mona
