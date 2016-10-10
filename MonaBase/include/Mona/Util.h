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
#include "Mona/Process.h"
#include "math.h"
#include <limits>
#include <mutex>
#include <thread>

namespace Mona {


class Util : virtual Static {
public:
	
	static UInt8 Get7BitValueSize(UInt32 value) { return Get7BitValueSize((UInt64)value); }
	static UInt8 Get7BitValueSize(UInt64 value);

	static THREAD_ID	CurrentThreadId();
	static void			SetCurrentThreadName(const char* name);
	template <typename BufferType>
	static BufferType&	GetThreadName(THREAD_ID id, BufferType& buffer, bool append = false) {
		if (!append)
			buffer.resize(0);
		std::lock_guard<std::recursive_mutex> lock(_MutexThreadNames);
		return String::Append(buffer,_ThreadNames[id]);
	}

	static void Dump(const UInt8* data, UInt32 size, Buffer& buffer);

	/// \brief Unpack url in path and query
	/// \param url Url to unpack
	/// \param path Part of the url between host address and '?'
	/// \param query Part of the url after '?' (if present)
	/// \return The position of the file in path (std::npos for a directory)
	static std::size_t UnpackUrl(const std::string& url, std::string& path, std::string& query) {std::string address; return UnpackUrl(url, address, path, query);}
	static std::size_t UnpackUrl(const char* url, std::string& path, std::string& query) {std::string address; return UnpackUrl(url, address, path, query);}
	static std::size_t UnpackUrl(const std::string& url, std::string& address, std::string& path, std::string& query) { return UnpackUrl(url.c_str(), address, path, query); }
	static std::size_t UnpackUrl(const char*, std::string& address, std::string& path, std::string& query);
	
	typedef std::function<bool(const std::string&, const char*)> ForEachParameter;

	static Parameters& UnpackQuery(const std::string& query, Parameters& parameters) { return UnpackQuery(query.data(), query.size(), parameters); }
	static Parameters& UnpackQuery(const char* query, std::size_t count, Parameters& parameters);
	static Parameters& UnpackQuery(const char* query, Parameters& parameters) { return UnpackQuery(query, std::string::npos, parameters); }

	/// \return the number of key/value found
	static UInt32 UnpackQuery(const std::string& query, const ForEachParameter& forEach) { return UnpackQuery(query.data(), query.size(), forEach); }
	static UInt32 UnpackQuery(const char* query, std::size_t count, const ForEachParameter& forEach);
	static UInt32 UnpackQuery(const char* query, const ForEachParameter& forEach) { return UnpackQuery(query, std::string::npos, forEach); }


	static bool ReadIniFile(const std::string& path, Parameters& parameters);

	static unsigned ProcessorCount() { unsigned result(std::thread::hardware_concurrency());  return result > 0 ? result : 1; }
	static const Parameters& Environment() { return _Environment; }

	typedef std::function<bool(char c,bool wasEncoded)> ForEachDecodedChar;

	static UInt32 DecodeURI(const std::string& value, const ForEachDecodedChar& forEach) { return DecodeURI(value.data(),value.size(),forEach); }
	static UInt32 DecodeURI(const char* value, const ForEachDecodedChar& forEach)  { return DecodeURI(value,std::string::npos,forEach); }
	static UInt32 DecodeURI(const char* value, std::size_t count, const ForEachDecodedChar& forEach);
	

	template <typename BufferType>
	static BufferType&	EncodeURI(const char* in, BufferType& out) { return EncodeURI(in, std::string::npos, out); }

	template <typename BufferType>
	static BufferType&	EncodeURI(const char* in, std::size_t count, BufferType& out) {
		while (count && (count!=std::string::npos || *in)) {
			char c = *in++;
			if (isxml(c))
				Buffer::Append(out, &c, 1);
			else if (c <= 0x20 || c > 0x7E || strchr(_URICharReserved,c))
				String::Append(out, '%', Format<UInt8>("%2X", (UInt8)c));
			else
				Buffer::Append(out, &c, 1);
			if(count!=std::string::npos)
				--count;
		}
		return out;
	}

	template<typename Type>
	static Type Random() {

		static UInt32 x = (UInt32)Time::Now() * Process::Id();
		static UInt32 y = 362436069, z = 521288629, w = 88675123;
		UInt32 t = x ^ (x << 11);
		x = y; y = z; z = w;
		return (w = w ^ (w >> 19) ^ (t ^ (t >> 8))) % std::numeric_limits<Type>::max();
	}
	static void Random(UInt8* data, UInt32 size) {for (UInt32 i = 0; i < size; ++i) data[i] = Random<UInt8>();}

	
	template <typename BufferType>
	static BufferType& ToBase64(const UInt8* data, UInt32 size, BufferType& buffer,bool append=false) {
		UInt32 accumulator(buffer.size()),bits(0);

		if (!append)
			accumulator = 0;
		buffer.resize(accumulator+UInt32(ceil(size/3.0)*4));

		char*		current = (char*)buffer.data();
		if (!current) // to expect null writer 
			return buffer;
		const char*	end(current+buffer.size());
		current += accumulator;

		const UInt8* endData = data + size;
		
		accumulator = 0;
		while(data<endData) {
			accumulator = (accumulator << 8) | (*data++ & 0xFFu);
			bits += 8;
			while (bits >= 6) {
				bits -= 6;
				*current++ = _B64Table[(accumulator >> bits) & 0x3Fu];
			}
		}
		if (bits > 0) { // Any trailing bits that are missing.
			accumulator <<= 6 - bits;
			*current++ = _B64Table[accumulator & 0x3Fu];
		}
		while (current<end) // padding with '='
			*current++ = '=';
		return buffer;
	}


