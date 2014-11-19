
function testNumber()
	
	data.number = 12
	assert(data.number == "12")
	assert(data["number"] == "12")
	assert(#data == 2)
end


