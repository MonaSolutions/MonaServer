local _dateTime = {__time=1412082917000}
local _bytes = {__raw="\xFF\x00\x14"}
local _array = {2.7,true,{__type="test",key="val"}}
local _map =  { map="map", __size=1 }
setmetatable(_map,{__mode="v"})
local _mixed =    { prop="prop", "elt1" }
local _notmixed = { { prop="prop" }, "elt1" }

local Object = {										-- object
	key1 = "value1",  									-- string
	key2 = 2.7,											-- number
	key3 = true,										-- boolean
	key4 = _dateTime,									-- date
	key5 = _bytes,  									-- bytes
	key6 = _array,										-- array
	key7 = _map,										-- map
	key8 = { "value", _dateTime, _bytes, _array, _map } -- repeated
}

function Values(cyclic,mixed)
	if cyclic then 
		Object.cyclic = Object
	else
		Object.cyclic = nil
	end
	if mixed then 
		Object.mixed = _mixed
		Object.notmixed = nil
	else
		Object.mixed = nil
		Object.notmixed = _mixed
	end
	return 3.7, Object, nil, "hello"
end

function check(...)
	local args = table.pack(...)
	if #args~=4 then error(#args.." different of 4") end
	if args[1]~=3.7 then error(args[1].." different of 3.7") end
	compareTable(Object,args[2],_notmixed)
	if args[3]~=nil then error(args[3].." is not nil") end
	if args[4]~="hello" then error(args[4].." different of 'hello'") end
end


function AMF()
	check(mona:fromAMF(mona:toAMF(Values(true,true))))
end

function AMF0()
	check(mona:fromAMF(mona:toAMF0(Values(true,true))))
end

function JSON()
	local json = mona:toJSON()
	assert(json=="[]")
	assert(mona:fromJSON(json)==nil)
	
	json = mona:toJSON({})
	assert(json=="[[]]")
	compareTable(mona:fromJSON(json),{})
	
	json = mona:toJSON({{}})
	assert(json=="[[[]]]")
	compareTable(mona:fromJSON(json),{{}})
	
	check(mona:fromJSON( mona:toJSON(Values(false,false))))
end

function XMLRPC()
	local xml = mona:toXMLRPC()
	assert(xml=="<?xml version=\"1.0\"?><methodResponse><params></params></methodResponse>")
	assert(mona:fromXMLRPC(xml)==nil)

	check(mona:fromXMLRPC(mona:toXMLRPC(Values(false,false))))
end

function run()
	return AMF,AMF0,JSON,XMLRPC 
end