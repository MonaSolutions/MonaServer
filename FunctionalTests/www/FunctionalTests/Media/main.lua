
function onConnection(client)
  NOTE("Connection to ", path)
  
  function client:readMedia(fileName)
    local response = {err="",packetType="",finished=false}    
    
    if not client.publication then
      client.publication = mona:publish("file")
      if not client.publication then error("already published") end
      
      client.media = io.open(mona:absolutePath(path) .. fileName,"rb")
      INFO("Reading file : ", mona:absolutePath(path) .. fileName)
      if not client.media then client.publication:close(); error("Enable to open media") end
      
      client.offset = 0; client.lastTime = 0
    end  
  
    return publishMedia(client, response)
  end
  
end

-- Publish current packet
-- (Header is of size 15)
function publishMedia(client, response)
  
  local data = ""
  if client.offset == 0 then
    data = client.media:read(24)
    -- Skip 9 first bytes of file header (FLV 0x1 0x5 0x00000009)
    data = data:sub(10)
    client.offset = 25
  else
    data = client.media:read(15)
    client.offset = client.offset + 15
  end
  
  -- Media finished?
  if not data or (#data < 15) then response.finished=true; io.close(client.media); client.publication:close(); return response end
  
  -- Read header
  response.packetType = data:byte(5)
  local size = data:byte(6)*65536+data:byte(7)*256+data:byte(8)
  local time = data:byte(9)*65536+data:byte(10)*256+data:byte(11)
  --DEBUG("Current packet type : ", response.packetType, " ; size : ", size, " ; time : ", time)
  
  -- Read payload data
  data = client.media:read(size)
  if not data then response.err="Unexpected end of the file"; io.close(client.media); client.publication:close(); return response end
  if #data < size then response.err="Unexpected payload size : "..#data.." (expected "..size..")"; io.close(client.media); client.publication:close(); return response end
  client.offset = client.offset + size
  
  -- publish Audio and Video
  if response.packetType == 0x09 then
    client.publication:pushVideo(time, data)
  elseif response.packetType == 0x08 then
    client.publication:pushAudio(time, data)
  end
  client.publication:flush()
  
  -- While time is < 500ms continue!
  if (time - client.lastTime) < 500 then
    return publishMedia(client, response)
  else
    INFO("Published ", (time - client.lastTime), "ms")
    client.lastTime = time
  end
  
  return response
end