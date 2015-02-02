require("utilTests.lua")

local tests = {} -- list of LUA Tests

-- ******* Socket Policy file *******

serverPolicyFile = mona:createTCPServer()
function serverPolicyFile:onConnection(client)
        
	function client:onData(data)
		INFO("Sending policy file...")
		self:send('<cross-domain-policy><allow-access-from domain="*" secure="false" to-ports="*"/></cross-domain-policy>\0')
		return data.length -- (all has been consumed here)
	end
end
NOTE("Starting server policy file on port 843 : ", serverPolicyFile:start(843)) -- start the server on the port 843


-- ******* For ParentFunctions Test *******
function getNameApp()
	INFO("FunctionalTests::getNameApp called")
	return "FunctionalTests"
end

-- ******* Main functions *******
function onConnection(client,...)
	
  INFO("New client on FunctionalTests (protocol : ", client.protocol, ")")
	
	-- Return the list of LUA tests availables
	function client:listTests()
		
		local list = {}
		local index = 1
		for _,filePath in pairs(mona:listPaths(path.."/LUATests")) do
			if filePath.isFolder then
				INFO(filePath.name)
				local child = children("LUATests/"..filePath.name)
				if child and child.run then
					tests[index] = child
					list[child.name] = index
					index = index+1
				end
			end
		end
		
		NOTE("List of tests : ", mona:toJSON(list))
		return list
	end
	
	-- Run the test and return the result of the test
	function client:runTest(index)
		local test = tests[index]
		if not test then return "Test of index '"..index.."' doesn't exists" end
		local start, elapsed = mona:time(), 0
		local args = table.pack(pcall(test.run))
		local err = false
		if args[1] then
			table.remove(args,1)
			for i,f in pairs(args) do
				if type(f) == "function" then
					for k,v in pairs(test) do
						if v==f then
							local start2 = mona:time()
							ok,err = pcall(f)
							if ok then
								elapsed = mona:time()-start2
								INFO(test.name,":",k," OK (",elapsed,"ms)")
								client.writer:writeInvocation("INFO",test.name..":"..k.." OK ("..elapsed.."ms)")
							else
								ERROR(test.name,":",k," test failed, ",err)
							end
						end
						if err then break end
					end
				end
				if err then break end
			end
			if not err then
				elapsed = mona:time()-start
				NOTE(test.name," OK (",elapsed,"ms)")
				return {err="", elapsed=elapsed}
			end
		else
			err = args[2]
		end
		ERROR(test.name," test failed, ",err)
		return {err=test.name.." test failed, "..err}
	end
  
	-- For HTTPLoad, HTTPReconnect and Deserialize tests, just return parameters deserialized and serialized
	function client:onMessage(...)
		NOTE(path) -- to see if we are in app or subapp
		INFO("Message : ", mona:toJSON(...))
		
		return ...
	end
  
  return {index="FunctionalTests.html",timeout=7}
end