	template <typename BufferType>
	static bool FromBase64(BufferType& buffer) { return FromBase64(BIN buffer.data(), buffer.size(), buffer); }

	template <typename BufferType>
	static bool FromBase64(const UInt8* data, UInt32 size,BufferType& buffer,bool append=false) {
		if (!buffer.data()) // to expect null writer 
			return false;

		UInt32 bits(0), oldSize(append ? buffer.size() : 0);
		UInt32 accumulator(oldSize + UInt32(ceil(size / 4.0) * 3));
		const UInt8* end(data+size);

		if (buffer.size()<accumulator)
			buffer.resize(accumulator); // maximum size!
		UInt8* out(BIN buffer.data() + oldSize);

		accumulator = size = 0;

		while(data<end) {
			const int c = *data++;
			if (isspace(c) || c == '=')
				continue;

			if ((c > 127) || (c < 0) || (_ReverseB64Table[c] > 63)) {
				// reset the oldSize
				buffer.resize(oldSize);
				return false;
			}
		
			accumulator = (accumulator << 6) | _ReverseB64Table[c];
			bits += 6;
			if (bits >= 8) {
				bits -= 8;
				size++;
				*out++ = ((accumulator >> bits) & 0xFFu);
			}
		}
		buffer.resize(oldSize+size);
		return true;
	}

	enum HexOption {
		HEX_CPP=1,
		HEX_TRIM_LEFT=2,
		HEX_APPEND=4,
		HEX_UPPER_CASE=8
	};

	template <typename BufferType>
	static BufferType&	FormatHex(const UInt8* data, UInt32 size, BufferType& buffer, UInt8 options=0) {
		if (!buffer.data()) // to expect null writer 
			return buffer;

		const UInt8* end(data+size);
		bool skipLeft(false);
		if (options&HEX_TRIM_LEFT) {
			while(data<end) {
				if (((*data) >> 4)>0)
					break;
				if (((*data) & 0x0F) > 0) {
					skipLeft = true;
					break;
				}
				++data;
			}
		}

		UInt32 oldSize(options&HEX_APPEND ? buffer.size() : 0);
		buffer.resize((end-data) * ((options&HEX_CPP) ? 4 : 2) - (skipLeft ? 1 : 0) + oldSize);
		UInt8* out((UInt8*)buffer.data() + oldSize);
	
		UInt8 ref(options&HEX_UPPER_CASE ? '7' : 'W');

		UInt8 value;
		while(data<end) {
			if (options&HEX_CPP) {
				*out++ = '\\';
				*out++ = 'x';
			}
			value = (*data) >> 4;
			if (!skipLeft)
				*out++ = (value>9 ? (value + ref) : '0' + value);
			else
				skipLeft = false;
			value = (*data) & 0x0F;
			*out++ = (value > 9 ? (value + ref) : '0' + value);
			++data;
		}
		return buffer;
	}

	template <typename BufferType>
	static BufferType& UnformatHex(BufferType& buffer) { return UnformatHex(BIN buffer.data(), buffer.size(), buffer); }

	template <typename BufferType>
	static BufferType& UnformatHex(const UInt8* data, UInt32 size, BufferType& buffer, bool append=false) {
		if (!buffer.data()) // to expect null writer 
			return buffer;

		const UInt8* end(data+size);
		UInt8* out;
		UInt32 oldSize(append ? buffer.size() : 0);
		size = oldSize + UInt32(ceil(size / 2.0));
		if (buffer.size()<size)
			buffer.resize(size);
		out = BIN buffer.data() + oldSize;

		while(data<end) {
			UInt8 first = toupper(*data++);
			UInt8 second = (data == end) ? '0' : toupper(*data++);
			*out++ = ((first - (first<='9' ? '0' : '7')) << 4) | ((second - (second<='9' ? '0' : '7')) & 0x0F);
		}
		buffer.resize(size);
		return buffer;
	}


private:

	class EnvironmentParameters : public MapParameters, public virtual Object {
	public:
		EnvironmentParameters();
	};

	static EnvironmentParameters			_Environment;

	static std::map<THREAD_ID, std::string>	_ThreadNames;
	static std::recursive_mutex				_MutexThreadNames;

	static const char*						_URICharReserved;
	static const char						_B64Table[65];
	static const char						_ReverseB64Table[128];
};


} // namespace Mona
