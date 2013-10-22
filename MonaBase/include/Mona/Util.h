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
#include "Mona/Exceptions.h"
#include "Mona/MapParameters.h"
#include "Poco/NullStream.h"
#include "Poco/Net/SocketAddress.h"
#include <cstring>
#include <vector>
#include <map>


namespace Mona {

class Util {
public:
	struct AddressComparator {
	   bool operator()(const Poco::Net::SocketAddress& a,const Poco::Net::SocketAddress& b) const {
		   return a.port()<b.port();
	   }
	};

	static bool		   SameAddress(const Poco::Net::SocketAddress& address1,const Poco::Net::SocketAddress& address2);
	static std::string FormatHex(const Mona::UInt8* data,Mona::UInt32 size);
	static std::string FormatHex2(const Mona::UInt8* data,Mona::UInt32 size);
	static Mona::UInt8 Get7BitValueSize(Mona::UInt32 value);
	static Mona::UInt8 Get7BitValueSize(Mona::UInt64 value);

	static void Dump(const Mona::UInt8* in, Mona::UInt32 size,std::vector<Mona::UInt8>& out,const char* header=NULL);

	static void UnpackUrl(const std::string& url, std::string& path, MapParameters& properties);
	static void UnpackUrl(const std::string& url, std::string& path, std::string& file, MapParameters& properties);
	static void UnpackUrl(const std::string& url, Poco::Net::SocketAddress& address, std::string& path, MapParameters& properties);
	static void UnpackUrl(const std::string& url, Poco::Net::SocketAddress& address, std::string& path, std::string& file, MapParameters& properties);
	
	static void UnpackQuery(const std::string& query, MapParameters& properties);

	static void ReadIniFile(Exception& ex, const std::string& path, MapParameters& parameters);

	static Poco::NullInputStream	NullInputStream;
	static Poco::NullOutputStream	NullOutputStream;
};

inline Mona::UInt8 Util::Get7BitValueSize(Mona::UInt32 value) {
	return Get7BitValueSize((Mona::UInt64)value);
}

inline bool Util::SameAddress(const Poco::Net::SocketAddress& address1,const Poco::Net::SocketAddress& address2) {
	return std::memcmp(address1.addr(),address2.addr(),address1.length())==0 && address1.port() == address2.port();
}

} // namespace Mona
