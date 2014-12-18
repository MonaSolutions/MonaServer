
function onConnection(client,...)
	
  INFO("New client on FunctionalTests/subapp/subapp (protocol : ", client.protocol, ")")
  
  function client:getRealNameApp()
	  INFO("getRealNameApp called")
	  return "subsubapp"
  end
  
  function client:getNameParentApp()
	  INFO("getNameParentApp called")
	  return super:getNameApp()
  end
  
  function client:getNameSuperParentApp()
	 INFO("getNameSuperParentApp called")
	 return super.super:getNameApp() 
  end
end