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
#include "Mona/String.h"

namespace Mona {

class Parameters : public virtual Object {
public:

	bool getString(const std::string& key, std::string& value) const;
	template<typename NumberType>
	bool getNumber(const std::string& key, NumberType& value) const {
		const  std::string* pTemp = getRaw(key);
		if (!pTemp)
			return false;
		return String::ToNumber<NumberType>(*pTemp, value);
	}
	bool getBool(const std::string& key, bool& value) const;

	bool hasKey(const std::string& key) { return getRaw(key) != NULL; }
	void erase(const std::string& key) { setRaw(key, NULL); }

	void setString(const std::string& key, const std::string& value) {setRaw(key, &value[0]);}
	void setString(const std::string& key, const char* value) {setRaw(key, value);}
	template<typename NumberType>
	void setNumber(const std::string& key, NumberType value) {
		std::string val;
		setString(key, String::Format(val, value));
	}
	void setBool(const std::string& key, bool value) {setRaw(key, value ? "true" : "false");}

protected:
	Parameters() {}

private:
	virtual const std::string* getRaw(const std::string& key) const = 0;
	// if value==NULL the property should be removed
	virtual void setRaw(const std::string& key, const char* value) = 0;
};



} // namespace Mona

