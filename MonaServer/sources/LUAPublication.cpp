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


#include "LUAPublication.h"

using namespace std;
using namespace Mona;


void LUAPublicationBase::Clear(lua_State* pState, const Mona::Publication& publication) {
	Script::ClearObject<QualityOfService, LUAQualityOfService>(pState, publication.dataQOS());
	Script::ClearObject<QualityOfService, LUAQualityOfService>(pState, publication.audioQOS());
	Script::ClearObject<QualityOfService, LUAQualityOfService>(pState, publication.videoQOS());
}

void LUAPublicationBase::AddListener(lua_State* pState, const Listener& listener, int indexListener, int indexClient) {
	if (Script::FromObject<Mona::Publication>(pState, listener.publication)) {
		Script::Collection(pState, -1, "listeners", listener.publication.listeners.count() + 1);
		lua_pushvalue(pState, indexListener);
		lua_pushvalue(pState, indexClient);
		lua_rawset(pState, -3); // rawset cause NewIndexProhibited
		lua_pop(pState, 2);
	}
}

void LUAPublicationBase::RemoveListener(lua_State* pState, const Listener& listener, int indexListener) {
	if (Script::FromObject<Mona::Publication>(pState, listener.publication)) {
		Script::Collection(pState, -1, "listeners", listener.publication.listeners.count() - 1);
		lua_pushvalue(pState, indexListener);
		lua_pushnil(pState);
		lua_rawset(pState, -3); // rawset cause NewIndexProhibited
		lua_pop(pState, 3);
	}
}

Mona::Publication* LUAPublicationBase::Publication(LUAMyPublication& luaPublication) {
	if (luaPublication.closed)
		return NULL;
	return &luaPublication.publication;
}

void LUAPublicationBase::Close(lua_State *pState, Mona::Publication& publication) {
	SCRIPT_BEGIN(pState)
		SCRIPT_ERROR("You have not the handle on publication ", publication.name(), ", you can't close it")
	SCRIPT_END
}

void LUAPublicationBase::Close(lua_State *pState, LUAMyPublication& luaPublication) {
	if (luaPublication.closed)
		return;
	luaPublication.invoker.unpublish(luaPublication.publication.name());
	luaPublication.closed = true;
}

int LUAPublicationBase::Item(lua_State *pState) {
	// 1 => publications table
	// 2 => parameter
	Invoker* pInvoker = Script::GetCollector<Invoker>(pState, 1);
	if (!pInvoker)
		return 0;
	int result(0);
	SCRIPT_BEGIN(pState)
	if (lua_isstring(pState, 2)) {
		auto& it = pInvoker->publications(lua_tostring(pState,2));
		if (it != pInvoker->publications.end()) {
			SCRIPT_ADD_OBJECT(Mona::Publication, LUAPublication<>, it->second);
			++result;
		}
	}
	SCRIPT_END
	return result;
}

int LUAMyPublication::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(LUAMyPublication, publication)
	if (!publication.closed)
		publication.invoker.unpublish(publication.publication.name());
	delete &publication;
	SCRIPT_CALLBACK_RETURN
}
