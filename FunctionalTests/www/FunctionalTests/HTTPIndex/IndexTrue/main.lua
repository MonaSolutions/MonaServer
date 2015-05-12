
function onConnection(client,...)
  INFO("Connection to ", client.path, "...")
  
  return {index=true}
end