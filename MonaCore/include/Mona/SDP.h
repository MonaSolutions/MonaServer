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
#include "Mona/Exceptions.h"
#include "Mona/Writer.h"
#include "Poco/URI.h"
#include "Mona/IPAddress.h"
#include <list>
#include <map>


namespace Mona {

struct SDPCandidate {
	std::string		candidate;
	std::string		mid;
	UInt16	mLineIndex;
};

class SDPMedia {
public:
	SDPMedia(UInt16 port,const std::string& codec) : codec(codec),port(port) {}

	const std::string		codec;
	const UInt16		port; // if port==0 the media is rejected!
	std::list<UInt8>	formats;
};

class Peer;
class SDP {
public:
	SDP() : supportMsId(false), version(0), sessionId(0) {}
	virtual ~SDP() { clearMedias(); }

	bool build(Exception& ex, const std::string& text);
	void build();

	UInt32			version;
	std::string				user;
	UInt32			sessionId;
	std::string				sessionVersion;
	std::string				sessionName;
	std::string				sessionInfos;
	IPAddress				unicastAddress;
	Poco::URI				uri;
	std::string				email;
	std::string				phone;
	std::string				encryptKey;

	std::string				iceUFrag;
	std::string				icePwd;
	std::list<std::string>	iceOptions;

	std::string				finger;
	bool					supportMsId;

	std::map<std::string,std::list<std::string> >	groups;

	std::map<std::string,std::list<std::string> >	extensions;

	SDPMedia*	addMedia(const std::string& name,UInt16 port,const std::string& codec);
	void		clearMedias();

	static void SendNewCandidate(Writer& writer,SDPCandidate& candidate);

private:
	std::map<std::string,SDPMedia*> _medias;
};

inline SDPMedia* SDP::addMedia(const std::string& name,UInt16 port,const std::string& codec) {
	return _medias.insert(std::pair<std::string,SDPMedia*>(name,new SDPMedia(port,codec))).first->second;
}


} // namespace Mona
