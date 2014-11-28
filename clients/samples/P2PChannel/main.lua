
peers = {}

  function onConnection(client, name)
    
    INFO("User connected on p2p sharing app : ", name)
    peers[client] = name
    
    function client:onInfoSend(file, index)
      
      INFO("User "..peers[client].." is sending file "..file.." ("..index..")")
    end
    
    function client:onInfoRequest()
      
      INFO("User "..peers[client].." has requested file")
    end
  end

  function onDisconnection(client)
    name = peers[client]

    if name then
      INFO("User disconnecting: "..name)
      peers[client] = nil
    end
  end