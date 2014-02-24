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
#include "Mona/DataWriter.h"

namespace Mona {

	
class XMLWriter : public DataWriter, virtual Object {
public:
	XMLWriter(const PoolBuffers& buffers);

	virtual void beginObject(const std::string& type="",bool external=false);
	virtual void endObject();

	virtual void writePropertyName(const std::string& value);

	virtual void beginArray(UInt32 size);
	virtual void endArray() {}

	virtual void writeDate(const Date& date) { writeRaw(date.toString(Date::ISO8601_FRAC_FORMAT, _buffer)); }
	virtual void writeNumber(double value) { writeRaw(String::Format(_buffer, value)); }
	virtual void writeString(const std::string& value) { writeRaw(value); }
	virtual void writeBoolean(bool value) { writeRaw(value? "true" : "false"); }
	virtual void writeNull() { writeRaw("null"); }
	virtual void writeBytes(const UInt8* data, UInt32 size);

	virtual void	clear();

	virtual void	endWrite() { packet.writeRaw("</root>"); }

	/// \brief Function called for writing <root> tag
	virtual void	beginDocument() { packet.writeRaw("<root>"); }

protected:
	virtual void	writeRaw(const char* value);
	virtual void	writeRaw(const std::string& value);

	/// \brief write right tag before writing primitive value
	void	begin();

	/// \brief write right tag after writing primitive value
	void	end();

	void	setTagHasChilds();

	/// \brief Intern class for managing tags
	/// while writing
	class TagPos : virtual Object {
	public:
		TagPos(const std::string& val) : name(val), counter(1), childs(false) {}
		std::string name;
		UInt8		counter;
		bool		childs;
	};

	std::deque<TagPos>	_queueTags; ///< queue of tags ordered by 

private:	

	std::string		_value; ///< last tag name readed 
	std::string		_buffer; ///< buffer string for writing raw

	bool _started;
};



} // namespace Mona
