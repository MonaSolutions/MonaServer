
clientWS = nil
clientAMF = nil

function onConnection(client,...)
	
	INFO("Connection of a new client to push (protocol:"..client.protocol..")")
  
  if client.protocol == "WebSocket" then
    clientWS = client
  elseif client.protocol == "RTMFP" then
      clientAMF = client
  end
  
  function client:onMessage(data)
    INFO("Reception Message : "..mona:toJSON(data))
    
    if client == clientAMF then
      clientWS.writer:writeInvocation("onReception", data)
    else
      clientAMF.writer:writeInvocation("onReception", data)
    end
  end
end