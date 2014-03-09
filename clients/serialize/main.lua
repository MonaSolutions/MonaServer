
function onConnection(client,...)
	
	INFO("Connection of a new client to serialize")
  
  function client:onRead(file)
    
    if file == "" and client.protocol == "HTTP" then
      return "index.html"
    end
  end
  
  function client:onMessage(mode, data)
    INFO("Message to convert in ", mode[1])
    INFO("msg : ", mona:toJSON(data))

    if mode[1] == "json" then
      client.writer:writeRaw(mona:toJSON(data))
    else -- xml
      client.writer:writeRaw(mona:toXML(data))
    end
	end
end