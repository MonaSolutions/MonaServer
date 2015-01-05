
-- Test for HTTP cookies

function onConnection(client,...)
	
	INFO("New client on ", path)
  
	function writeCookies(cookies)
		-- check sended cookies
		assert(client["sendCookie1"]=="Marcel Pagnol")
		assert(client["sendCookie2"]=="Charles-Pierre Baudelaire")
		assert(client["sendCookie3"]=="one=1&two=2")
		
		-- check Cookie header
		assert(client["Cookie"]=="sendCookie1=Marcel Pagnol; sendCookie2=Charles-Pierre Baudelaire; sendCookie3=one=1&two=2")
		
		-- Test wit wrong parameters
		assert(client.properties() == nil)
		assert(client.properties("") == nil)
		assert(client.properties(10) == nil)
		assert(client.properties("keyTest") == nil)
		
		-- Add each cookie
		for index, cookie in ipairs(cookies) do
			
			DEBUG(cookie.key, " : ", cookie.value)
			-- prepare object Cookie
			local obj = {expires=cookie.expires,secure=cookie.secure,httponly=cookie.httponly,path=cookie.path}
			
			-- set the cookie and assert result == value
			assert(client.properties(cookie.key, cookie.value, obj) == cookie.value)
			
			-- check if the client property has been well setted
			if type(cookie.value) == "string" then -- string
				assert(client[cookie.key]==cookie.value)
			else -- number
				assert(tonumber(client[cookie.key])==cookie.value)
			end
		end
	end
  
	function client:onMessage(cookies)
		
		local result, message = pcall(writeCookies, cookies)
		INFO("writeCookies result : ", result, " ; ", message)
		
		if result then
			return ""
		else
			error(message)
		end
	end
  
	return {index=false}
end