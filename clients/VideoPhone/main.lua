
meeters = {}

function onConnection(client, userName)

  if (client.protocol == "RTMFP") then
    INFO("User connected: ", userName , " address: ", client.address)
    client.name = userName
    meeters[userName] = client
  else
    INFO("Connection from client ", client.protocol)
  end
  
  function client:callUser(name)
  
    INFO("Trying to call ", name, "...")
    peer = meeters[name]
    
    if peer then
      INFO("Founded!")
      return peer.id
    else
      error("User "..name.." not founded!")
    end
  end
  
  function client:relay(identity, action, userName)
  
    INFO("Relaying action ", action, " from ", userName, " to ", identity)
    for id, peer in pairs(mona.clients) do
      if id == identity then
        peer.writer:writeInvocation("onRelay", client.id, action, client.name)
        return
      end
    end
    
  end
end
