local Dimensions = {}

function Dimensions.new(node)
	local self = {}
	self.node = node
	return setmetatable(self, Dimensions)
end



Dimensions.outer = {}
Dimensions.outer.__index = Dimensions.outer

function Dimensions.outer.new(node)
	local self = Dimensions.new(node)
	return setmetatable(self, Dimensions.outer)
end

function Dimensions.outer:X()
	return self.node.X and self.node:X() or 0
end
function Dimensions.outer:Y()
	return self.node.Y and self.node:Y() or 0
end
function Dimensions.outer:W()
	return self.node.W and self.node:W() or 0
end
function Dimensions.outer:H()
	return self.node.H and self.node:H() or 0
end

setmetatable(Dimensions.outer, {
	__index = Dimensions,
	__call = function(_, ...) return Dimensions.outer.new(...) end,
})



Dimensions.inner = {}
Dimensions.inner.__index = Dimensions.inner

function Dimensions.inner.new(node)
	local self = Dimensions.new(node)
	return setmetatable(self, Dimensions.inner)
end

function Dimensions.inner:X()
	return (self.node.outer and self.node.outer:X() or 0) + (self.node.padding or 0)
end
function Dimensions.inner:Y()
	return (self.node.outer and self.node.outer:Y() or 0) + (self.node.padding or 0)
end
function Dimensions.inner:W()
	return (self.node.outer and self.node.outer:W() or 0) - 2*(self.node.padding or 0)
end
function Dimensions.inner:H()
	return (self.node.outer and self.node.outer:H() or 0) - 2*(self.node.padding or 0)
end

setmetatable(Dimensions.inner, {
	__index = Dimensions,
	__call = function(_, ...) return Dimensions.inner.new(...) end,
})



return setmetatable(Dimensions, {
	__call = function(_, ...) return Dimensions.new(...) end,
})
