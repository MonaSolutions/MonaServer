-- module.lua
local M = {} -- public interface
-- private
local loaded = true

function M.isLoaded()
  return loaded
end

return M