local data = require "data"

function data:map(fn)
	for i = 0, #self-1 do
		self[i] = fn(self, self[i])
	end
	return self
end

function rot(n)
	return function(self, c)
		if c >= string.byte("A") and c <= string.byte("Z") then
			return (c-string.byte("A")+n) % 26 + string.byte("A")
		elseif c >= string.byte("a") and c <= string.byte("z") then
			return (c-string.byte("a")+n) % 26 + string.byte("a")
		else
			return c
		end
	end
end

local d = data.of("Hello, World!")
d:set(3, "abc")
print(d, #d)
print(d:map(rot(13)))