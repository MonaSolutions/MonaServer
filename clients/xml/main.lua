
function onConnection(client,...)
	
	INFO("Connection of a new client XML")
  
  function client:onRead(file)
    
    if file == "" and client.protocol == "HTTP" then
      return "index.html"
    end
  end
  
  function client:onMessage(data)
		INFO("Reception XML : ")
    INFO("toJSON : ", mona:toJSON(data))

    return data
	end
end