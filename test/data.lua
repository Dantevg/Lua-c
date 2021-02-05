local data = require "data"
local value = require "value"

function data:map(fn)
	for i = 0, #self-1 do
		self[i] = fn(self, value.of(self[i])):get()
	end
	return self
end

function rot(n)
	return function(self, c)
		if c >= "A" and c <= "Z" then
			return (c-"A"+n) % 26 + "A"
		elseif c >= "a" and c <= "z" then
			return (c-"a"+n) % 26 + "a"
		else
			return c
		end
	end
end

local d = data.of("Hello, World!")
-- d:set(3, "abc")
print(d, #d)
print(d:map(rot(13)))

print(value.of(0x42) + 1)
