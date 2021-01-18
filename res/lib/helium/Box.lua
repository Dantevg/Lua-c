local Node = require "helium.Node"

local Box = {}
Box.__index = Box

function Box.new(x, y, w, h)
	local self = Node(x, y)
	table.insert(self.tags, 1, "Box")
	self.w = w
	self.h = h
	return setmetatable(self, Box)
end

function Box:W() return self.w or 0 end
function Box:H() return self.h or 0 end

function Box:drawself(canvas)
	canvas:colour(table.unpack(self:Style("colour")))
	canvas:rect(self:X(), self:Y(), self:W(), self:H(), true)
end

function Box:__tostring()
	return string.format("Box @ (%d,%d) %dx%d", self.x, self.y, self.w, self.h)
end

return setmetatable(Box, {
	__index = Node,
	__call = function(_, ...) return Box.new(...) end,
})
