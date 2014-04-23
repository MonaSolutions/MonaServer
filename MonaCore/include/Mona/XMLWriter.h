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

	
class XMLWriter : public DataWriter, public virtual Object {
public:
	XMLWriter(const PoolBuffers& buffers);

	virtual void beginObject(const std::string& type="",bool external=false);

	/** Brief Function called at end of :
	 - an object,
	 - a list of attributes of a mixed array
	*/
	virtual void endObject();

	virtual void writePropertyName(const std::string& value);

	virtual void beginArray(UInt32 size);
	virtual void endArray();

	virtual void writeDate(const Date& date) { writeRaw(date.toString(Date::ISO8601_FRAC_FORMAT, _buffer)); }
	virtual void writeNumber(double value) { writeRaw(String::Format(_buffer, value)); }
	virtual void writeString(const std::string& value) { writeRaw(value); }
	virtual void writeBoolean(bool value) { writeRaw(value? "true" : "false"); }
	virtual void writeNull() { writeRaw("null"); }
	virtual void writeBytes(const UInt8* data, UInt32 size);

	virtual void	clear();

	virtual void	endWrite();

	/** 
	 * \brief Function called for writing first tag
	 * \param doubleArray if double encapsulation is needed
	 */
	virtual void	beginDocument(bool doubleArray = false);

protected:
	virtual void	writeRaw(const char* value);
	virtual void	writeRaw(const std::string& value);

	/*! \brief Write the right tag before writing primitive value
	*/
	void	begin(bool empty = false);

	/*! \brief Write the right tag after writing primitive value
	*/
	void	end();

	/*! \brief Function to call before adding a child to a tag
	* It close the tag if needed and set the flag 'childs' to true
	*/
	void	setTagHasChilds();

	/*! \brief Intern class for managing tags
	*	while writing
	*/
	class TagPos : public virtual Object {
	public:
		TagPos(const std::string& val) : name(val), childs(false), arrayLevel(1), empty(false) {}
		std::string name;
		bool		childs;			//! This tag has childs? Used to determine the closure mode
		bool		empty;			//! Empty tag => no end tag
		UInt8		arrayLevel;
	};

	std::deque<TagPos>	_queueTags; //! queue of tags ordered by 

private:	

	std::string		_tagName;	// last tag name readed 
	std::string		_buffer;	// buffer for raw conversion

	bool	_started; // writing started
	bool	_close2Times; // for an array of objects => close 2 times
};



} // namespace Mona
