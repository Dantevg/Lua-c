local data = require "data"

function data:rot(n)
	for i = 0, #self-1 do
		if self[i] >= string.byte("A") and self[i] <= string.byte("Z") then
			self:set(i, (self[i]-string.byte("A")+n) % 26 + string.byte("A") )
		elseif self[i] >= string.byte("a") and self[i] <= string.byte("z") then
			self:set(i, (self[i]-string.byte("a")+n) % 26 + string.byte("a") )
		end
	end
	return self
end

local d = data.of("Hello, World!")
d:set(3, "abc")
print(d, #d)
print(d:rot(13))