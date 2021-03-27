local Thread = require "safethread"

do
	-- Function argument and return value order
	local t = Thread()
	local _, a, b, c = t:pcall(function(x, y) return x, y, x - y end, 42, 10)
	assert(a == 42)
	assert(b == 10)
	assert(c == 32)
	t:wait()
end

do
	-- Function environment
	local t = Thread(function() x = 10 end)
	local y = 20
	z = 30
	local _, a, b, c = t:pcall(function() return x, y, z end)
	assert(a == 10)
	assert(b == 20)
	assert(c == nil)
	t:wait()
end

do
	-- More environment stuff
	local t = Thread(function() x = 10 end)
	local y = 20
	z = 30
	local _, a, b, c = t:pcall(function()
		x, y, z = (x or 0)+1, (y or 0)+1, (z or 0)+1
		return x, y, z
	end)
	assert(x == nil)
	assert(a == 11)
	assert(y == 20)
	assert(b == 21)
	assert(z == 30)
	assert(c == 1)
	t:wait()
end

do
	-- Recursive references
	local t = Thread()
	function f(tbl)
		assert(tbl.t == tbl.t.t)
		assert(tbl.t == tbl.f())
		assert(tbl.f == tbl.f().f)
		return tbl
	end
	local tbl; tbl = {f = function() return tbl end}
	tbl.t = tbl
	f(tbl)
	f(select(2, t:pcall(f, tbl)))
	t:wait()
end

do
	-- Async functions, return values
	local t = Thread(function() x = 42 return 10, 20 end)
	t:async(function(y) return x - y end, 10)(function(a) assert(a == 32) end)
	local a, b = t:wait()
	assert(a == 10)
	assert(b == 20)
end
