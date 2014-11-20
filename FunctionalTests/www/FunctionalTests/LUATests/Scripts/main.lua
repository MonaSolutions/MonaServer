local tab = {10,15,20,25}

function run()
	
	-- pairs & ipairs
	local i = 1
	for k,v in pairs(tab) do
		assert(i < 5)
		i = i + 1
	end
	
	for i,v in ipairs(tab) do
		assert(i < 5)
	end
end