
function onConnection(client,...)
	
	INFO("Connection of a new client TestTemplates")
  
	function client:onRead(file, parameters)
    INFO("Reading file "..file.."...")
    
    if file == "templateFile.txt" then
      return file, parameters
    end
	end
  
end