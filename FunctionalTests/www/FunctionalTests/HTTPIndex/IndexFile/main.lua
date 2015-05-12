
function onConnection(client,...)
  INFO("Connection to ", client.path, "...")
  
  return {index="file.txt"}
end