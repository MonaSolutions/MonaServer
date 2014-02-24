
function onConnection(client,...)
	
	INFO("Connection of a new client json")
  
	function client:onMessage(data)
    INFO("New message from json : ")
    INFO("toJSON : ", mona:toJSON(data))
    
    --client.writer:writeInvocation("onReception", data)
    return data
	end
end