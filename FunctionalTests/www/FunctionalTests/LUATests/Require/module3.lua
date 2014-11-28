module("module3", package.seeall)  -- optionally omitting package.seeall if desired

-- private
local loaded3 = true

function isLoaded()
  return loaded3
end