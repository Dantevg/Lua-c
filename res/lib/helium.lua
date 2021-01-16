--[[
	
	Helium (He) Framework
	by RedPolygon
	
	for MoonBox-C
	
]]--

local Box = require "helium.Box"

local He = {}
He._VERSION = "0.5"

He.__index = He

function He.new(w, h)
	local self = {}
	self.root = Box(0, 0, w, h)
	self.root.drawself = function() end -- Don't draw root element itself
	self.style = {}
	return setmetatable(self, {__index = self.root})
end

return setmetatable(He, {
	__call = function(_, ...) return He.new(...) end,
})