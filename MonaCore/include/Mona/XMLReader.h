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
#include "Mona/DataReader.h"
#include "Mona/Time.h"


namespace Mona {

class XMLReader : public DataReader, virtual Object {
public:
	XMLReader(PacketReader& packet);

	virtual std::string&		readString(std::string& value);
	virtual double				readNumber();
	virtual bool				readBoolean();
	virtual Time&				readTime(Time& time);
	virtual void				readNull() {}
	
	bool						readArray(UInt32& size);
	virtual bool				readObject(std::string& type,bool& external);
	virtual Type				readItem(std::string& name);
	virtual Type				followingType();

	virtual void				reset();

	/// \brief Return true if content is valid
	virtual bool				isValid();

protected:

	/// \brief Intern structure for ordering tags and subtags
	/// and return the good type in followingType()
	class TagXML : virtual Object {
	public:
		TagXML(const std::string& name) : tagName(name), arrayStarted(false) {}
		virtual ~TagXML() {}

		bool		arrayStarted;
		std::string tagName;
		std::string currentSubTag;
	};
	
    std::deque<TagXML>		_queueTags; ///< record each tags and the is last subtag	
	UInt32					_pos;

private:
	const UInt8*	readBytes(UInt32& size);

	/**
	 * \brief ignore comment and return next type
	 * \param first current character
	 */
	Type			parseComment(const UInt8* first);

	/**
	 * \brief add a tag added and return type
	 * \param cur current character
	 * \param evaluateArray if true evaluate if array need to be created
	 */
	Type			addTag(const UInt8* cur, bool evaluateArray = false);
	bool			nextTagIsSame(const std::string& name);

	/// \brief remove last tag added and return type
	Type			removeTag();
	
	/// \brief parse a primitive value : string (with/without quotes), data, number
	/// and return type
	Type			parsePrimitive(const UInt8* cur);

	/// \brief End an eventualy opened array of subtag
	/// \return True if subtag ended
	bool			endSubtagArray();

	/// \brief ignore spaces and get current char
	const UInt8*	current();

	enum ReadStep {
		NOTHING=0,
		POP_TAG,
		ADD_TAG,
		START_ARRAY_TAG
	};

	ReadStep			_nextStep; //! if there is an action to terminate
	std::string		    _tagName;
	std::string		    _value;
	Time			    _date;
	Type			    _last;
	double			    _dval;
	bool                _tagOpened; //! Tag is opened, we expect attributes

	Type				_nextType; //! if next type has already been readed
	bool				_isProperty; //! current is a property : need to write property name
};


} // namespace Mona
