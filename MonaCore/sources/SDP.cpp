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


using namespace std;
using namespace Poco::Net;

namespace Mona {

void SDP::clearMedias() {
	map<string,SDPMedia*>::iterator it;
	for(it=_medias.begin();it!=_medias.end();++it)
		delete it->second;
	_medias.clear();
}

bool SDP::build(Exception& ex, const string& text) {

	SDPMedia* pMedia = NULL;

	vector<string> lines;
	String::Split(text, "\r\n", lines, String::TOK_IGNORE_EMPTY | String::TOK_TRIM);
	for(string& line : lines) {

		if(line[1] != '=') {
			ex.set(Exception::FORMATTING, "SDP line ",line," malformed, second byte isn't an equal sign");
			return false;
		}
	
		UInt8 type = line[0];
		line.erase(0,2);
		Poco::trimLeftInPlace(line);

		// RFC 4566
		switch(type) {
			case 'v': // v=  (protocol version)
				version = String::ToNumber<UInt32>(ex, line);
				break;
			case 'o': { // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
				vector<string> fields;
				String::Split(line," ", fields, String::TOK_IGNORE_EMPTY | String::TOK_TRIM);
				if (fields.size()!=6) {
					ex.set(Exception::PROTOCOL, "fields.size()!=6");
					break;
				}
				user = fields[0];
				sessionId = String::ToNumber<UInt32>(ex, fields[1]);
				if (ex)
					break;
				sessionVersion = String::ToNumber<UInt32>(ex, fields[2]);
				if (ex)
					break;

				try {
					unicastAddress = IPAddress(fields[5],fields[4]=="IP6" ? IPAddress::IPv6 : IPAddress::IPv4);
				} catch(Poco::Exception& exp) {
					ex.set(Exception::PROTOCOL, exp.displayText());
				}
				
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
				vector<string> fields;
				String::Split(line," ", fields, String::TOK_IGNORE_EMPTY | String::TOK_TRIM);
				if(fields.size()!=3) {
					ex.set(Exception::PROTOCOL, "fields.size()!=3");
					break;
				}

				try {
					IPAddress defaultAddress = IPAddress(fields[2],fields[1]=="IP6" ? IPAddress::IPv6 : IPAddress::IPv4); // TODO defaultAddress is useless for what?
				} catch (Poco::Exception exp) {
					ex.set(Exception::PROTOCOL, exp.displayText());
				}
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
				vector<string> values;
				String::Split(line," ", values, String::TOK_IGNORE_EMPTY | String::TOK_TRIM);
				if (values.size()<4) {
					ex.set(Exception::PROTOCOL , "values.size()<4");
					break;
				}

				UInt16 val = String::ToNumber<UInt16>(ex, values[1]);
				if (ex)
					break;

				pMedia = addMedia(values[0], val, values[2]);
				for(const string& st : values) {
					pMedia->formats.push_back(String::ToNumber<UInt8>(ex, st));
					if (ex)
						break;
				}
				break;
			}
			case 'a': { // a=* (zero or more session attribute lines)

				list<string>* pFields = NULL;
				bool	isMsId = false;
				string   fingerHash;

				// TODO SDPSource* pSource = NULL;
				// TODO list<UInt32>* pSourceGroupe = NULL;

				vector<string> fields;
				String::Split(line," ", fields, String::TOK_IGNORE_EMPTY | String::TOK_TRIM);
				for(const string& st : fields) {
					size_t pos = st.find(':');
					string key,value;
					if(pos!=string::npos) {
						key = st.substr(0,pos);
						value = st.substr(pos+1);
					} else 
						key = st;

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
								WARN("Media source attribute ",key," unknown");
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
							WARN("Media attribute ",key," unknown");
					} else if(pFields)
						pFields->push_back(st);
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
						WARN("SDP attribute ",key," unknown");
				}
			} // case 'a':
			break;
		} // switch(type)

		if (ex) {
			ex.set(Exception::PROTOCOL, "SDP '",type,"' value ",line," malformed, ",ex.error());
			return false;
		}
	} // for(string& line : lines)

	return true;
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
