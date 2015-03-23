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
#include "Mona/PacketReader.h"
#include "Mona/DataWriter.h"

namespace Mona {

class DataReaderNull;

class DataReader : public virtual NullableObject {
public:
	enum {
		END=0, // keep equal to 0!
		NIL,
		BOOLEAN,
		NUMBER,
		STRING,
		DATE,
		BYTES,
		OTHER
	};

	void			next() { read(DataWriter::Null,1); }
	UInt8			nextType() { if (_nextType == END) _nextType = followingType(); return _nextType; }

	// return the number of writing success on writer object
	// can be override to capture many reading on the same writer
	virtual UInt32	read(DataWriter& writer,UInt32 count=END);

	bool			read(UInt8 type, DataWriter& writer);
	bool			available() { return nextType()!=END; }

////  OPTIONAL DEFINE ////
	virtual void	reset() { packet.reset(_pos); }
////////////////////


	bool			readString(std::string& value) { return read(STRING,wrapper(&value)); }
	bool			readNumber(double& number) {  return read(NUMBER,wrapper(&number)); }
	bool			readBoolean(bool& value) {  return read(BOOLEAN,wrapper(&value)); }
	bool			readDate(Date& date)  {  return read(DATE,wrapper(&date)); }
	bool			readNull() { return read(NIL,wrapper(NULL)); }
	template <typename BufferType>
	bool			readBytes(BufferType& buffer) { BytesWriter<BufferType> writer(buffer); return read(BYTES, writer); }

	operator bool() const { return packet; }

	PacketReader&				packet;

	static DataReaderNull		Null;

protected:
	DataReader(PacketReader& packet);
	DataReader(); // Null


	bool			readNext(DataWriter& writer);

private:
	
////  TO DEFINE ////
	// must return true if something has been written in DataWriter object (so if DataReader has always something to read and write, !=END)
	virtual bool	readOne(UInt8 type, DataWriter& writer) = 0;
	virtual UInt8	followingType()=0;
////////////////////

	UInt32						_pos;
	UInt8						_nextType;
	
	class WrapperWriter : public DataWriter {
	public:
		UInt64	beginObject(const char* type) { return 0; }
		void	writePropertyName(const char* value) {}
		void	endObject() {}
		
		UInt64	beginArray(UInt32 size) { return 0; }
		void	endArray() {}

		UInt64	writeDate(const Date& date) { if(pData) ((Date*)pData)->update(date); return 0; }
		void	writeNumber(double value) { if(pData) *((double*)pData) = value; }
		void	writeString(const char* value, UInt32 size) { if(pData)  ((std::string*)pData)->assign(value,size); }
		void	writeBoolean(bool value) { if(pData)  *((bool*)pData) = value; }
		void	writeNull() {}
		UInt64	writeBytes(const UInt8* data, UInt32 size) { return 0; }

		void*	pData;
	};

	template<typename BufferType>
	class BytesWriter : public DataWriter {
	public:
		BytesWriter(BufferType& buffer) : _buffer(buffer) {}
		UInt64	beginObject(const char* type, UInt32 size) { return 0; }
		void	writePropertyName(const char* value) {}
		void	endObject() {}
		
		UInt64	beginArray(UInt32 size) { return 0; }
		void	endArray() {}

		UInt64	writeDate(const Date& date) { Int64 time(date.time()); _buffer.write(&time, sizeof(time)); return 0; }
		void	writeNumber(double value) { _buffer.write(&value, sizeof(value)); }
		void	writeString(const char* value, UInt32 size) { _buffer.write(value, size); }
		void	writeBoolean(bool value) { _buffer.write(&value, sizeof(value)); }
		void	writeNull() {}
		UInt64	writeBytes(const UInt8* data, UInt32 size) { _buffer.write(data, size); return 0; }
	private:
		BufferType&  _buffer;
	};

	DataWriter&					wrapper(void* pData) { _wrapper.pData = pData; return _wrapper; }
	WrapperWriter				_wrapper;
};

class DataReaderNull : public DataReader {
public:
	DataReaderNull() {}

private:
	bool	readOne(UInt8 type, DataWriter& writer) { return false; }
	UInt8	followingType() { return END; }
};


} // namespace Mona
