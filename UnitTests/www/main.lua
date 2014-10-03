
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


function runTest(test)
	local start = mona:time()
	local args = table.pack(pcall(test.run))
	if args[1] then
		table.remove(args,1)
		for i,f in pairs(args) do
			if type(f) == "function" then
				for k,v in pairs(test) do
					if v==f then
						local start2 = mona:time()
						ok,err = pcall(f)
						if ok then
							INFO(test.name,":",k," OK (",mona:time()-start2,"ms)")
						else
							ERROR(test.name,":",k," test failed, ",err)
						end
					end
					if err then break end
				end
			end
			if err then break end
		end
		if not err then
			NOTE(test.name," OK (",mona:time()-start,"ms)")
			return true
		end
	else
		err = args[2]
	end
	ERROR(test.name," test failed, ",err)
	return false
end


local index=1
local tests = {}
local test = nil

for _,path in pairs(mona:listPaths()) do
	if path.isDirectory then
		local child = children(path.name)
		if child and child.run then
			if mona.arguments.module == child.name then
				test = child
				break
			end
			tests[index] = child
			index = index+1
		end
	end
end

if mona.arguments.module and not test then WARN("Test module ",mona.arguments.module," not found") end

print("UnitTests Application Server")

if not test or #tests>1 then
	for index,test in pairs(tests) do
		io.write(index," - ",test.name,"\n")
	end
	print("Choose the index of the test to run (or type enter to run all) :")

	index = io.read("*line")
	if not index then return end
	if #index>0 then
		test = tests[tonumber(index)]
		if not test then ERROR("index ",index," out of acceptable proposition") end
	end
	if not test then INFO("All the tests will run") end
end

if not test then
	for _,test in pairs(tests) do
		if not runTest(test) then break end
	end
else
	runTest(test)
end