function onConnection(client,...)
	
	INFO("Connection of a new client soap")
  
  function client:onRead()
    if client.protocol == "HTTP" then
      return "index.html"
    end
  end
  
  function client:onMessage(data)
		INFO("Reception SOAP : ")
    INFO("ToJSON : "..mona:toJSON(data))
    data2 = mona:fromJSON(mona:toJSON(data))
    INFO("ToJSON 2 : "..mona:toJSON(data2))
    INFO("ToXML : "..mona:toXML(data))
    
		return data
	end
  
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