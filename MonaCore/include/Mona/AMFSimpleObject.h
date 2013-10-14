/* 
	Copyright 2013 OpenRTMFP
 
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License received along this program for more
	details (or else see http://www.gnu.org/licenses/).

	This file is a part of Cumulus.
*/

#pragma once

#include "Cumulus/Cumulus.h"
#include "Cumulus/AMF.h"
#include "Poco/Util/MapConfiguration.h"
#include "Poco/Timestamp.h"

namespace Cumulus {


class AMFSimpleObject : public Poco::Util::MapConfiguration {
public:
	AMFSimpleObject();
	virtual ~AMFSimpleObject();

	int				getInteger(const std::string& key, int defaultValue);
	double			getNumber(const std::string& key, double defaultValue);
	Poco::Timestamp	getDate(const std::string& key, Poco::Timestamp& defaultDate);
	bool			getBoolean(const std::string& key, bool defaultValue);

	void setString(const std::string& key, const std::string& value);
	void setInteger(const std::string& key, int value);
	void setNumber(const std::string& key, double value);
	void setDate(const std::string& key, Poco::Timestamp& date);
	void setBoolean(const std::string& key, bool value);
	void setNull(const std::string& key);

	bool has(const std::string& key) const;

};

inline bool AMFSimpleObject::has(const std::string& key) const {
	return hasProperty(key);
}

inline int	AMFSimpleObject::getInteger(const std::string& key, int defaultValue) {
	return getInt(key,defaultValue);
}

inline double AMFSimpleObject::getNumber(const std::string& key, double defaultValue) {
	return getDouble(key,defaultValue);
}

inline Poco::Timestamp AMFSimpleObject::getDate(const std::string& key, Poco::Timestamp& defaultDate) {
	return Poco::Timestamp((Poco::Timestamp::TimeVal)getDouble(key,(double)(defaultDate.epochMicroseconds()/1000))*1000);
}

inline bool AMFSimpleObject::getBoolean(const std::string& key, bool defaultValue) {
	return getBool(key,defaultValue);
}


} // namespace Cumulus
