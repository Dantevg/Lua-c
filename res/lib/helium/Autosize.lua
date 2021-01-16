local Autosize = {}
Autosize.__index = Autosize

function Autosize.new(node, get)
	local self = {}
	self.node = node
	self.get = get
	return setmetatable(self, Autosize)
end

function Autosize:__call()
	return self:get()
end



Autosize.FitParent = {}
Autosize.FitParent.__index = Autosize.FitParent
Autosize.FitParent.__call = Autosize.__call

function Autosize.FitParent.new(node, get)
	local self = Autosize(node, get)
	return setmetatable(self, Autosize.FitParent)
end

function Autosize.FitParent.w(node)
	return Autosize.FitParent(node, Autosize.FitParent.W)
end
function Autosize.FitParent.h(node)
	return Autosize.FitParent(node, Autosize.FitParent.H)
end

function Autosize.FitParent:W()
	return self.node.parent
		and self.node.parent.inner:W() - (self.node.outer:X() - self.node.parent.inner:X())
		or 0
end
function Autosize.FitParent:H()
	return self.node.parent
		and self.node.parent.inner:H() - (self.node.outer:Y() - self.node.parent.inner:Y())
		or 0
end

setmetatable(Autosize.FitParent, {
	__index = Autosize,
	__call = function(_, ...) return Autosize.FitParent.new(...) end,
})



Autosize.FitContent = {}
Autosize.FitContent.__index = Autosize.FitContent
Autosize.FitContent.__call = Autosize.__call

function Autosize.FitContent.new(node, get)
	local self = Autosize(node, get)
	return setmetatable(self, Autosize.FitContent)
end

function Autosize.FitContent.w(node)
	return Autosize.FitContent(node, Autosize.FitContent.W)
end
function Autosize.FitContent.h(node)
	return Autosize.FitContent(node, Autosize.FitContent.H)
end

function Autosize.FitContent:W()
	if not self.node.nodes then return 0 end
	local max = 0
	for _, node in ipairs(self.node.nodes) do
		if node.W ~= Autosize.FitParent.W then
			max = math.max(max, node.outer:W())
		end
	end
	return max + 2*(self.node.padding or 0)
end
function Autosize.FitContent:H()
	if not self.node.nodes then return 0 end
	local max = 0
	for _, node in ipairs(self.node.nodes) do
		if node.W ~= Autosize.FitParent.H then
			max = math.max(max, node.outer:H())
		end
	end
	return max + 2*(self.node.padding or 0)
end

setmetatable(Autosize.FitContent, {
	__index = Autosize,
	__call = function(_, ...) return Autosize.FitContent.new(...) end,
})



return setmetatable(Autosize, {
	__call = function(_, ...) return Autosize.new(...) end,
})
