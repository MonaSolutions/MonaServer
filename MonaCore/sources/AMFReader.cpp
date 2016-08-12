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

#include "Mona/AMFReader.h"
#include "Mona/StringWriter.h"
#include "Mona/Logs.h"
#include "Mona/Exceptions.h"

using namespace std;

namespace Mona {



AMFReader::AMFReader(PacketReader& packet) : ReferableReader(packet),_amf3(0),_referencing(true) {

}

void AMFReader::reset() {
	DataReader::reset();
	_stringReferences.clear();
	_classDefReferences.clear();
	_references.clear();
	_amf0References.clear();
	_amf3 = 0;
	_referencing = true;
}

const char* AMFReader::readText(UInt32& size) {
	UInt32 reset(0), reference(0);
	if (_amf3) {
		reference = packet.position();
		size = packet.read7BitValue();
		bool isInline = size & 0x01;
		size >>= 1;
		if(!isInline) {
			if (size >= _stringReferences.size()) {
				ERROR("AMF3 string reference not found");
				return NULL;
			}
			reset = packet.position();
			packet.reset(_stringReferences[size]);
			size = (packet.read7BitValue() >> 1);
		}
	} else
		size = packet.read16();

	if (size > packet.available()) {
		ERROR("AMF text bad-formed with a ", size, " size exceeding the ", packet.available(), " bytes available");
		if (reset)
			packet.reset(reset);
		return NULL;
	}
	const char* text(STR packet.current());

	if (reset)
		packet.reset(reset);
	else if(size) { // not record string empty
		if (reference)
			_stringReferences.emplace_back(reference);
		packet.next(size);
	}

	return text;
}

UInt8 AMFReader::followingType() {

	if(!packet.available())
		return END;
	UInt8 type = *packet.current();
	
	if(_amf3) {
		switch(type) {
			case AMF3_UNDEFINED:
			case AMF3_NULL:
				return NIL;
			case AMF3_FALSE:
			case AMF3_TRUE:
				return BOOLEAN;
			case AMF3_INTEGER:
			case AMF3_NUMBER:
				return NUMBER;
			case AMF3_STRING:
				return STRING;
			case AMF3_DATE:
				return DATE;
			case AMF3_BYTEARRAY:
				return BYTES;
			case AMF3_ARRAY:
				return ARRAY;
			case AMF3_DICTIONARY:
				return MAP;
			case AMF3_OBJECT:
				return OBJECT;
		}

		ERROR("Unknown AMF3 type ",Format<UInt8>("%.2x",(UInt8)type))
		packet.next(packet.available());
		return END;
	}
		
	switch(type) {
		case AMF_AVMPLUS_OBJECT:
			packet.next();
			_amf3 = 1;
			return followingType();
		case AMF_UNDEFINED:
		case AMF_NULL:
			return NIL;
		case AMF_BOOLEAN:
			return BOOLEAN;
		case AMF_NUMBER:
			return NUMBER;
		case AMF_LONG_STRING:
		case AMF_STRING:
			return STRING;
		case AMF_MIXED_ARRAY:
		case AMF_STRICT_ARRAY:
			return ARRAY;
		case AMF_DATE:
			return DATE;
		case AMF_BEGIN_OBJECT:
		case AMF_BEGIN_TYPED_OBJECT:
			return OBJECT;
		case AMF_REFERENCE:
			return AMF0_REF;
		case AMF_END_OBJECT:
			ERROR("AMF0 end object type without begin object type before")
			packet.next(packet.available());
			return END;
		case AMF_UNSUPPORTED:
			WARN("Unsupported type in AMF0 format")
			packet.next(packet.available());
			return END;		
	}

	ERROR("Unknown AMF0 type ",Format<UInt8>("%.2x",type))
	packet.next(packet.available());
	return END;
}

bool AMFReader::readOne(UInt8 type, DataWriter& writer) {
	bool resetAMF3(false);
	if (_amf3 == 1) {
		resetAMF3 = true;
		_amf3 = 2;
	}

	bool written(writeOne(type, writer));

	if (resetAMF3)
		_amf3 = 0;

	return written;
}

bool AMFReader::writeOne(UInt8 type, DataWriter& writer) {

	switch (type) {

		case AMF0_REF: {
			packet.next();
			UInt16 reference = packet.read16();
			if(reference>=_amf0References.size()) {
				ERROR("AMF0 reference not found")
				return false;
			}
			if (writeReference(writer, (reference+1) << 1))
				return true;
			UInt32 reset(packet.position());
			packet.reset(_amf0References[reference]);
			bool referencing(_referencing);
			_referencing = false;
			bool written = readNext(writer);
			_referencing = referencing;
			packet.reset(reset);
			return written;
		}

		case STRING:  {
			type = packet.read8();
			UInt32 size(0);
			if (type == AMF_LONG_STRING) {
				size = packet.read32();
				if (size > packet.available()) {
					ERROR("AMF long string bad-formed with a ", size, " size exceeding the ", packet.available(), " bytes available");
					return false;
				}
				writer.writeString(STR packet.current(), size);
				packet.next(size);
				return true;
			}
			const char* value(readText(size));
			if (!value)
				return false;
			writer.writeString(value, size);
			return true;
		}

		case BOOLEAN:
			type = packet.read8();
			writer.writeBoolean(_amf3 ? (type == AMF3_TRUE) : packet.read8()!=0x00);
			return true;

		case NUMBER: {
			type = packet.read8();
			if (!_amf3 || type==AMF3_NUMBER) {
				writer.writeNumber(packet.readNumber<double>());
				return true;
			}
			// Forced in AMF3 here!
			UInt32 value = packet.read7BitValue();
			if(value>AMF_MAX_INTEGER)
				value-=(1<<29);
			writer.writeNumber(value);
			return true;
		}

		case NIL:
			packet.next();
			writer.writeNull();
			return true;

		case BYTES: {
			packet.next();
			// Forced in AMF3 here!
			UInt32 pos = packet.position();
			UInt32 size = packet.read7BitValue();
			bool isInline = size&0x01;
			size >>= 1;

			if (size > packet.available()) {
				ERROR("AMF ByteArray bad-formed with a ", size, " size exceeding the ", packet.available(), " bytes available");
				return false;
			}

			if(isInline) {
				if (_referencing) {
					_references.emplace_back(pos);
					writeBytes(writer, (_references.size()<<1) | 0x01, packet.current(), size);
				} else
					writer.writeBytes(packet.current(), size);
				packet.next(size);
				return true;
			}
			if(!writeReference(writer, ((size+1)<<1) | 0x01)) {
				if(size>=_references.size()) {
					ERROR("AMF3 reference not found")
					return false;
				}
				UInt32 reset = packet.position();
				packet.reset(_references[size]);
				writeBytes(writer, ((++size)<<1) | 0x01, packet.current(), packet.read7BitValue()>>1);
				packet.reset(reset);
			}
			return true;
		}

		case DATE: {
			packet.next();
			Date date;
			if (_amf3) {
				UInt32 pos = packet.position();
				UInt32 flags = packet.read7BitValue();
				bool isInline = flags & 0x01;
				if (isInline) {
					if (_referencing) {
						_references.emplace_back(pos);
						writeDate(writer, (_references.size()<<1) | 0x01, date = (Int64)packet.readNumber<double>());
					} else
						writer.writeDate(date = (Int64)packet.readNumber<double>());
					return true;
				}
				flags >>= 1;
				if(!writeReference(writer, ((flags+1)<<1) | 0x01)) {
					if (flags >= _references.size()) {
						ERROR("AMF3 reference not found")
						return false;
					}
					UInt32 reset = packet.position();
					packet.reset(_references[flags]);
					writeDate(writer,((++flags)<<1) | 0x01,date = (Int64)packet.readNumber<double>());
					packet.reset(reset);
				}
				return true;
			}
			
			writer.writeDate(date = (Int64)packet.readNumber<double>());
			packet.next(2); // Timezone, useless
			return true;
		}

		case MAP: {
			packet.next();
			// AMF3
			UInt32 reference = packet.position();
			UInt32 size = packet.read7BitValue();
			bool isInline = size&0x01;
			size >>= 1;


			UInt32 reset(0);
			bool referencing(_referencing);
			Reference* pReference(NULL);
			Exception ex;

			if (!isInline) {
				if (writeReference(writer, (((reference=size)+1)<<1) | 0x01))
					return true;
				if (size >= _references.size()) {
					ERROR("AMF3 map reference not found")
					return false;
				}
				reset = packet.position();
				packet.reset(_references[reference]);
				size = packet.read7BitValue() >> 1;
				pReference = beginMap(writer,((++reference)<<1) | 0x01, ex, size, packet.read8() & 0x01);
				_referencing = false;
			} else if (_referencing) {
				_references.emplace_back(reference);
				pReference = beginMap(writer,(_references.size()<<1) | 0x01,ex, size, packet.read8() & 0x01);
			} else
				writer.beginMap(ex, size, packet.read8() & 0x01);

			if (ex)
				WARN(ex.error());

			while (size-- > 0) {
				if (ex) {
					_buffer.clear();
					StringWriter stringWriter(_buffer);
					if (!readNext(stringWriter))
						continue;
					writer.writePropertyName(_buffer.c_str());
				} else if (!readNext(writer)) // key
					writer.writeNull();

				if (!readNext(writer)) // value
					writer.writeNull();
			}

			endMap(writer,pReference);

			if (reset)
				packet.reset(reset);

			_referencing = referencing;

			return true;

		}

		case ARRAY: {

			UInt32 size(0);
			const char* text(NULL);
			UInt32 reset(0);
			bool referencing(_referencing);
			Reference* pReference(NULL);
			UInt32 reference(0);

			if(!_amf3) {

				type = packet.read8();

				if (_referencing) {
					_amf0References.emplace_back(packet.position());
					reference = _amf0References.size();
				}
				size = packet.read32();

				if (type == AMF_MIXED_ARRAY) {
					
	
					// skip the elements
					_referencing = false;
					UInt32 position(packet.position());
					UInt32 sizeText;
					for (UInt32 i = 0; i < size; ++i) {
						if (!readText(sizeText)) { // key (string number, "0")
							// Fix beginning
							if (!i)
								position = packet.position();
							--i;
						} else
							readNext(DataWriter::Null); // value
						if (!packet.available()) {
							// Fix size!
							ERROR("AMF array bad-formed with a ", size, " of elements unfound");
							size = ++i; 
							break;
						}
					}
					_referencing = referencing;

					// AMF_MIXED_ARRAY
					pReference = beginObjectArray(writer,reference<<1,size);
		
					// write properties in first
					while ((text = readText(sizeText)) && sizeText) { // property can't be empty
						writer.writePropertyName(_buffer.assign(text,sizeText).c_str());
						if (!readNext(writer))
							writer.writeNull();
					}
					// skip end object marker
					if(packet.read8()!=AMF_END_OBJECT)
						ERROR("AMF0 end marker object absent for this mixed array");

					// finalize object part
					endObject(writer,pReference);

					// reset on elements
					reset = packet.position();
					packet.reset(position);

					while (size-- > 0) {
						if (!readText(sizeText) || !readNext(writer))
							writer.writeNull();
					}

					endArray(writer,pReference);

					packet.reset(reset);
					_referencing = referencing;
					return true;
				} 
				
				// AMF_STRICT_ARRAY
				pReference = beginArray(writer,reference<<1,size);

			} else {
	
				// AMF3
				packet.next();
				
				UInt32 reference = packet.position();
				size = packet.read7BitValue();
				bool isInline = size&0x01;
				size >>= 1;

				if(!isInline) {
					reference = ((size+1)<<1) | 0x01;
					if (writeReference(writer, reference))
						return true;
					if (size >= _references.size()) {
						ERROR("AMF3 array reference not found")
						return false;
					}
		
					reset = packet.position();
					packet.reset(_references[size]);
					size = packet.read7BitValue() >> 1;
					_referencing = false;
				} else if (_referencing) {
					_references.emplace_back(reference);
					reference = (_references.size()<<1) | 0x01;
				} else
					reference = 0;

				bool started(false);

				UInt32 sizeText(0);
				while ((text = readText(sizeText)) && sizeText) {  // property can't be empty
					if (!started) {
						pReference = beginObjectArray(writer,reference,size);
						started = true;
					}
					writer.writePropertyName(_buffer.assign(text, sizeText).c_str());
					if (!readNext(writer))
						writer.writeNull();
				}

				if (!started)
					pReference = beginArray(writer,reference, size);
				else
					endObject(writer,pReference);

			}

			while (size-- > 0) {
				if (!readNext(writer))
					writer.writeNull();
			}

			endArray(writer,pReference);

			if (reset)
				packet.reset(reset);

			_referencing = referencing;

			return true;
		}

	}

	// Object

	Reference* pReference(NULL);
	UInt32 reference(0);

	if(!_amf3) {

		///  AMF0

		type = packet.read8();
		if (_referencing) {
			_amf0References.emplace_back(packet.position());
			reference = _amf0References.size() << 1;
		}
		const char* text(NULL);
		UInt32 size(0);
		if (type == AMF_BEGIN_TYPED_OBJECT) {
			text = readText(size);
			pReference = beginObject(writer, reference, _buffer.assign(text, size).c_str());
		} else
			pReference = beginObject(writer, reference);
		
		while ((text = readText(size)) && size) {  // property can't be empty
			writer.writePropertyName(_buffer.assign(text, size).c_str());
			if (!readNext(writer))
				writer.writeNull();
		}

		if(packet.read8()!=AMF_END_OBJECT)
			ERROR("AMF0 end marker object absent");

		endObject(writer,pReference);

		return true;
	}
	
	///  AMF3
	packet.next();

	UInt32 flags = packet.read7BitValue();
	UInt32 pos(packet.position());
	UInt32 resetObject(0);
	bool isInline(flags&0x01);
	flags >>= 1;
	bool referencing(_referencing);

	if(!isInline) {
		reference = ((flags+1)<<1) | 0x01;
		if (writeReference(writer, reference))
			return true;
		if (flags >= _references.size()) {
			ERROR("AMF3 object reference not found")
			return false;
		}
		
		resetObject = packet.position();
		packet.reset(_references[flags]);
		flags = packet.read7BitValue() >> 1;
		_referencing = false;
	} else if (_referencing) {
		_references.emplace_back(pos);
		reference = (_references.size()<<1) | 0x01;
	} else
		reference = 0;

	// classdef reading
	isInline = flags&0x01; 
	flags >>= 1;
	UInt32 reset(0);
	UInt32 size(0);
	const char* text(NULL);
	if(isInline) {
		 _classDefReferences.emplace_back(pos);
		text = readText(size); // type
	} else if(flags<_classDefReferences.size()) {
		reset = packet.position();
		packet.reset(_classDefReferences[flags]);
		flags = packet.read7BitValue()>>2;
		text = readText(size);
		_referencing = false;
	} else
		ERROR("AMF3 classDef reference not found")

	if (flags & 0x01) {
		// external, support just "flex.messaging.io.ArrayCollection"
		if (reset)
			packet.reset(reset);
		bool result(readNext(writer));
		if (resetObject)
			packet.reset(resetObject); // reset object
		_referencing = referencing;
		return result;
	}

	if (text)
		pReference = beginObject(writer, reference, _buffer.assign(text, size).c_str());
	else
		pReference = beginObject(writer, reference);

	isInline = (flags&0x02) ? true : false; // is dynamic!

	flags>>=2;

	 // Find values position for classdef name properties
	pos = packet.position(); // save name position
	for (UInt32 i = 0; i < flags;++i) {
		if (!readText(size)) {
			// fix flags if error during properties reading
			if (!i)
				pos = packet.position();
			--flags; 
		}
	}
	// Reset classdef to be on values now!
	if (reset) {
		packet.reset(reset);
		reset = 0;
		if (!resetObject)
			_referencing = referencing;
	}

	// Read classdef properties
	while (flags--) {
		reset = packet.position(); // save value position
		// Read property name in classdef
		packet.reset(pos); // reset on name
		while(!(text = readText(size)));
		writer.writePropertyName(_buffer.assign(text, size).c_str());
		pos = packet.position(); // save new name position
		// Read value
		packet.reset(reset); // reset on value
		if (!readNext(writer))
			writer.writeNull();
	}

	
	if (isInline) { // is dynamic
		while ((text = readText(size)) && size) { // property can't be empty
			writer.writePropertyName(_buffer.assign(text, size).c_str());
			if (!readNext(writer))
				writer.writeNull();
		}
	}

	endObject(writer,pReference);

	if (resetObject)
		packet.reset(resetObject); // reset object
	_referencing = referencing;

	return true;
}



} // namespace Mona
