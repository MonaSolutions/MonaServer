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
#include "Mona/BinaryReader.h"
#include "Mona/BinaryWriter.h"


namespace Mona {

class WS : virtual Static {
public:

	enum MessageType {
		//FRAME_OP_CONT    = 0x00, /// Continuation frame.
		TYPE_NIL		= 0,
		TYPE_TEXT		= 0x01, /// Text frame.
		TYPE_BINARY	= 0x02, /// Binary frame.
		TYPE_CLOSE	= 0x08, /// Close connection.
		TYPE_PING		= 0x09, /// Ping frame.
		TYPE_PONG		= 0x0a /// Pong frame.
	};

	enum ResponseCode {
		CODE_NORMAL_CLOSE				= 1000,
		CODE_ENDPOINT_GOING_AWAY		= 1001,
		CODE_PROTOCOL_ERROR			= 1002,
		CODE_PAYLOAD_NOT_ACCEPTABLE	= 1003,
		CODE_RESERVED					= 1004,
		CODE_RESERVED_NO_STATUS_CODE	= 1005,
		CODE_RESERVED_ABNORMAL_CLOSE	= 1006,
		CODE_MALFORMED_PAYLOAD		= 1007,
		CODE_POLICY_VIOLATION			= 1008,
		CODE_PAYLOAD_TOO_BIG			= 1009,
		CODE_EXTENSION_REQUIRED		= 1010,
		CODE_UNEXPECTED_CONDITION		= 1011
	};

	static std::string&	ComputeKey(std::string& key);
	static void		    Unmask(BinaryReader& reader);
	static UInt8	WriteHeader(MessageType type,UInt32 size,BinaryWriter& writer);
	static UInt8	HeaderSize(UInt32 size);

	
};


} // namespace Mona
