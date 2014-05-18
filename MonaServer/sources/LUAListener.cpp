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

#include "LUAListener.h"
#include "LUAQualityOfService.h"
#include "LUAPublication.h"
#include "LUAClient.h"

using namespace std;
using namespace Mona;

void LUAListener::Clear(lua_State* pState,const Listener& listener){
	Script::ClearObject<QualityOfService, LUAQualityOfService>(pState, listener.dataQOS());
	Script::ClearObject<QualityOfService, LUAQualityOfService>(pState, listener.audioQOS());
	Script::ClearObject<QualityOfService, LUAQualityOfService>(pState, listener.videoQOS());
}

int LUAListener::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Listener,listener)
		const char* name = SCRIPT_READ_STRING(NULL);
		if (name) {
			if(strcmp(name,"audioQOS")==0) {
				SCRIPT_ADD_OBJECT(QualityOfService,LUAQualityOfService,listener.audioQOS())
			} else if(strcmp(name,"videoQOS")==0) {
				SCRIPT_ADD_OBJECT(QualityOfService, LUAQualityOfService, listener.videoQOS())
			} else if (strcmp(name, "dataQOS") == 0) {
				SCRIPT_ADD_OBJECT(QualityOfService, LUAQualityOfService, listener.dataQOS())
			} else if(strcmp(name,"publication")==0) {
				SCRIPT_ADD_OBJECT(Publication, LUAPublication<>, listener.publication);
			} else if(strcmp(name,"receiveAudio")==0) {
				SCRIPT_WRITE_BOOL(listener.receiveAudio);
			} else if(strcmp(name,"receiveVideo")==0) {
				SCRIPT_WRITE_BOOL(listener.receiveVideo);
			} else if(strcmp(name,"client")==0) {
				SCRIPT_ADD_OBJECT(Client, LUAClient, listener.client);
			} else if (strcmp(name,"properties")==0) {
				if (Script::Collection(pState, 1, "properties")) {
					for (auto& it : listener)
						Script::PushKeyValue(pState, it.first, it.second);
					Script::FillCollection(pState, listener.count());
				}
			} else {
				string value;
				if (listener.getString(name, value))
					Script::PushValue(pState, value);
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAListener::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Listener,listener)
		const char* name = SCRIPT_READ_STRING("");
		if(strcmp(name,"receiveAudio")==0)
			listener.receiveAudio = lua_toboolean(pState,-1)==0 ? false : true;
		else if (strcmp(name, "receiveVideo") == 0)
			listener.receiveVideo = lua_toboolean(pState,-1)==0 ? false : true;
		else
			lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
