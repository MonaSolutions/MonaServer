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
#include "Mona/XMLWriter.h"

namespace Mona {


class SOAPWriter : public XMLWriter, public virtual NullableObject {
public:
	SOAPWriter(const PoolBuffers& buffers);

	virtual void beginObject(const std::string& type="",bool external=false);
	virtual void endObject();

	virtual void writePropertyName(const std::string& value);

	virtual void beginArray(UInt32 size);

	virtual void writeDate(const Time& date);
	virtual void writeNumber(double value);
	virtual void writeString(const std::string& value) { writeRaw(value); }
	virtual void writeBoolean(bool value) { writeRaw(value? "true" : "false"); }
	virtual void writeNull() { writeRaw("null"); }
	virtual void writeBytes(const UInt8* data, UInt32 size);

protected:
	virtual void writeRaw(const char* value);
	virtual void writeRaw(const std::string& value);

private:

	bool _first;

	/// \brief Start the soap structure
	void writeHeader();

	/// \brief Close the soap structure
	void writeFooter();
};



} // namespace Mona
