-- Test client for push

writers = {}

function onConnection(client,...)
	
  INFO("Connection of a new client to the chatroom")
  writers[client] = nil
  
  function client:onIdentification(name)
  
    if name then
      INFO("Trying to connect user : ", name)
      INFO(mona:toJSON(name))
      
      writers[client] = name
      writeMsgToChat("User "..name.." has joined the chatroom!")
    end
  end
  
	function client:onMessage(data)
  
    nameClient = writers[client]
    local message = nil
    if type(data) == "string" then
      message = data
    else 
      if type(data) == "table" and type(data[1]) == "string" then  -- prevent date parsing
        message = data[1]
      end
    end
    
    if nameClient and message then
      INFO("New message from user "..nameClient.." : ")
      INFO(mona:toJSON(message))
      writeMsgToChat(nameClient..">"..message)
    end
	end
end

function writeMsgToChat(message)
  for user,name in pairs(writers) do
      --local newWriter = user.writer:newWriter()
      user.writer:writeInvocation("onReception",message)
  end
end

function onDisconnection(client)

  local name = writers[client]
  if name then
    writers[client] = nil
    writeMsgToChat("User "..name.." has quit the chatroom!")
  end
end