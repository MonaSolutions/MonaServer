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
#include "Mona/String.h"

namespace Mona {

class Parameters {
public:

	bool getString(const std::string& key, std::string& value) const {return getRaw(key, value);}
	bool getNumber(const std::string& key, double& value) const;
	bool getNumber(const std::string& key, int& value) const;
	bool getBool(const std::string& key, bool& value) const;

	void setString(const std::string& key, const std::string& value) {setRaw(key, value);}
	void setNumber(const std::string& key, double value) { std::string val; setRaw(key, String::Format(val, value)); }
	void setNumber(const std::string& key, int value) { std::string val; setRaw(key, String::Format(val, value)); }
	void setBool(const std::string& key, bool value) {setRaw(key, value ? "true" : "false");}

protected:
	Parameters() {}
	virtual ~Parameters() {}

private:
	virtual bool getRaw(const std::string& key, std::string& value) const = 0;
	virtual void setRaw(const std::string& key, const std::string& value) = 0;
};

template<class IteratorType>
class IterableParameters : public Parameters {
public:
	typedef IteratorType Iterator;

	virtual Iterator begin() const = 0;
	virtual Iterator end() const = 0;
protected:
	IterableParameters() {}
	virtual ~IterableParameters() {}
};



} // namespace Mona

