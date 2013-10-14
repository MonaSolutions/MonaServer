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

#include "Mona/SDP.h"
#include "Mona/Peer.h"
#include "Mona/Logs.h"
#include "Poco/String.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"
#include "Poco/Format.h"
#include "Poco/Exception.h"


using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

SDP::SDP() : supportMsId(false),version(0),sessionId(0) {
}

SDP::SDP(const string& text) : supportMsId(false),version(0),sessionId(0) {
	build(text);
}

SDP::~SDP() {
	clearMedias();
}

void SDP::clearMedias() {
	map<string,SDPMedia*>::iterator it;
	for(it=_medias.begin();it!=_medias.end();++it)
		delete it->second;
	_medias.clear();
}

void SDP::build(const string& text) {

	SDPMedia* pMedia = NULL;

	StringTokenizer lines(text,"\r\n",StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
	StringTokenizer::Iterator it;
	for(it=lines.begin();it!=lines.end();++it) {
		// process each line!
		string& line = (string&)*it;
		if(line[1] != '=')
			throw Exception("SDP line %s malformed, second byte isn't an equal sign",line.c_str());
	
		UInt8 type = line[0];
		line.erase(0,2);
		trimLeftInPlace(line);

		try {

			// RFC 4566
			switch(type) {
				case 'v': // v=  (protocol version)
					version = NumberParser::parseUnsigned(line);
					break;
				case 'o': { // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
					StringTokenizer fields(line," ",StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
					poco_assert(fields.count()==6);
					user = fields[0];
					sessionId = NumberParser::parseUnsigned(fields[1]);
					sessionVersion = NumberParser::parseUnsigned(fields[2]);
					unicastAddress = IPAddress(fields[5],fields[4]=="IP6" ? IPAddress::IPv6 : IPAddress::IPv4);
					break;
				}
				case 's': // s=  (session name)
					sessionName = line;
					break;

				/// Optional lines
				case 'i': // i=* (session information)
					sessionInfos = line;
					break;
				case 'u': // u=* (URI of description)
					uri = line;
					break;
				case 'e': // e=* (email address)
					email = line;
					break;
				case 'p': // p=* (phone number)
					phone = line;
					break;
				case 'c': { // c=* (connection information -- not required if included in all media)
					StringTokenizer fields(line," ",StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
					poco_assert(fields.count()==3);
					IPAddress defaultAddress = IPAddress(fields[2],fields[1]=="IP6" ? IPAddress::IPv6 : IPAddress::IPv4); // TODO defaultAddress is useless for what?
					break;
				}
				case 'b': // b=* (zero or more bandwidth information lines)
					// TODO useless?
					break;

				 
				 // One or more time descriptions ("t=" and "r=" lines; see below)
				 // t=  (time the session is active)
				 // r=* (zero or more repeat times)
				case 't':
					// TODO useless?
					break;
				case 'r':
					// TODO useless?
					break;
				case 'z': // z=* (time zone adjustments)
					// TODO useless?
					break;

				case 'k': // k=* (encryption key)
					encryptKey = line;
					break;
				case 'm': { // m=<name> <port> <proto> <fmt>
					StringTokenizer values(line," ",StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
					poco_assert(values.count()>=4);
					pMedia = addMedia(values[0],NumberParser::parseUnsigned(values[1]),values[2]);
					StringTokenizer::Iterator it;
					for(it=values.begin();it!=values.end();++it)
						pMedia->formats.push_back(NumberParser::parseUnsigned(*it));
					break;
				}
				case 'a': // a=* (zero or more session attribute lines)

					list<string>* pFields = NULL;
					bool	isMsId = false;
					string   fingerHash;

					// TODO SDPSource* pSource = NULL;
					// TODO list<UInt32>* pSourceGroupe = NULL;

					StringTokenizer fields(line," ",StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
					StringTokenizer::Iterator it;
					for(it=fields.begin();it!=fields.end();++it) {
						const string& field=*it;
						size_t pos = field.find(':');
						string key,value;
						if(pos!=string::npos) {
							key = it->substr(0,pos);
							value = it->substr(pos+1);
						} else 
							key = *it;

						// RFC 5576
						if(pMedia) {
							/* TODO
							if(pSourceGroupe)
								pSourceGroupe->push_back(NumberParser::parseUnsigned(field));
							else if(pSource) {
								if(isMsId)
									pSource->msData = field;
								else if(key=="cname") // cname:<value>
									pSource->cname = value;
								else if(key=="msid") {// draft-alvestrand-mmusic-msid-00
									isMsId = true;
									pSource->msId = value;
								} else if(key=="mslabel") // draft-alvestrand-rtcweb-mid-01
									pSource->msLabel = value;
								else if(key=="label") // draft-alvestrand-rtcweb-mid-01
									pSource->label = value;
								else
									WARN("Media source attribute %s unknown",key.c_str());
							} else if(key=="ssrc") // a=ssrc:<ssrc-id>
								pSource = &sources[NumberParser::parseUnsigned(key)];// a=ssrc:<ssrc-id> <attribute>
							else if(key=="rtcp-mux")
								pMedia->rtcpMux = true;
							else if(key=="mid")  // RFC 3388, a=mid:<token>
								pMedia->mid = value;
							else if(key=="ssrc-group")  // a=ssrc-group:<semantics>
								pSourceGroupe = &sourceGroupes[value];
							else if(key="crypto") { // a=crypto:<tag> <crypto-suite> <key-params> [<session-params>]
								StringTokenizer values(value," ",StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
								poco_assert(values.size()>2)
								pMedia->cryptoTag = 
							} else */
								WARN("Media attribute %s unknown",key.c_str());
						} else if(pFields)
							pFields->push_back(field);
						else if(!fingerHash.empty())
							int i=0; // TODO finger = talk_base::SSLFingerprint::CreateFromRfc4572(fingerHash, field);
						else if(key=="group") // RFC 5888 and draft-holmberg-mmusic-sdp-bundle-negotiation-00
							pFields = &groups[value];
						else if(key=="ice-ufrag")
							iceUFrag = value;
						else if(key=="ice-pwd")
							icePwd = value;
						else if(key=="ice-options") {
							pFields = &iceOptions;
							iceOptions.push_back(value);
						} else if(key=="fingerprint") // fingerprint:<hash> <algo>
							fingerHash = value; // fingerprint:<hash>
						else if(key=="msid-semantic")
							supportMsId = value=="VMS";
						else if(key=="extmap") // RFC 5285 a=extmap:<value>["/"<direction>] <URI> <extensionattributes>
							pFields = &extensions[value];
						else 
							WARN("SDP attribute %s unknown",key.c_str());
					}
					
					break;
			}

		} catch(Exception& ex) {
			throw Exception(format("SDP '%c' value %s malformed, %s",type,line,ex.displayText()));
		}
	}
}

void SDP::build() {
	// TODO
}


void SDP::SendNewCandidate(Writer& writer,SDPCandidate& candidate) {
	DataWriter& packet = writer.writeInvocation("__ice");
	packet.beginObject();
	packet.writeStringProperty("candidate",candidate.candidate);
	// TODO add? packet.writeStringProperty("sdpMid",candidate.mid);
	packet.writeNumberProperty("sdpMLineIndex",(double)candidate.mLineIndex);
	packet.endObject();
}

} // namespace Mona
