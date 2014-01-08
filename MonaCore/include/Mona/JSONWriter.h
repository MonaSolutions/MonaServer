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
#include "Mona/DataWriter.h"

namespace Mona {


class JSONWriter : public DataWriter, virtual Object {
public:
	JSONWriter(const PoolBuffers& buffers,bool modeRaw=false);

	void beginObject(const std::string& type="",bool external=false);
	void endObject();

	void writePropertyName(const std::string& value);

	void beginArray(UInt32 size);
	void endArray();

	void writeDate(const Time& date) { writeRaw(date.toString(Time::ISO8601_FRAC_FORMAT, _buffer)); }
	void writeNumber(double value) { writeRaw(String::Format(_buffer, value)); }
	void writeString(const std::string& value);
	void writeBoolean(bool value) { writeRaw( value ? "true" : "false"); }
	void writeNull() { writeRaw("null"); }
	void writeBytes(const UInt8* data,UInt32 size);

	void	end();
	void	clear();
private:

	template <typename ...Args>
	void writeRaw(Args&&... args) {
		if(!_started) {
			_started=true;
			packet.write8('[');
		}
		if(!_first)
			packet.write8(',');
		_first=false;
		packet.writeRaw(args ...);
	}

	bool		_modeRaw;
	bool		_started;
	bool		_first;
	UInt32		_layers;
	std::string _buffer;
};



} // namespace Mona
