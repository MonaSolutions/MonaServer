
-- ******* TestSerialize parameters *******

tabSerialize = {
  {10,10,100,100},
  {x=10,y=10,width=100,height=100},
  {x=10,y=10,100,100},
  {x={10,10,10,10}},
  {x={width=100,height=100,100,100}},
  {x={100,100,width=100,height=100}},
  {a=1,b=2,3,4,x={{{1,2,3}}}}
}

-- ******* Socket Policy file *******

serverPolicyFile = mona:createTCPServer()
function serverPolicyFile:onConnection(client)
        
        function client:onData(data)
                INFO("Sending policy file...")
                self:send("<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>\0")
                return 0 -- return rest (all has been consumed here)
        end
end
serverPolicyFile:start(843); -- start the server on the port 843


-- ******* Main functions *******
function onConnection(client,...)
	
  INFO("New client on FunctionalTests (protocol : ", client.protocol, ")")
  
	function client:onSerialize(mode)
	
		for key,value in pairs(tabSerialize) do
			-- Serialize current object
			local result2JSON = ""
			-- JSON
			local result = mona:toJSON(value)
			INFO("JSON : ", result)
			result2JSON = mona:toJSON(mona:fromJSON(result))
			local msg2JSON = mona:toJSON(value) -- JSON serialization is considerated as valid
			
			-- Return result
			if result2JSON ~= msg2JSON then -- incorrect result
				local message = "Test["..key.."] : expected '"..msg2JSON.."' and '"..result2JSON.."' received"
				client.writer:writeInvocation("onFinished", message)
			end
		end
		client.writer:writeInvocation("onFinished", "")
	end
  
	-- For HTTPLoad, HTTPReconnect and Deserialize tests
	function client:onMessage(...)
		NOTE(path) -- to see if we are in app or subapp
		INFO("Message : ", mona:toJSON(...))
		
		client.writer:writeMessage(...)
	end
  
  return {index="flash.html",timeout=7}
end