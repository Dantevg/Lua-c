local he = require "helium"

local Box = {}



function Box.new(x, y, w, h, colour)
	local self = setmetatable({}, {
		__name = "Box",
		__index = function(t, k)
			return Box[k] or (rawget(t, "parent") and t.parent[k])
		end,
	})
	self.type = "box"
	self.objects = {}
	self.x = x or 0
	self.y = y or 0
	self.w = w or 1
	self.h = h or 1
	self.colour = colour or {255}
	
	return self
end

function Box:draw()
	self.screen:colour(table.unpack(self.colour))
	self.screen:rect(self.x, self.y, self.w, self.h)
end



return setmetatable(Box, {
	__call = function(_, ...) return Box.new(...) end,
})