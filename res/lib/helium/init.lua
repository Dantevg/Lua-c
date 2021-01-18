--[[
	
	Helium (He) Framework
	by RedPolygon
	
	for MoonBox-C
	
]]--

local Box = require "helium.Box"

local He = {}
He._VERSION = "0.5"

He.__index = He

-- Wraps the value in a function which returns that value
function He.proxy(value)
	return function() return value end
end

function He.new(w, h)
	local self = Box(0, 0, w, h)
	self.style = setmetatable({}, {__index = He.style})
	return setmetatable(self, He)
end

function He:Style(style, node)
	for _, tag in ipairs(node.tags or {"*"}) do
		if self.style[tag] and self.style[tag][style] then
			local style = self.style[tag][style](node)
			if style then return style end
		end
	end
end

function He:drawself(canvas) end -- Do not draw root node

function He:__tostring()
	return string.format("Helium Root %dx%d", self.w, self.h)
end

-- Default style
He.style = {}
He.style["*"] = {
	colour = He.proxy {255, 255, 255}
}

return setmetatable(He, {
	__index = Box,
	__call = function(_, ...) return He.new(...) end,
})
