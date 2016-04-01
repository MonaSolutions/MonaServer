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
	struct OnChange : Event<void(const char* key, const char* value, size_t size)> {};
	struct OnClear : Event<void()> {};
};

class Parameters : public virtual Object,
	public Events::OnChange,
	public Events::OnClear {
public:

	/*! Return false if key doesn't exist (and don't change 'value'), otherwise return true and assign string 'value' */
	bool getString(const std::string& key, std::string& value) const { return getString(key.c_str(), value); }
	bool getString(const char* key, std::string& value) const;

	/*! Return false if key doesn't exist or if it's not a numeric type, otherwise return true and assign numeric 'value' */
	template<typename NumberType>
	bool getNumber(const std::string& key, NumberType& value) const { return getNumber<NumberType>(key.c_str(), value); }
	template<typename NumberType>
	bool getNumber(const char* key, NumberType& value) const {
		const char* temp = getRaw(key);
		if (!temp)
			return false;
		return String::ToNumber<NumberType>(temp, value);
	}

	/*! Return false if key doesn't exist or if it's not a boolean type, otherwise return true and assign boolean 'value' */
	bool getBoolean(const std::string& key, bool& value) const { return getBoolean(key.c_str(), value); }
	bool getBoolean(const char* key, bool& value) const;


	/*! A short version of getNumber with template default argument to get value as returned result */
	template<typename NumberType=double,int defaultValue=0>
	NumberType getNumber(const std::string& key) const { return getNumber<NumberType,defaultValue>(key.c_str()); }
	template<typename NumberType,int defaultValue=0>
	NumberType getNumber(const char* key) const {
		NumberType result((NumberType)defaultValue);
		getNumber(key,result);
		return result;
	}

	/*! A short version of getBoolean with template default argument to get value as returned result */
	template<bool defaultValue=false>
	bool getBoolean(const std::string& key) const { return getBoolean<defaultValue>(key.c_str()); }
	template<bool defaultValue=false>
	bool getBoolean(const char* key) const {
		bool result(defaultValue);
		getBoolean(key,result);
		return result;
	}


	bool hasKey(const std::string& key) { return getRaw(key.c_str()) != NULL; }
	bool hasKey(const char* key) { return getRaw(key) != NULL; }

	void erase(const std::string& key) { setIntern(key.c_str(),NULL); }
	void erase(const char* key) { setIntern(key,NULL); }

	void setString(const std::string& key, const std::string& value) {setIntern(key.c_str(),value.c_str(),value.size());}
	void setString(const char* key, const std::string& value) { setIntern(key,value.c_str(),value.size());}
	void setString(const std::string& key, const char* value, std::size_t size = std::string::npos) { setIntern(key.c_str(), value ? value : String::Empty.c_str(),size); }
	void setString(const char* key, const char* value, std::size_t size = std::string::npos) { setIntern(key, value ? value : String::Empty.c_str(),size);}

	template<typename NumberType>
	void setNumber(const std::string& key, NumberType value) { setNumber<NumberType>(key.c_str(), value); }
	template<typename NumberType>
	void setNumber(const char* key, NumberType value) {
		std::string val;
		setString(key, String::Format(val, value));
	}

	void setBoolean(const std::string& key, bool value) { setIntern(key.c_str(), value ? "true" : "false"); }
	void setBoolean(const char* key, bool value) { setIntern(key, value ? "true" : "false"); }

	typedef std::function<void(const std::string&, const std::string&)> ForEach;

	UInt32  iterate(const ForEach& function) const { return iteration(NULL, function); };
	UInt32  iterate(const std::string& prefix, const ForEach& function) const { return iteration(prefix.c_str(), function); };
	UInt32  iterate(const char* prefix, const ForEach& function) const { return iteration(prefix, function);};
	
	
	void clear() { clearAll(); _bytes = 0; OnClear::raise(); }

	bool empty() const { return count() == 0; }
	virtual UInt32 count() const = 0;
	UInt32			bytes() const { return _bytes; };

	virtual const char* getRaw(const char* key) const = 0;
	// if value==NULL the property should be removed, return true if something has changed
	virtual UInt32 setRaw(const char* key, const char* value, UInt32 size) = 0;

protected:
	Parameters() : _bytes(0) {}

	 // value==NULL means "deletion"
	virtual void onChange(const char* key, const char* value, std::size_t size) { OnChange::raise(key,value, size); }


private:
	void setIntern(const char* key, const char* value, std::size_t size = std::string::npos);

	virtual UInt32 iteration(const char* prefix,const ForEach& function) const = 0;

	virtual void clearAll() = 0;


	UInt32	_bytes;
};



} // namespace Mona

