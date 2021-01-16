local Node = require "helium.Node"
local Box = require "helium.Box"
local Autopos = require "helium.Autopos"

local List = {}
List.__index = List

function List.new(x, y, w, h)
	local self = Box(x, y, w, h)
	table.insert(self.tags, 1, "List")
	return setmetatable(self, List)
end

function List:insert(node, i)
	Node.insert(self, node, i)
	node.X = Autopos.vert.x(node)
	node.Y = Autopos.vert.y(node)
end

function List:__tostring()
	return string.format("List @ (%d,%d) %d nodes", self.x, self.y, #self.nodes)
end

return setmetatable(List, {
	__index = Box,
	__call = function(_, ...) return List.new(...) end,
})
