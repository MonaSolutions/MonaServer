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
#include "math.h"
#include <limits>
#include <mutex>
#include <thread>

namespace Mona {


class Util : virtual Static {
public:
	
	static UInt8 Get7BitValueSize(UInt32 value) { return Get7BitValueSize((UInt64)value); }
	static UInt8 Get7BitValueSize(UInt64 value);

	static const std::string&	CurrentThreadInfos(THREAD_ID& id);
	static void					SetCurrentThreadName(const std::string& name);

	static void Dump(const UInt8* in, UInt32 size, Buffer& out) { std::string header; Dump(in, size, out, header); }
	static void Dump(const UInt8* in, UInt32 size, Buffer& out, const std::string& header);

	/// \brief Unpack url in path and query
	/// \param url Url to unpack
	/// \param path Part of the url between host address and '?'
	/// \param query Part of the url after '?' (if present)
	/// \return The position of the file in path (std::npos for a directory)
	static std::size_t UnpackUrl(const std::string& url, std::string& path, std::string& query) {std::string address; return UnpackUrl(url, address, path, query);}
	static std::size_t UnpackUrl(const std::string& url, std::string& address, std::string& path, std::string& query);
	
	static Parameters& UnpackQuery(const std::string& query, Parameters& properties) { return UnpackQuery(query.data(), properties); }
	static Parameters& UnpackQuery(const char* query, Parameters& properties);

	static char DecodeURI(const char* value) { if (!value) return '%';  DecodeURI(value, value += strlen(value)); }
	static char DecodeURI(const char* begin,const char* end);

	static bool ReadIniFile(Exception& ex, const std::string& path, Parameters& parameters);

	static unsigned ProcessorCount() { unsigned result(std::thread::hardware_concurrency());  return result > 0 ? result : 1; }
	static const Parameters& Environment();

	template<typename Type>
	static Type Random() {
		static UInt32 x = (UInt32)Time::Now();
		static UInt32 y = 362436069, z = 521288629, w = 88675123;
		UInt32 t = x ^ (x << 11);
		x = y; y = z; z = w;
		return (w = w ^ (w >> 19) ^ (t ^ (t >> 8))) % std::numeric_limits<Type>::max();
	}
	static void Random(UInt8* data, UInt32 size) {for (UInt32 i = 0; i < size; ++i) data[i] = Random<UInt8>();}


	template <typename BufferType>
	static BufferType& ToBase64(const UInt8* data, UInt32 size, BufferType& buffer) {
		UInt32 i(0),j(0),accumulator(0),bits(0);
		buffer.resize((UInt32)ceil(size/3.0)*4);

		for (i = 0; i < size;++i) {
			accumulator = (accumulator << 8) | (data[i] & 0xFFu);
			bits += 8;
			while (bits >= 6) {
				bits -= 6;
				buffer[j++] = _B64Table[(accumulator >> bits) & 0x3Fu];
			}
		}
		if (bits > 0) { // Any trailing bits that are missing.
			accumulator <<= 6 - bits;
			buffer[j++] = _B64Table[accumulator & 0x3Fu];
		}
		while (buffer.size() > j) // padding with '='
			buffer[j++] = '=';
		return buffer;
	}


	template <typename BufferType>
	static bool FromBase64(BufferType& buffer) {
		UInt32 bits(0),j(0),accumulator(0),size(buffer.size());

		for (UInt32 i = 0; i < size; ++i) {
			const int c = buffer[i];
			if (isspace(c) || c == '=')
				continue;

			if ((c > 127) || (c < 0) || (_ReverseB64Table[c] > 63))
				return false;
		
			accumulator = (accumulator << 6) | _ReverseB64Table[c];
			bits += 6;
			if (bits >= 8) {
				bits -= 8;
				buffer[j++] = ((accumulator >> bits) & 0xFFu);
			}
		}
		buffer.resize(j);
		return true;
	}

	enum HexOption {
		HEX_CPP=1,
		HEX_TRIM_LEFT=2,
		HEX_APPEND=4
	};

	template <typename BufferType>
	static BufferType&	FormatHex(const UInt8* data, UInt32 size, BufferType& buffer, UInt8 options=0) {

		UInt32 i(0), j(options&HEX_APPEND ? buffer.size() : 0);
		bool skipLeft(false);
		if (options&HEX_TRIM_LEFT) {
			for (i; i < size; ++i) {
				if ((data[i] >> 4)>0)
					break;
				if ((data[i] & 0x0F) > 0) {
					skipLeft = true;
					break;
				}
			}
		}

		buffer.resize((size-i) * ((options&HEX_CPP) ? 4 : 2) - (skipLeft ? 1 : 0) + j);

		UInt8 value;
		for (i; i < size; ++i) {
			if (options&HEX_CPP) {
				buffer[j++] = '\\';
				buffer[j++] = 'x';
			}
			value = data[i] >> 4;
			if (!skipLeft)
				buffer[j++] = (value>9 ? (value + '7') : '0' + value);
			else
				skipLeft = false;
			value = data[i] & 0x0F;
			buffer[j++] = (value > 9 ? (value + '7') : '0' + value);
		}
		return buffer;
	}

	template <typename BufferType>
	static BufferType& UnformatHex(BufferType& buffer) {
		UInt32 j(0),i(0),size(buffer.size());
		while(j<size) {
			UInt8 first = buffer[j++];
			UInt8 second = j == size ? '0' : buffer[j++];
			buffer[i++] = ((first - (first<='9' ? '0' : '7')) << 4) | ((second - (second<='9' ? '0' : '7')) & 0x0F);
		}
		buffer.resize(i);
		return buffer;
	}


private:

	static MapParameters					_Environment;
	static std::mutex						_MutexEnvironment;
	
	static std::map<THREAD_ID, std::string>	_ThreadNames;
	static std::recursive_mutex				_MutexThreadNames;

	static const char						_B64Table[65];
	static const char						_ReverseB64Table[128];
};


} // namespace Mona
