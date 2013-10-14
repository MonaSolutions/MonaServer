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
#include "Mona/ServerParams.h"
#include "Mona/Protocol.h"
#include <list>


namespace Mona {

class Protocols {
public:
	Protocols(Invoker& invoker);
	virtual ~Protocols();

	void load(Gateway& gateway);
	void unload();
	void manage();

private:
	template<class ProtocolType, class ParamsType>
	void loadProtocol(const char* name,const ParamsType& params,Gateway& gateway) {
		if(params.port==0)
			return;
		try {
			Protocol* pProtocol = new ProtocolType(name,params,gateway,_invoker);
			_protocols.push_back(pProtocol);
			NOTE("%s server starts on %u %s port",pProtocol->name.c_str(),params.port,dynamic_cast<UDProtocol*>(pProtocol) ? "UDP" : "TCP");
		} catch(Poco::Exception& ex) {
			ERROR("%s server, %s",name,ex.displayText().c_str());
		} catch (std::exception& ex) {
			ERROR("%s server, %s",name,ex.what());
		} catch (...) {
			ERROR("%s server, unknown error",name);
		}
	}

	void requestHandle();

	std::list<Protocol*>	_protocols;
	Invoker&				_invoker;
};


} // namespace Mona
