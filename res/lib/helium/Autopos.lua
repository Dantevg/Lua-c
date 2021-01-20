local Autopos = {}
Autopos.__index = Autopos

function Autopos.new(node, get)
	local self = {}
	self.node = node
	self.get = get
	return setmetatable(self, Autopos)
end

function Autopos:__call()
	return self:get()
end



Autopos.vert = {}
Autopos.vert.__index = Autopos.vert
Autopos.vert.__call = Autopos.__call

function Autopos.vert.new(node, get)
	local self = Autopos(node, get)
	return setmetatable(self, Autopos.vert)
end

function Autopos.vert.x(node)
	return Autopos.vert(node, Autopos.vert.X)
end
function Autopos.vert.y(node)
	return Autopos.vert(node, Autopos.vert.Y)
end

function Autopos.vert:X()
	local prev = self.node:sibling(-1)
	return prev and prev.outer:X() + (self.node.x or 0)
		or (self.node.parent and self.node.parent.inner:X() + (self.node.x or 0) or 0)
end
function Autopos.vert:Y()
	local prev = self.node:sibling(-1)
	return prev and prev.outer:Y() + prev.outer:H() + (self.node.y or 0)
		or (self.node.parent and self.node.parent.inner:Y() + (self.node.y or 0) or 0)
end

setmetatable(Autopos.vert, {
	__index = Autopos,
	__call = function(_, ...) return Autopos.vert.new(...) end,
})



Autopos.hor = {}
Autopos.hor.__index = Autopos.hor
Autopos.hor.__call = Autopos.__call

function Autopos.hor.new(node, get)
	local self = Autopos(node, get)
	return setmetatable(self, Autopos.hor)
end

function Autopos.hor.x(node)
	return Autopos.hor(node, Autopos.hor.X)
end
function Autopos.hor.y(node)
	return Autopos.hor(node, Autopos.hor.Y)
end

function Autopos.hor:X()
	local prev = self.node:sibling(-1)
	return prev and prev.outer:X() + prev.outer:W() + (self.node.x or 0)
		or (self.node.parent and self.node.parent.inner:X() + self.node.x or 0)
end
function Autopos.hor:Y()
	local prev = self.node:sibling(-1)
	return prev and prev.outer:Y() + (self.node.y or 0)
		or (self.node.parent and self.node.parent.inner:Y() + (self.node.y or 0) or 0)
end

setmetatable(Autopos.hor, {
	__index = Autopos,
	__call = function(_, ...) return Autopos.hor.new(...) end,
})



return setmetatable(Autopos, {
	__call = function(_, ...) return Autopos.new(...) end,
})
