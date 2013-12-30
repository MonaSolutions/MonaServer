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

#include "Mona/HTTP/HTTPWriter.h"
#include "Mona/HTTP/HTTP.h"
#include "Mona/String.h"

using namespace std;

namespace Mona {

UInt8	HTTPWriter::HLSInitVideoBuff[] = {
0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x81, 0x80, 
0x05, 0x21
};

UInt8	HTTPWriter::BeginBuff1[] = {
0x47, 0x40, 0x00, 0x30, 0xa6, 0x00, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0x00, 0x00, 0xb0, 0x0d, 0x00,
0x01, 0xc1, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x20,
0xa2, 0xc3, 0x29, 0x41,
};

UInt8	HTTPWriter::BeginBuff2[] = {
0x47, 0x40, 0x20, 0x30, 0x8b, 0x00, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0x00, 0x02, 0xb0, 0x28, 0x00, 0x01, 0xc1, 0x00,
0x00, 0xe0, 0x40, 0xf0, 0x0c, 0x05, 0x04, 0x48,
0x44, 0x4d, 0x56, 0x88, 0x04, 0x0f, 0xff, 0xfc,
0xfc, 0x1b, 0xe0, 0x40, 0xf0, 0x0a, 0x05, 0x08,
0x48, 0x44, 0x4d, 0x56, 0xff, 0x1b, 0x44, 0x3f,
0xfb, 0xa2, 0xe2, 0x49,
};

UInt32 HTTPWriter::CounterRow(0);
UInt32 HTTPWriter::CounterFrame(0);
char HTTPWriter::CounterA(0);
UInt32 HTTPWriter::BeginTime(0);

HTTPWriter::HTTPWriter(StreamSocket& socket) : _socket(socket) {
	
}


HTTPPacketWriter& HTTPWriter::newWriter() {
	HTTPSender* pSender = new HTTPSender();
	_senders.emplace_back(pSender);
	return pSender->writer;
}

void HTTPWriter::flush(bool full) {
	if(_senders.empty())
		return;
	// TODO _qos.add(ping,_sent);
	FLUSH_SENDERS("HTTPSender flush", HTTPSender, _senders)
}


HTTPWriter::State HTTPWriter::state(State value,bool minimal) {
	State state = Writer::state(value,minimal);
	if(state==CONNECTED && minimal)
		_senders.clear();
	return state;
}



DataWriter& HTTPWriter::writeInvocation(const std::string& name) {
	DataWriter& writer = writeMessage();
	string header("HTTP/1.1 ");
	Exception ex;
	UInt16 code(0);
	if (String::ToNumber<UInt16>(name, code)) {
		string message;
		header.append(name);
		header.append(" ");
		HTTP::CodeToMessage(code,message);
		if(!message.empty())
			header.append(message);
	} else
		header.append(name);
	writer.writeString(header);
	return writer;
}

DataWriter& HTTPWriter::writeMessage() {
	if(state()==CLOSED)
        return DataWriter::Null;
	return newWriter();
}

bool HTTPWriter::writeMedia(MediaType type, UInt32 time, MemoryReader& data) {

	switch(type) {
		case INIT:
			INFO("INIT HLS");
			break;
		case START:
			INFO("Start HLS ");
			writeHeader();
			break;
		case STOP:
			INFO("Stop HLS");
			break;
		case AUDIO:
			//writeMessage().writeBytes(data.current(), data.available());
			break;
		case VIDEO:
			if (data.available()>13 && *(data.current()+9)==0x09) {
				
				// Record time offset
				if (!BeginTime)
					BeginTime = 0; //BeginTime = time - (time % 1000);

				bool isMetadata = *data.current() == 0x17;
				data.next(9);
				BinaryWriter& writer = writeMessage().writer;

				UInt32 available = data.available() - 5;
				UInt32 max = isMetadata? HLS_PACKET_SIZE - 24 : HLS_PACKET_SIZE - 30;

				// Big Frame
				if (available > max) {

					// First row of frame
					writeVideoPacket(writer, max, time-BeginTime, data.current(), isMetadata, FIRST);
					data.next(max + 1); // +1 because of patch
					available = data.available();

					if (available <= 4) // ignore last 5 bytes
						return true;
					
					// Next rows of frame
					while (available - 4 > HLS_PACKET_SIZE - 6) {

						writeVideoPacket(writer, HLS_PACKET_SIZE - 4, time-BeginTime, data.current(), isMetadata, OTHER);
						data.next(HLS_PACKET_SIZE - 4);
						available = data.available();

						if (available <= 4)  // ignore last 5 bytes
							return true;
					}

					// Last row of frame
					writeVideoPacket(writer, available - 4, time-BeginTime, data.current(), isMetadata, LAST);
				}
				else 
					writeVideoPacket(writer, available, time-BeginTime, data.current(), isMetadata, UNIQUE);
			} else {

				WARN("Video data is not well formated (available : ", data.available(), ")")
				DUMP(data)
			}
			break;
		case DATA:
			INFO("HLS Data");
			break;
	}
	return true;
}

void HTTPWriter::writeVideoPacket(BinaryWriter& writer, UInt32 available, UInt32 time, UInt8* pData, bool isMetadata, TypeFrame type) {

	//INFO("Video Packet Type = ", type, " available = ", available)
	//DUMP(pData, available)

	// TODO Audio data??
	if ((type == FIRST || type == UNIQUE) && (CounterFrame % 3) == 0) {

		writer.write8(0x47);
		writer.write8(0x40);
		writer.write8(0x00);
		writer.write8(0x30 + (CounterA & 0x0F));
		writer.writeRaw(&BeginBuff1[4], HLS_PACKET_SIZE - 4);
					
		writer.write8(0x47);
		writer.write8(0x40);
		writer.write8(0x20);
		writer.write8(0x30 + (CounterA++ & 0x0F));
		writer.writeRaw(&BeginBuff2[4], HLS_PACKET_SIZE - 4);
	}

	// header
	writer.write8(0x47);
	writer.write8((type == FIRST || type == UNIQUE)? 0x40 : 0x00);
	writer.write8(0x40);
	writer.write8(((type == OTHER)? 0x10 : 0x30) + (CounterRow++ & 0x0F)); // timestamp on first row

	// Adaptive Field
	UInt16 nbFFBytes = 0;
	if (type != OTHER) {
		
		if (type != LAST) { 

			CounterFrame++; // It is a new frame
			writer.write8(HLS_PACKET_SIZE - (5 + available + 18)); // size of Adaptive Field
			writer.write8(isMetadata? 0x40 : 0x10); // packet with timestamp

			if (!isMetadata) {
				// timestamp
				writer.write32(CounterFrame*180);
				writer.write8(0x00);
				writer.write8(0x00);
			}
			nbFFBytes = HLS_PACKET_SIZE - (available + 18) - (isMetadata? 6 : 12);
		} else { // LAST

			writer.write8(HLS_PACKET_SIZE - (5 + available)); // size of Adaptive Field
			writer.write8(0x00);
			nbFFBytes = HLS_PACKET_SIZE - (6 + available);
		}

		// empty cells
		for (int i = 0; i < nbFFBytes; i++)
			writer.write8(0xFF);

		// Video header
		if (type != LAST) {

			writer.writeRaw(HLSInitVideoBuff, 10); // Marker init video
			writer.write32(time*180); // Time
			writer.write32(1); // 0x00 0x00 0x00 0x01

			// Video data
			if (isMetadata) {
				writer.writeRaw(pData, 5);
				writer.write8(0x01); // Patch 0x17 by 0x01 at position 5
				writer.writeRaw(pData+6, 26);
				writer.write8(0x01); // Patch 0x05 by 0x01 at position 32
				writer.writeRaw(pData+33, 7);
				writer.write8(0x01); // Patch 0x00 43 by 0x01 at position 41
				writer.writeRaw(pData+42, available - 41);
			} else {
				writer.writeRaw(pData, 4);
				writer.write8(0x01); // Patch 0x00 0x01 by 0x01 at position 5
				writer.writeRaw(pData+6, available -5);
			}
		} else // LAST
			writer.writeRaw(pData, available);

	} else // OTHER
		writer.writeRaw(pData, available);
}

void HTTPWriter::writeHeader() {

	DataWriter& response = writeMessage();
	response.writeString("HTTP/1.1 200 OK");
	response.beginObject();
	string stDate;
	response.writeStringProperty("Date", Time().toString(Time::HTTP_FORMAT, stDate));
	response.writeStringProperty("Content-Type", "video/mp2t");
	response.writeStringProperty("Accept-Ranges", "bytes");
	response.writeStringProperty("Server","Mona");
	response.writeStringProperty("Cache-Control", "no-cache");
	response.writeNumberProperty("Content-Length", HLS_PACKET_SIZE*1050);
	response.endObject();
	
	// TODO write the first audio buffer dynamicly
	//BinaryWriter& writer = response.writer;
	//writer.writeRaw(BeginBuff1, HLS_PACKET_SIZE);
	//writer.writeRaw(BeginBuff2, HLS_PACKET_SIZE);
	INFO("START : CounterA = ", (int)CounterA, " CounterFrame = ", CounterFrame, " CounterRow = ", CounterRow)
}

} // namespace Mona
