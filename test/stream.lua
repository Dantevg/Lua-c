local stream = require "stream"
local value = require "value"
local op = require "operator"
local base64 = require "base64"

function rot(n)
	return function(c)
		local x = value.of(c)
		if x >= "A" and x <= "Z" then
			return (x-"A"+n) % 26 + "A"
		elseif x >= "a" and x <= "z" then
			return (x-"a"+n) % 26 + "a"
		else
			return x
		end
	end
end

-- for x in stream(math.random):filter(stream.op.gt(0.5)):take(3) do
-- 	print(x)
-- end

-- for x in stream("Hello, World!"):group(stream.util.match("%l")):map(stream.string) do
-- 	print(x)
-- end

-- Rot13
print(stream("Hello, World!"):map(rot(13)):string())

-- All alphabetic characters
local alpha = stream.from(32)
	:map(string.char)
	:filter(stream.util.match("%a"))
	:take(52)
	:table()
print(table.concat(alpha))

-- Fibonacci
local fib = stream.iterate({1,1}, function(t) return {t[2], t[1] + t[2]} end)
	:map(function(t) return t[1] end):take(10):table()
print(table.concat(fib, ", "))

-- Base64
-- local enc = stream{250,251,252,253}:base64encode():string()
local enc = stream("Lorem ipsum dolor sit amet, consectetur adipiscing elit")
	:base64encode(true):string()
local dec = stream(enc):base64decode(true):string()
print(enc)
print(dec)

-- Random hex
local hex = stream(math.random)
	:mapRange(0,1,1,16)
	:mapIndex("0123456789abcdef")
	:take(5)
	:string()
print(hex)

-- String parsing (pretty long code in comparison to plain Lua)
local t = stream("hello=world,foo=bar")
	:splitAt(",")
	:map(function(x)
		return x
			:splitAt("=")
			:map(stream.string)
			:table()
	end)
	:table()

for _, v in ipairs(t) do
	print(table.concat(v, ", "))
end
