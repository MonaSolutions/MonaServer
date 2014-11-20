
function onConnection(client, query)
	
	INFO("Connection of a new client to serialize")
  
    -- Generate a fake SOAP data with XML-RPC content
	function makeSOAP(data)
		local xmlrpc = mona:toXMLRPC(data)
		string.gsub(xmlrpc, "<params>(.-)</params>", function(a) data = a; end)
		
		local result = '<?xml version="1.0" encoding="utf-8"?>' ..
                '<soapenv:Envelope ' .. 
                'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ' ..
                'xmlns:xsd="http://www.w3.org/2001/XMLSchema" ' ..
                'xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/">' ..
                '<soapenv:Body>' ..
                  '<urn:result xmlns="http://localhost/">' ..
                    data ..
                  '</urn:result>' ..
                '</soapenv:Body>' ..
                '</soapenv:Envelope>'
		
		return result
	end
	
	-- Generate XML data with XML-RPC content
	function makeXML(data)
		local xmlrpc = mona:toXMLRPC(data)
		string.gsub(xmlrpc, "<params>(.-)</params>", function(a) data = a; end)
		return data
	end
	
	-- Parser SOAP (extract the container from content)
	function parseSOAP(data)
		local xml = mona:fromXML(data)
		local params = xml["soapenv:Envelope"]["soapenv:Body"]["urn:onMessage"]
		local result = {}
		return readTag(result, params)
	end
	
	-- Parse a Tag from XML (recursive)
	function readTag(result, params)
		for k,v in pairs(params) do
			if type(v) == "table" and type(v[1]) ~= "table" then -- primtive?
				v = v[1]
			end
			
			-- sub tag
			if type(v) == "table" then
				result[k] = {}
				readTag(result[k], v)
			-- property
			elseif k ~= "__name" then
				if tonumber(v) ~= nil then
					result[k] = tonumber(v) -- number
				else
					result[k] = v -- string
				end
			end
		end
		return result
	end
  
	function client:onMessage(mode, data)
		if query then
			if query["type"] == "soap" then
				data = parseSOAP(mode)
			elseif query["type"] == "xml" then
				data = readTag({}, mona:fromXML(mode))
			elseif query["type"] == "query" then
				data = mona:fromQuery(mode)
			end
			mode = query["mode"]
		end
		NOTE("Message to convert in ", mode)
		INFO("toJSON : ", mona:toJSON(data))
		
		if mode == "json" then
			client.writer:writeRaw(mona:toJSON(data))
		elseif mode == "xmlrpc" then
			client.writer:writeRaw(mona:toXMLRPC(data))
		elseif mode == "soap" then
			client.writer:writeRaw(makeSOAP(data))
		elseif mode == "xml" then
			client.writer:writeRaw(makeXML(data))
		elseif mode == "query" then
			client.writer:writeRaw(mona:toQuery(data))
		elseif mode == "amf" then
			local amf = mona:toAMF(data)
			amf = string.gsub (amf, "([^%w ])", function (c) return string.format ("%02X", string.byte(c)) end)
			client.writer:writeRaw(amf)
		elseif mode == "amf0" then
			local amf = mona:toAMF0(data)
			amf = string.gsub (amf, "([^%w ])", function (c) return string.format ("%02X", string.byte(c)) end)
			client.writer:writeRaw(amf)
		end
	end
	return {index="index.html"}
end