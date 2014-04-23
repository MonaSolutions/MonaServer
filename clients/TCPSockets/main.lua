--[[
This lua script implements :
 - a TCP Client which send messages to a TCP Server on port 1234
 (messages are first sended by a websocket client to Mona)
 - a TCP Server which is waiting for messages comming on port 1235
]]

----------------- TCP Client part -----------------
socket = mona:createTCPClient()
function socket:onReception(data)
	NOTE("TCPClient::Reception from "..self.peerAddress.." to "..self.address.." : ")
	INFO(data)
	return 0 -- return rest (all has been consumed here)
end

function socket:onDisconnection()
	if self.error then -- error? or normal disconnection?
		ERROR(self.error)
	end
	NOTE("TCPClient::Disconnection")
end

----------------- TCP Server part --------------------
server = mona:createTCPServer()
function server:onConnection(client,...)
	
	INFO("Connection to TCPServer from : ", mona:toJSON(client))
	
	-- Here we have a TCPClient object, same usage than TCPClient
	function client:onReception(data)
		NOTE("TCPServer::Reception from ", self.peerAddress, " to ", self.address)
		INFO(data)
		self:send(data) -- return message to the sender
		return 0 -- return rest (all has been consumed here)
	end
	
	function client:onDisconnection()
		NOTE("TCPServer::Disconnection of a tcp client")
	end
end
server:start(1235); -- start the server on the port 1235

----------------- Websocket Server part -----------------
function onConnection(client,...)
	
	function client:onMessage(message)
		
		if socket.connected then -- useless if already disconnected
			socket:disconnect()
		end
		
		local err = socket:connect("localhost", 1234) -- Client connect on 1234
		if err then ERROR(err) end
		
		INFO("Message from websocket : ", message)
		socket:send(message)
	end
end
