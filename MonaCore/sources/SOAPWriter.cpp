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

#include "Mona/SOAPWriter.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {


SOAPWriter::SOAPWriter() : XMLWriter() {
	writer.writeRaw("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	writer.writeRaw("<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" ");
	writer.writeRaw("xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" ");
	writer.writeRaw("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ");
	writer.writeRaw("xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" ");
	writer.writeRaw("xmlns:ns=\"urn:mona\">\n");
	writer.writeRaw("<SOAP-ENV:Body>\n");
	writer.writeRaw("<ns:MonaResponse>\n");
	writer.writeRaw("<result>\n");
}

void SOAPWriter::end() {

	writer.writeRaw("\n</result>\n");
	writer.writeRaw("</ns:aMonaResponse>\n");
	writer.writeRaw("</SOAP-ENV:Body>\n");
	writer.writeRaw("</SOAP-ENV:Envelope>");
}

} // namespace Mona
