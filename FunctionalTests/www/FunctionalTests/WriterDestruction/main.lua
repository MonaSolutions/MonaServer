-- 1) Connect with RTMFP
-- 2) Disconnect
-- 3) Connect with HTTP, if it does not crash problem is resolved!

function onConnection(client)
  NOTE("Connection to WriterDestruction (",client.protocol,")")
  
  if (client.protocol == "HTTP") then
    collectgarbage("collect")
  else
    wtest = client.writer:newWriter()
  end
end

function onDisconnection(client)
  NOTE("onDisconnection called from WriterDestruction")
end