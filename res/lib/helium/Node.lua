local Dimensions = require "helium.Dimensions"

local Node = {}
Node.__index = Node

function Node.new(x, y)
	local self = {}
	self.x = x
	self.y = y
	self.outer = Dimensions.outer(self)
	self.inner = Dimensions.inner(self)
	self.tags = {"*"}
	self.style = {}
	self.nodes = {}
	return setmetatable(self, Node)
end

function Node:X()
	return (self.x or 0) + (self.parent and self.parent.inner:X() or 0)
end
function Node:Y()
	return (self.y or 0) + (self.parent and self.parent.inner:Y() or 0)
end

function Node:insert(node, i)
	node.parent = self
	table.insert(self.nodes, i or #self.nodes+1, node)
end

function Node:find(fn)
	if not self.nodes then return end
	for i, v in ipairs(self.nodes) do
		if fn(v, i) then return v, i end
	end
end

function Node:sibling(n)
	if not self.parent then return end
	local _, i = self.parent:find(function(x) return x == self end)
	return i and self.parent.nodes[i+n]
end

function Node:Style(style, node)
	node = node or self
	return self.style[style] or (self.parent and self.parent:Style(style, node))
end

function Node:draw(canvas)
	if self.drawself then self:drawself(canvas) end
	for _, node in ipairs(self.nodes) do
		node:draw(canvas)
	end
end

function Node:__tostring()
	return string.format("Node @ (%d,%d)", self.x, self.y)
end

return setmetatable(Node, {__call = function(_, ...) return Node.new(...) end})
