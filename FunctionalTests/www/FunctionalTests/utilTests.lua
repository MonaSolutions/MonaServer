
function printTable(t,...)

  local args = table.pack(...)
  table.remove(args,1)
  io.write(table.unpack(args))
  print("{")
 
  for k,v in pairs(t) do
   	if type(v) == "table" then
		if type(k) == "string" then
			io.write(...)
			print(k,"=")
		end
		printTable(v,"    ",...)
	else
		io.write(...)
		if type(k) == "string" then io.write(k,"=") end
		print(v)
	end
  end
  
  io.write(...)
  print("}")
end


function compareTable(Table1,Table2,notmixed)
	for key,value1 in pairs(Table1) do
		if type(key) ~= "table" then
			local value2 = Table2[key]
			local type1 = type(value1)
			
			if type1 ~= type(value2) then error(type1.." type different of "..type(value2).." type") end
		
			if type1 == "table" then
				if notmixed and key=="notmixed" then
					compareTable(notmixed,value2)
				elseif key~="cyclic" then -- to avoid infinite loop
					compareTable(value1,value2)
				end 
			else
				if type1 == "function" or type1 == "userdata" or type1 == "thread" or type1 == "lightuserdata" then error(type1.." type non serializable") end
				-- can be now LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING
				if value1~=value2 then error(value1.." value different of "..value2.." value") end
			end
		end
	end
end
