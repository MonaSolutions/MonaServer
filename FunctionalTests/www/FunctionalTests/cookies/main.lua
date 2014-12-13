-- Test for HTTP cookies

function onConnection(client,...)
	
	INFO("New client on ", path)
  
	function client:onMessage(cookies)
		
		-- check sended cookies
		assert(client["sendCookie1"]=="Marcel Pagnol")
		assert(client["sendCookie2"]=="Charles-Pierre Baudelaire")
		assert(client["sendCookie3"]=="one=1&two=2")
		
		for index, cookie in ipairs(cookies) do
			
			INFO(cookie.key, " : ", cookie.value)
			-- prepare object Cookie
			local obj = {expires=cookie.expires,secure=cookie.secure,httponly=cookie.httponly,path=cookie.path}
			obj[cookie.key]=cookie.value
			
			-- set the cookie and assert result == value
			assert(client.properties(obj) == cookie.value)
			
			-- check if the client property has been well setted
			if type(cookie.value) == "string" then -- string
				assert(client[cookie.key]==cookie.value)
			else -- number
				assert(tonumber(client[cookie.key])==cookie.value)
			end
		end
		return ""
	end
  
	return {index=false}
end