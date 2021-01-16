local Box = require "helium.Box"

local Square = {}
Square.__index = Square

function Square.new(x, y, size, name)
	local self = Box(x, y, size, size)
	self.size = size
	self.name = name
	return setmetatable(self, Square)
end

function Square:Size() return self.size end
function Square:W() return self:Size() end
function Square:H() return self:Size() end

function Square:__tostring()
	return string.format("Square @ (%d,%d) %dx%d", self.x, self.y, self.size, self.size)
end

return setmetatable(Square, {
	__index = Box,
	__call = function(_, ...) return Square.new(...) end,
})
