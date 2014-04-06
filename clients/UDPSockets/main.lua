--[[
This lua script implements :
 - a UDP Socket which send messages to an other UDP Socket on port 1234
 (messages are first sended by a websocket client to Mona)
]]

----------------- UDP Socket part --------------------
serverUDP = mona:createUDPSocket()
function serverUDP:onReception(data,address)
    NOTE("Reception from "..address)
    INFO(data)
end
err = serverUDP:bind("0.0.0.0",1235) -- start the server on port 1235
if err then ERROR(err) end

----------------- Websocket Server part -----------------
clientUDP = mona:createUDPSocket()
function onConnection(client,...)
	
    clientUDP:connect("127.0.0.1", 1234) -- Connect to udp server on port 1234
    
	function client:onMessage(message)
		
		INFO("Message from websocket : ", message)
        NOTE("UDP socket opened on ", clientUDP.address, " connected to ", clientUDP.peerAddress)
        clientUDP:send(message)
	end
end
