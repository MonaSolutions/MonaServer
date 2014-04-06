-- Test client for push

writers = {}

-- Function for parsing data coming from a client
-- Prevent formatting problems
function parseMessage(data)
	
	if type(data) == "string" then
		return data
	else
		-- TODO I don't remind me why a table can be a well formed message
		if type(data) == "table" and type(data[1]) == "string" then  -- prevent date parsing
			return data[1]
		else
			WARN("Error in message formatting : ", mona:toJSON(data))
			return data[1]
			
			--for key,value in pairs(data[1]) do
			--	INFO(key, " = ", value)
			--end
		end
	end
end

function onConnection(client,...)
	
	INFO("Connection of a new client to the chatroom")
	writers[client] = nil
  
	-- Identification function
	function client:onIdentification(data)
	
		local name = parseMessage(data)
		
		if name then
			INFO("Trying to connect user : ", name)
			INFO(mona:toJSON(name))
			
			writers[client] = name
			writeMsgToChat("", "User "..name.." has joined the chatroom!")
		end
	end
  
	-- Reception of a message from a client
	function client:onMessage(data)
  
		nameClient = writers[client]
		local message = parseMessage(data)
		
		if not nameClient then
			WARN("Unauthentied user has tried to send a message : ", mona:toJSON(data))
		else
			if message then
				INFO("New message from user "..nameClient.." : ")
				INFO(mona:toJSON(message))
				writeMsgToChat(nameClient..">", message)
			end
		end
	end
end

function writeMsgToChat(prompt, message)
	
  for user,name in pairs(writers) do
      user.writer:writeInvocation("onReception", prompt, message)
  end
end

function onDisconnection(client)

  local name = writers[client]
  if name then
    writers[client] = nil
    writeMsgToChat("", "User "..name.." has quit the chatroom!")
  end
end