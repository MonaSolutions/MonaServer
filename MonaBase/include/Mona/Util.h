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
#include "Mona/MapParameters.h"
#include "Mona/Time.h"
#include "Mona/Exceptions.h"
#include "Mona/Buffer.h"
#include <map>
#include <thread>
#include <limits>
#include <mutex>

namespace Mona {

class Util : virtual Static {
public:
	enum HexOption {
		HEX_CPP=1,
		HEX_TRIM_LEFT=2
	};
	
	static UInt8 Get7BitValueSize(UInt32 value) { return Get7BitValueSize((UInt64)value); }
	static UInt8 Get7BitValueSize(UInt64 value);

	static const std::string&	GetThreadName(std::thread::id id);
	static void					SetThreadName(std::thread::id id,const std::string& name);

	static void Dump(const UInt8* in, UInt32 size, Buffer& out) { std::string header; Dump(in, size, out, header); }
	static void Dump(const UInt8* in, UInt32 size, Buffer& out, const std::string& header);

	static std::size_t UnpackUrl(const std::string& url, std::string& path, std::string& query) {std::string address; return UnpackUrl(url, address, path, query);}
	static std::size_t UnpackUrl(const std::string& url, std::string& address, std::string& path, std::string& query);
	
	static Parameters& UnpackQuery(const std::string& query, Parameters& properties);

	static UInt8*		UnformatHex(UInt8* data,UInt32& size);
	static std::string&	FormatHex(const UInt8* data, UInt32 size, std::string& result, UInt8 options=0) { result.clear(); return AppendHex(data, size, result, options); }
	static std::string&	AppendHex(const UInt8* data, UInt32 size, std::string& result,UInt8 options=0);
	static bool			FromBase64(const UInt8* data, UInt32 size, Buffer& result);
	static Buffer&		ToBase64(const UInt8* data, UInt32 size, Buffer& result);
	

	static bool ReadIniFile(Exception& ex, const std::string& path, Parameters& parameters);

	static unsigned ProcessorCount() { unsigned result(std::thread::hardware_concurrency());  return result > 0 ? result : 1; }
	static const MapParameters& Environment();

	template<typename Type>
	static Type Random() {
		static UInt32 x = (UInt32)Time();
		static UInt32 y = 362436069, z = 521288629, w = 88675123;
		UInt32 t = x ^ (x << 11);
		x = y; y = z; z = w;
		return (w = w ^ (w >> 19) ^ (t ^ (t >> 8))) % std::numeric_limits<Type>::max();
	}
	static void Random(UInt8* data, UInt32 size) {for (UInt32 i = 0; i < size; ++i) data[i] = Random<UInt8>();}
private:
	static char DecodeURI(const std::string::const_iterator& it,const std::string::const_iterator& end);

	static MapParameters	_Environment;
	static std::mutex		_MutexEnvironment;
	
	static std::map<std::thread::id, std::string>	_ThreadNames;
	static std::mutex								_MutexThreadNames;
};


} // namespace Mona
