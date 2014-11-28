
function onConnection(client,...)
  INFO("Connection of a new client "..client.protocol)
  function client:onMethod(data)
    INFO("Reception Message : "..mona:toJSON(data))
    client.writer:writeInvocation("onReception",data.." received")
  end
end
