child = children("subdir")

function run()
	testInitialization()
	testNumber()
	testArray()
	testHash()
	testSubdir()
	
	assert(#data == 1)
end

function testInitialization()
	-- note that if Databases fail you should manually remove files created
	
	assert(data.init == true)
	assert(#data == 1)
	assert(data ~= nil) -- check if data is always present
	
	assert(data.subdir.init == true)
	assert(#data.subdir == 1)
end

function testNumber()
	
	data.number = 10
	assert(data.number ~= 10)
	assert(data.number == "10")
	assert(data["number"] == "10")
	data.number = data.number + 1
	assert(data.number == "11")
	assert(#data == 2)
	data.number = nil
end

function testArray()
	
	data.table = nil
	data.table = {1,2,3,4,5}
	local index = 1
	assert(data.table["1"] == "1")
	while index <= #data.table do
		assert(data.table[index] == ""..index)
		index = index + 1
	end
	data.table = nil
	assert(data["table"] == nil)
	assert(data.table == nil)
end

function testHash()
	
	data.hash = nil
	data.hash = {a="one",b="two",c={c1="three"}}
	assert(data.hash.a == "one")
	assert(data.hash["a"] == "one")
	assert(data.hash.b == "two")
	assert(data.hash["b"] == "two")
	assert(data.hash.c.c1 == "three")
	assert(data.hash.c["c1"] == "three")
	assert(data.hash["c"].c1 == "three")
	assert(data.hash["c"]["c1"] == "three")
	data.hash = nil
	assert(data.hash == nil)
end

function testSubdir()
	
	child.testNumber()
	assert(data.subdir.number == "12")
	assert(#data.subdir == 2)
	data.subdir.number = nil
	assert(data.subdir.number == nil)
	assert(#data.subdir == 1)
end
