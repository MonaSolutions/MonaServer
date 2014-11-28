local module1 = dofile("module1.lua")
local module2 = dofile("module2.lua")
local module1bis = require("module1.lua")
local module2bis = require("module2.lua")
require("module3.lua")
local file4 = loadfile("module4.lua")

function run()
	
	-- test dofile (from parent and from subdir)
	assert(module1.isLoaded())
	assert(module2.isLoaded())
	
	-- test require (from parent and from subdir)
	assert(module3.isLoaded())
	assert(module1bis.isLoaded())
	assert(module2bis.isLoaded())
	
	-- test loadfile (from parent)
	result, module4 = pcall(file4)
	assert(result)
	assert(module4.isLoaded())
end
