function onConnection(client,...)
	
	INFO("Connection of a new client")

	function client:add(a, b)
		INFO("Reception "..a.." + "..b.." = "..(a+b))
		
		return a+b
	end
	
	function client:sayHello(message)
		INFO("Reception "..message)
		
		-- Test writer XML
		-- return {a={att1="lorem",a1={{},{att1="lorem ip",__value="Lorem ipsum"}},a2={{}}}}
		return {message.." recieved"}
	end
end