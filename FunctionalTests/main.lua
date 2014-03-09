
tabSerialize = {
  {10,10,100,100},
  {x=10,y=10,width=100,height=100},
  {x=10,y=10,100,100},
  {x={10,10,10,10}},
  {x={width=100,height=100,100,100}},
  {x={100,100,width=100,height=100}},
  {a=1,b=2,3,4,x={{{1,2,3}}}}
}

indexSerialize = 1

function onConnection(client,...)
	
	INFO("New client on FunctionalTests (protocol : ", client.protocol, ")")
  
  function client:onRead(file)
    
    if file == "" and client.protocol == "HTTP" then
      return "index.html"
    end
  end
  
  function client:onSerialize(mode)
    
    -- Serialize current object
    local result2JSON = ""
    if mode == "xml" then
      local result = mona:toXML(tabSerialize[indexSerialize])
      INFO("XML : ", result)
      result2JSON = mona:toJSON(mona:fromXML(result))
    else -- JSON
      local result = mona:toJSON(tabSerialize[indexSerialize])
      INFO("JSON : ", result)
      result2JSON = mona:toJSON(mona:fromJSON(result))
    end
    local msg2JSON = mona:toJSON(tabSerialize[indexSerialize])
    
    -- Return result
    local message = "continue"
    if result2JSON ~= msg2JSON then -- incorrect result
      message = "Test["..indexSerialize.."] : expected '"..msg2JSON.."' and '"..result2JSON.."' received"
      client.writer:writeRaw(message)
      indexSerialize = 1
    else
      if indexSerialize == #tabSerialize then -- result ok and end 
        client.writer:writeRaw("terminate")
        indexSerialize = 1
      else
        client.writer:writeRaw("continue") -- result ok, continue
        indexSerialize = indexSerialize + 1
      end
    end
  end
  
  function client:onMessage(data)
    INFO("Message : ", mona:toJSON(data))

    return data
	end
end