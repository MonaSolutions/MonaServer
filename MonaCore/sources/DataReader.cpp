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

#include "Mona/DataReader.h"
#include "Mona/Logs.h"

using namespace std;
using namespace Poco;

namespace Mona {

DataReader::DataReader(MemoryReader& reader): reader(reader),_pos(reader.position()) {
}

DataReader::~DataReader() {
}


bool DataReader::readMap(UInt32& size,bool& weakKeys) {
	bool external;
	string type;
	return readObject(type,external);
}

void DataReader::next() {
	DataWriterNull writerNull;
	read(writerNull,1);
}


void DataReader::read(DataWriter& writer,UInt32 count) {
	Type type;
	bool all=count==0;
	if(all)
		count=1;
	while(count>0 && (type = followingType())!=END) {
		read(type,writer);
		if(!all)
			--count;
	}
}

void DataReader::read(Type type,DataWriter& writer) {
	switch(type) {
		case NIL:
			readNull();
			writer.writeNull();
			break;
		case BOOLEAN:
			writer.writeBoolean(readBoolean());
			break;
		case NUMBER:
			writer.writeNumber(readNumber());
			break;
		case STRING: {
			string value;
			readString(value);
			writer.writeString(value);
			break;
		}
		case DATE:
			writer.writeDate(readDate());
			break;
		case BYTES: {
			UInt32 size;
			const UInt8* bytes = readBytes(size);
			writer.writeBytes(bytes,size);
			break;
		}
		case ARRAY: {
			UInt32 size;
			if(readArray(size)) {
				writer.beginArray(size);
				string name;
				while((type=readItem(name))!=END) {
					if(!name.empty())
						writer.writePropertyName(name);
					read(type,writer);
				}
				writer.endArray();
			}
			break;
		}
		case MAP: {
			bool weakKeys=false;
			UInt32 size=0;
			if(readMap(size,weakKeys)) {
				writer.beginMap(size,weakKeys);
				while((type=readKey())!=END) {
					read(type,writer);
					read(readValue(),writer);
				}
				writer.endMap();
			}
			break;
		}
		case OBJECT: {
			string objectType;
			bool external=false; // TODO what's happen when external==true (ArrayList in AMF for example)
			if(readObject(objectType,external)) {
				writer.beginObject(objectType,external);
				string name;
				while((type = readItem(name))!=END) {
					writer.writePropertyName(name);
					read(type,writer);
				}
				writer.endObject();
			}
			break;
		}	 
		default:
			ERROR("Unknown DataReader %.2x type",type);
	}
}



} // namespace Mona
