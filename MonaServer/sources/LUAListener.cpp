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

#include "LUAListener.h"
#include "LUAQualityOfService.h"
#include "LUAPublication.h"
#include "LUAClient.h"

using namespace std;
using namespace Mona;

const char*		LUAListener::Name="Mona::Listener";

void LUAListener::Clear(lua_State* pState,const Listener& listener){
	Script::ClearPersistentObject<QualityOfService,LUAQualityOfService>(pState,listener.dataQOS());
	Script::ClearPersistentObject<QualityOfService,LUAQualityOfService>(pState,listener.audioQOS());
	Script::ClearPersistentObject<QualityOfService,LUAQualityOfService>(pState,listener.videoQOS());
	Script::ClearPersistentObject<Listener,LUAListener>(pState,listener);
}

int LUAListener::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Listener,LUAListener,listener)
		string name = SCRIPT_READ_STRING("");
		if(name=="audioQOS") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(QualityOfService,LUAQualityOfService,listener.audioQOS())
		} else if(name=="videoQOS") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(QualityOfService,LUAQualityOfService,listener.videoQOS())
		} else if(name=="dataQOS") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(QualityOfService,LUAQualityOfService,listener.dataQOS())
		} else if(name=="publication") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(Publication,LUAPublication,listener.publication);
		} else if(name=="receiveAudio") {
			SCRIPT_WRITE_BOOL(listener.receiveAudio);
		} else if(name=="receiveVideo") {
			SCRIPT_WRITE_BOOL(listener.receiveVideo);
		} else if(name=="client") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,listener.client);
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAListener::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Listener,LUAListener,listener)
		string name = SCRIPT_READ_STRING("");
		if(name=="receiveAudio")
			listener.receiveAudio = lua_toboolean(pState,-1)==0 ? false : true;
		 else if(name=="receiveVideo")
			listener.receiveVideo = lua_toboolean(pState,-1)==0 ? false : true;
		else
			lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
