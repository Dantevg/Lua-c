local consolecolours = require "consolecolours"

local prettyprint = {}

local nop = tostring
local function pretty(x)
	local t = type(x)
	return prettyprint[t] and prettyprint[t](x) or nop(x)
end
local function prettykey(x)
	if type(x) == "string" and x:match("^[%a_][%w_]*$") then
		return tostring(x)
	else
		return "["..pretty(x).."]"
	end
end

prettyprint["nil"] = function(x) return consolecolours.fg.grey..nop(x)..consolecolours.reset end
prettyprint["number"] = function(x) return consolecolours.fg.blue..nop(x)..consolecolours.reset end
prettyprint["string"] = function(x) return consolecolours.fg.green..'"'..x..'"'..consolecolours.reset end
prettyprint["boolean"] = function(x) return consolecolours.fg.yellow..nop(x)..consolecolours.reset end
prettyprint["table"] = function(x)
	local contents = {}
	
	for i, v in ipairs(x) do
		table.insert(contents, pretty(v))
	end
	
	for k, v in pairs(x) do
		if type(k) ~= "number" then
			table.insert(contents, prettykey(k).." = "..pretty(v))
		end
	end
	
	return "{ "..table.concat(contents, ", ").." }"
end
prettyprint["function"] = nop
prettyprint["thread"] = nop
prettyprint["userdata"] = nop

local function eval(input)
	local fn1, err1 = load(input, "@stdin", "t")
	local fn2, err2 = load("return "..input, "@stdin", "t")
	
	return (fn2 or fn1), err1
end

local function result(success, ...)
	local t = {...}
	if not success then
		print(t[1])
	else
		for i = 1, select("#", ...) do
			print(pretty(t[i]))
		end
	end
end

while true do
	io.write(consolecolours.reset, "> ")
	local input = io.read()
	if not input then print() break end
	local fn, err = eval(input)
	if not fn then
		print(err)
	else
		result(pcall(fn))
	end
end
