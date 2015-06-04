
function onConnection(client)
  
  -- Return an array of publications' names
  function client:getPublications()
    local res = {publications={}, listeners={}}
    for name,pub in pairs(mona.publications) do
      table.insert(res.publications, name)
      
      for c, listener in pairs(pub.listeners) do
         table.insert(res.listeners, c.id)
      end        
    end
    return res
  end
  
  function client:onSubscribe(listener)
    INFO("onSubscribe called")
    return false -- return false to remove the listener
  end
end