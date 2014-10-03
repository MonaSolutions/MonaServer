
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


-- ******* TestBadRequests parameters *******
nbBadRequests = 0
socket = mona:createTCPClient()

function socket:onDisconnection()
	if self.error then -- error? or normal disconnection?
		ERROR(self.error)
		self.clientWriter:writeInvocation("onFinished", self.error);
	else
		if nbBadRequests < 10 then
			nbBadRequests = nbBadRequests + 1
			local ok = self:connect("localhost",80)
			if ok then self:send("TESTBADREQUEST") end
		else
			self.clientWriter:writeInvocation("onFinished", "");
		end
	end
	NOTE("TCP disconnection")
end


-- ******* Main functions *******
function onConnection(client,...)
	
  INFO("New client on FunctionalTests (protocol : ", client.protocol, ")")
  
	function client:onSerialize(mode)
	
		for key,value in pairs(tabSerialize) do
			-- Serialize current object
			local result2JSON = ""
			if mode == "xml" then
				local result = mona:toXML(value)
				INFO("XML : ", result)
				result2JSON = mona:toJSON(mona:fromXML(result))
			else -- JSON
				local result = mona:toJSON(value)
				INFO("JSON : ", result)
				result2JSON = mona:toJSON(mona:fromJSON(result))
			end
			local msg2JSON = mona:toJSON(value) -- JSON serialization is considerated as valid
			
			-- Return result
			if result2JSON ~= msg2JSON then -- incorrect result
				local message = "Test["..key.."] : expected '"..msg2JSON.."' and '"..result2JSON.."' received"
				client.writer:writeInvocation("onFinished", message)
			end
		end
		client.writer:writeInvocation("onFinished", "")
	end
  
	function client:testBadRequests()
		nbBadRequests = 0
		
		socket.clientWriter = self.writer
		local ok = socket:connect("localhost",80)
		if ok then socket:send("TESTBADREQUEST") end
	end
  
	-- For TestHTTP, TestHTTPReconnect and Deserialize tests
	function client:onMessage(data)
		NOTE(path) -- to see if we are in app or subapp
		INFO("Message : ", mona:toJSON(data))
		
		client.writer:writeMessage(data)
	end
  
  --return {index="index.html",timeout=7}
end