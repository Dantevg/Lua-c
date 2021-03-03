local Call = {}
Call.__index = Call

function Call.new(fn, parent)
	local self = {}
	self.fn = fn
	self.parent = parent
	self.time = 0
	self.selfTime = 0
	self.startTime = os.clock()
	self.calls = {}
	
	table.insert(fn.calls, self)
	if self.parent then table.insert(self.parent.calls, self) end
	
	return setmetatable(self, Call)
end

function Call:stop()
	if self.stopTime then error "Attempt to stop inactive call" end
	
	self.stopTime = os.clock()
	self.time = self.stopTime - self.startTime
	
	-- Calculate self-time
	local childTime = 0
	for i = 1, #self.calls do
		childTime = childTime + self.calls[i].time
	end
	self.selfTime = self.time - childTime
	
	return self.parent
end

function Call.__lt(a, b)
	if getmetatable(a) == Call and getmetatable(b) == Call then
		return a.time < b.time
	else
		return a < b
	end
end

setmetatable(Call, {
	__call = function(_, ...) return Call.new(...) end,
})



local Fn = {}
Fn.__index = Fn

Fn.sortBy = "nCalls"

function Fn.new(d)
	local self = {}
	self.d = d
	self.id = (d.name or "(no name)").." "..d.source..":"..d.linedefined
	self.calls = {}
	
	return setmetatable(self, Fn)
end

function Fn:getTime()
	if self.time then return self.time end -- Memoise
	
	self.time = 0
	for i = 1, #self.calls do
		self.time = self.time + self.calls[i].time
	end
	return self.time
end

function Fn:getSelfTime()
	if self.selfTime then return self.selfTime end -- Memoise
	
	self.selfTime = 0
	for i = 1, #self.calls do
		self.selfTime = self.selfTime + self.calls[i].selfTime
	end
	return self.selfTime
end

function Fn.__eq(a, b)
	return getmetatable(a) == Fn and getmetatable(b) == Fn and a.id == b.id
end

function Fn.__lt(a, b)
	if getmetatable(a) == Fn and getmetatable(b) == Fn then
		if Fn.sortBy == "selfTime" then
			return a:getTime() < b:getTime()
		elseif Fn.sortBy == "selfTime" then
			return a:getSelfTime() < b:getSelfTime()
		elseif Fn.sortBy == "nCalls" then
			return #a.calls < #b.calls
		end
	else
		return a < b
	end
end

function Fn:__tostring()
	return (self.d.name or "(no name)").."\t"..self.d.source..":"..self.d.linedefined
end

setmetatable(Fn, {
	__call = function(_, ...) return Fn.new(...) end,
})



local profile = {}

function profile.hook(type)
	local d = debug.getinfo(2)
	
	if d.func == profile.start or d.func == debug.sethook
		or d.func == profile.stop then return end
	
	if type == "call" then
		local fn = Fn(d)
		if profile.functions[fn.id] then
			fn = profile.functions[fn.id]
		else
			profile.functions[fn.id] = fn
		end
		
		profile.current = Call(fn, profile.current)
		profile.root = profile.root or profile.current
	elseif type == "return" and profile.current then
		profile.current = profile.current:stop()
	end
end

function profile.start()
	profile.functions = {}
	profile.root = nil
	profile.current = nil
	profile.sorted = {}
	profile.table = {}
	debug.sethook(profile.hook, "cr")
end

function profile.stop()
	debug.sethook() -- Stop debug hook
	
	while profile.current do
		profile.current = profile.current:stop()
	end
	
	profile.sorted = {}
	
	for _, f in pairs(profile.functions) do
		table.insert(profile.sorted, f)
	end
	
	-- Sort descending
	table.sort(profile.sorted, function(a, b) return b < a end)
	
	-- Make display table
	profile.table = {"Time\tSelf-time\t# Calls\tName\tSource"}
	for i = 1, #profile.sorted do
		local f = profile.sorted[i]
		table.insert(profile.table, string.format("%f\t%f\t%d\t%s",
			f:getTime(), f:getSelfTime(), #f.calls, tostring(f)))
	end
	require("tabularise")(profile.table)
	
	return profile.functions, profile.sorted
end

function profile.profile(fn, ...)
	profile.start()
	fn(...)
	profile.stop()
end

function profile.save(path)
	local file = io.open(path, "w")
	for _, line in ipairs(profile.table) do
		file:write(line.."\n")
	end
	file:close()
end

return setmetatable(profile, {
	__call = function(_, ...) return profile.profile(...) end,
})