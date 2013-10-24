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
#include "Poco/NullStream.h"
#include "Mona/Exceptions.h"
#include <cstring>
#include <vector>



namespace Mona {

class Util : virtual Static {
public:
	static std::string FormatHex(const UInt8* data,UInt32 size);
	static std::string FormatHex2(const UInt8* data,UInt32 size);
	static UInt8 Get7BitValueSize(UInt32 value) { return Get7BitValueSize((UInt64)value); }
	static UInt8 Get7BitValueSize(UInt64 value);

	static void Dump(const UInt8* in, UInt32 size, std::vector<UInt8>& out) { std::string header; Dump(in,size,out,header); }
	static void Dump(const UInt8* in, UInt32 size,std::vector<UInt8>& out,const std::string& header);

	static bool UnpackUrl(Exception& ex, const std::string& url, std::string& path, MapParameters& properties);
	static bool UnpackUrl(Exception& ex, const std::string& url, std::string& path, std::string& file, MapParameters& properties);
	static bool UnpackUrl(Exception& ex, const std::string& url, SocketAddress& address, std::string& path, MapParameters& properties);
	static bool UnpackUrl(Exception& ex, const std::string& url, SocketAddress& address, std::string& path, std::string& file, MapParameters& properties);
	
	static void UnpackQuery(const std::string& query, MapParameters& properties);

	static bool ReadIniFile(Exception& ex, const std::string& path, MapParameters& parameters);

	static unsigned ProcessorCount();

	static Poco::NullInputStream	NullInputStream;
	static Poco::NullOutputStream	NullOutputStream;
};


} // namespace Mona
