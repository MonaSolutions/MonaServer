
function onConnection(client,...)
	
	INFO("Connection of a new client json")

	function client:onMessage(data)

    --print(data)
    --dataLua = mona:toJSON(data)
    --print(fromJSON(dataLua))
		return data
	end
end