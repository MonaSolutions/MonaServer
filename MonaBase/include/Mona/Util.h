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
*/

#pragma once

#include "Mona/Mona.h"
#include "Mona/MapParameters.h"
#include "Mona/SocketAddress.h"
#include "Mona/Buffer.h"
#include "Mona/Exceptions.h"
#include <vector>
#include <map>
#include <thread>


namespace Mona {

class Util : virtual Static {
public:
	
	static UInt8 Get7BitValueSize(UInt32 value) { return Get7BitValueSize((UInt64)value); }
	static UInt8 Get7BitValueSize(UInt64 value);

	static const std::string&	GetThreadName(std::thread::id id);
	static void					SetThreadName(std::thread::id id,const std::string& name);

	static void Dump(const UInt8* in, UInt32 size, Buffer<UInt8>& out) { std::string header; Dump(in, size, out, header); }
	static void Dump(const UInt8* in, UInt32 size, Buffer<UInt8>& out, const std::string& header);

	static bool UnpackUrl(Exception& ex, const std::string& url, std::string& path, MapParameters& properties);
	static bool UnpackUrl(Exception& ex, const std::string& url, std::string& path, std::string& file, MapParameters& properties);
	static bool UnpackUrl(Exception& ex, const std::string& url, SocketAddress& address, std::string& path, MapParameters& properties);
	static bool UnpackUrl(Exception& ex, const std::string& url, SocketAddress& address, std::string& path, std::string& file, MapParameters& properties);
	
	static void UnpackQuery(const std::string& query, MapParameters& properties);

	static std::string& DecodeURI(std::string& uri);

	static UInt8*			UnformatHex(UInt8* data,UInt32& size);
	static std::string&		FormatHex(const UInt8* data, UInt32 size, std::string& result);
	static std::string&		FormatHexCpp(const UInt8* data, UInt32 size, std::string& result);
	static bool				FromBase64(const UInt8* data, UInt32 size, Buffer<UInt8>& result);
	static Buffer<UInt8>&	ToBase64(const UInt8* data, UInt32 size, Buffer<UInt8>& result);
	

	static bool ReadIniFile(Exception& ex, const std::string& path, MapParameters& parameters);

	static unsigned ProcessorCount() { unsigned result(std::thread::hardware_concurrency());  return result > 0 ? result : 1; }
	static const MapParameters& Environment();

	static void Random(UInt8* data, UInt32 size);
private:
	static MapParameters	_Environment;
	static std::mutex		_MutexEnvironment;
	
	static std::map<std::thread::id, std::string>	_ThreadNames;
	static std::mutex								_MutexThreadNames;
};


} // namespace Mona
