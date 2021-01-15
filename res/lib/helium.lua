--[[
	
	Helium (He) Framework
	by RedPolygon
	
	for MoonBox-C
	
]]--

local Binding = require "Binding"
local tableutil = require "tableutil"

local he = {}
he._VERSION = "0.4"



-- METATABLE

he.mt = {}
he.mt.__index = he
he.mt.__name = "Helium"



-- HELIUM FRAMEWORK

function he:append(object)
	table.insert(self.objects, object)
	object.parent = self
end

function he:hasTag(tag)
	return tableutil.contains(self, tag)
end

function he:find(fn, deep)
	for i, obj in ipairs(self.objects) do
		if fn(obj, i) then return obj end
	end
	-- Didn't find directly
	if deep then
		for i, obj in ipairs(self.objects) do
			local founddeep = obj:find(fn, deep)
			if founddeep then return founddeep end
		end
	end
end

function he:findAll(fn, deep)
	local found = {}
	for i, obj in ipairs(self.objects) do
		if fn(obj, i) then table.insert(found, obj) end
		if deep then
			tableutil.concat(found, obj:findAll(fn, deep))
		end
	end
	return found
end

function he:findByType(type, all, deep)
	if all then
		return self:findAll(function(obj) return obj.type == type end, deep)
	else
		return self:find(function(obj) return obj.type == type end, deep)
	end
end

function he:findByTag(tag, all, deep)
	if all then
		return self:findAll(function(obj) return obj:hasTag(tag) end, deep)
	else
		return self:find(function(obj) return obj:hasTag(tag) end, deep)
	end
end

function he:draw()
	for _, v in ipairs(self.objects) do
		v:draw()
	end
end

function he.new(screen)
	if not screen then error("Expected screen") end
	
	local self = setmetatable({}, he.mt)
	self.type = "root"
	self.objects = {}
	self.screen = screen
	
	return setmetatable(self, he.mt)
end



-- RETURN

return he
