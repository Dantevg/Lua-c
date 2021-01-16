local Node = require "helium.Node"

local Text = {}
Text.__index = Text

function Text.new(x, y, text)
	local self = Node(x, y)
	table.insert(self.tags, 1, "Text")
	self.text = text
	return setmetatable(self, Text)
end

function Text:Text() return self.text or "" end
function Text:W() return #self:Text() end -- TODO: get bounding box
function Text:H() return 1 end            -- TODO: get bounding box

function Text:drawself(canvas)
	canvas:colour(table.unpack(self:Style("colour")))
	canvas:write(self:Text(), self:X(), self:Y())
end

function Text:__tostring()
	return string.format("Text @ (%d,%d) %q", self.x, self.y, self.text)
end

return setmetatable(Text, {
	__index = Node,
	__call = function(_, ...) return Text.new(...) end,
})