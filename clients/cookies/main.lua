-- Client sample for HTTP cookies

function onConnection(client,...)
	
	INFO("New client on ", path)
  
	function client:setCookies(cookies)
		
		NOTE("client.properties : ")
		local array = "<h2>Client Properties :</h2><table>"
		for key, prop in pairs(client.properties) do
			INFO(key, " : ", prop)
			array = array .. "<tr><td>" .. key .. "</td><td>" .. prop .. "</td></tr>"
		end
		array = array .. "</table>"
		
		NOTE("Cookies : ")
		array = array .. "<h2>Cookies :</h2><table>"
		for index, cookie in ipairs(cookies) do
			INFO(index, " : ", mona:toJSON(cookie))
			local result = client.properties(cookie.key, cookie.value, cookie)
			INFO("set cookie result : ", result)
			array = array .. "<tr><td>" .. mona:toJSON(cookie) .. "</td><td>" .. result .. "</td></tr>"
		end
		array = array .. "</table>"
		client.writer:writeRaw(array)
	end
end