
function onConnection(client,...)
	
	INFO("Connection of a new client amf")
  
	function client:onMessage(data)
    INFO("New message from amf : ")
    INFO("toJSON : ", mona:toJSON(data))
  
    --client.writer:writeInvocation("onReception", data)
    return data
	end
end