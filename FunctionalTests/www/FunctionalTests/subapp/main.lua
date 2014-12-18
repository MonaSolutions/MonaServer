
-- For ParentFunctions test
function getNameApp()
	INFO("subapp::getNameApp called")
   return "subapp" 
end

function onConnection(client,...)
	
  INFO("New client on FunctionalTests/subapp (protocol : ", client.protocol, ")")
  
  function client:onMessage(data)
	NOTE(path)
	INFO("Message : ", mona:toJSON(data))

    client.writer:writeMessage(data)
  end
  
  return {index=true,timeout=7}
end