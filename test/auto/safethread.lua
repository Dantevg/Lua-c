local thread = require "safethread"

do
	-- Function argument and return value order
	local t = thread.new(function(x, y) return x, y, x - y end, 42, 10)
	local a, b, c = t:wait()
	assert(a == 42)
	assert(b == 10)
	assert(c == 32)
end

do
	-- Function environment
	local t = thread.new(function() x = 10 end)
	local y = 20
	z = 30
	local _, a, b, c = t:pcall(function() return x, y, z end)
	assert(a == 10)
	assert(b == 20)
	assert(c == nil)
	t:wait()
end

do
	-- Recursive references
	local t = thread.new(function() end)
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
