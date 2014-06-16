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
#include "Mona/Event.h"


namespace Mona {

namespace Events {
	struct OnChange : Event<void(const std::string&,const char*)> {};
	struct OnClear : Event<void()> {};
};

class Parameters : public virtual Object,
	public Events::OnChange,
	public Events::OnClear {
public:

	/*! Return false if key doesn't exist (and don't change 'value'), otherwise return true and assign string 'value' */
	bool getString(const std::string& key, std::string& value) const;

	/*! Return false if key doesn't exist or if it's not a numeric type, otherwise return true and assign numeric 'value' */
	template<typename NumberType>
	bool getNumber(const std::string& key, NumberType& value) const {
		const  std::string* pTemp = getRaw(key);
		if (!pTemp)
			return false;
		return String::ToNumber<NumberType>(*pTemp, value);
	}

	/*! Return false if key doesn't exist or if it's not a boolean type, otherwise return true and assign boolean 'value' */
	bool getBool(const std::string& key, bool& value) const;

	/*! A short version of getNumber with template default argument to get value as returned result */
	template<typename NumberType,NumberType defaultValue=0>
	NumberType getNumber(const std::string& key) const {
		NumberType result(defaultValue);
		getNumber(key,result);
		return result;
	}

	/*! A short version of getBool with template default argument to get value as returned result */
	template<bool defaultValue=false>
	bool getBool(const std::string& key) const {
		bool result(defaultValue);
		getBool(key,result);
		return result;
	}



	bool hasKey(const std::string& key) { return getRaw(key) != NULL; }
	void erase(const std::string& key) { setIntern(key, NULL); }

	void setString(const std::string& key, const std::string& value) {setIntern(key,value.c_str());}
	void setString(const std::string& key, const char* value) {setIntern(key, value ? value : "");}
	template<typename NumberType>
	void setNumber(const std::string& key, NumberType value) {
		std::string val;
		setString(key, String::Format(val, value));
	}
	void setBool(const std::string& key, bool value) { setIntern(key, value ? "true" : "false"); }

	typedef std::function<void(const std::string&, const std::string&)> ForEach;

	UInt32  iterate(ForEach& function) const { return iteration(NULL, function); };
	UInt32  iterate(const std::string& prefix, ForEach& function) const { return iteration(prefix.c_str(), function); };
	UInt32  iterate(const char* prefix, ForEach& function) const { return iteration(prefix, function);};
	
	
	void clear() { clearAll(); OnClear::raise(); }


	virtual UInt32 count() const = 0;

protected:
	Parameters() {}

private:
	
	void setIntern(const std::string& key, const char* value) { if (setRaw(key, value)) OnChange::raise(key,value); }

	virtual UInt32 iteration(const char* prefix,ForEach& function) const = 0;

	virtual void clearAll() = 0;
	virtual const std::string* getRaw(const std::string& key) const = 0;
	// if value==NULL the property should be removed, return true if something has changed
	virtual bool setRaw(const std::string& key, const char* value) = 0;
};



} // namespace Mona

