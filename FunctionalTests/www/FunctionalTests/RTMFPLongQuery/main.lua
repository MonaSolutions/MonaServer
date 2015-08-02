-- 1) Connect with RTMFP and a long query
-- 2) Return the parameters in a new Array and check the result in FunctionalTests

function onConnection(client)
  INFO("Connection to RTMFPLongQuery ")
  DEBUG("Parameters : ", mona:toJSON(client.properties))
  
  return {answer1=client.properties["param1"], answer2=client.properties["param2"], answer3=client.properties["param3"]}
end
