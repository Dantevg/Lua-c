local he = require "helium"

local Text = {}



function Text.new(text, x, y, colour)
	local self = setmetatable({}, {
		__name = "Text",
		__index = function(t, k)
			return Text[k] or (rawget(t, "parent") and t.parent[k])
		end,
	})
	self.type = "text"
	self.objects = {}
	self.text = text or ""
	self.x = x or 0
	self.y = y or 0
	self.colour = colour or {255}
	
	return self
end

function Text:draw()
	self.screen:colour(table.unpack(self.colour))
	self.screen:write(self.text, self.x, self.y)
end



return setmetatable(Text, {
	__call = function(_, ...) return Text.new(...) end,
})