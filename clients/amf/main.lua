
function onConnection(client,...)
	
	INFO("Connection of a new client amf")
  
	function client:onMessage(data)
    INFO("New message from amf : ")
    INFO("toJSON : ", mona:toJSON(data))
  
    return data
	end
end