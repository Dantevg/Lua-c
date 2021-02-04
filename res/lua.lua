local console = require "console"

local prettyprint = {}

local resultstyle = console.reset..console.bright
local nop = function(x) return resultstyle..tostring(x) end
local function pretty(x, ...)
	local t = type(x)
	return prettyprint[t] and prettyprint[t](x, ...)..resultstyle or nop(x)..resultstyle
end
local function prettykey(x)
	if type(x) == "string" and x:match("^[%a_][%w_]*$") then
		return tostring(x)
	else
		return "["..pretty(x).."]"
	end
end

prettyprint["nil"] = function(x) return console.reset..console.fg.grey..tostring(x) end
prettyprint["number"] = function(x) return console.reset..console.fg.cyan..tostring(x) end
prettyprint["string"] = function(x) return console.reset..console.fg.green..'"'..x..'"' end
prettyprint["boolean"] = function(x) return console.reset..console.fg.yellow..tostring(x) end
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
	
	return resultstyle.."{ "..table.concat(contents, ", ").." }"
		..(getmetatable(x) and console.fg.grey.." + mt" or "")
end
prettyprint["function"] = function(x, long)
	local d = debug.getinfo(x, "S")
	local filename = d.short_src:match("([^/]+)$")
	local str = string.format("%s[%s @ %s:%d]",
		console.bright..console.fg.magenta, tostring(x), filename,
		d.linedefined)
		
	if not long or d.source:sub(1,1) ~= "@" or d.source == "@stdin" then
		return str
	end
	
	local file = io.open(d.short_src)
	local contents = {}
	local i = 1
	for line in file:lines() do
		if i >= d.linedefined then
			if i > d.lastlinedefined then break end
			table.insert(contents, line)
		end
		i = i+1
	end
	file:close()
	return str..resultstyle.."\n"..table.concat(contents, "\n")
end
prettyprint["thread"] = nop
prettyprint["userdata"] = nop

prettyprint["error"] = function(x)
	return console.bright..console.fg.red..tostring(x)..resultstyle
end

local function onerror(err)
	print(debug.traceback(prettyprint.error(err), 2))
end

local function result(success, ...)
	local t = {...}
	if success then -- Error case has already been handled by onerror via xpcall
		for i = 1, select("#", ...) do
			print(pretty(t[i], true))
		end
	end
end

local function multiline(input)
	local fn, err = load(input, "=stdin", "t")
	while not fn do
		io.write(console.reset, "... ")
		local newinput = io.read()
		if not newinput then print() os.exit() end
		input = input.."\n"..newinput
		fn, err = load(input, "=stdin", "t")
	end
	return fn, err
end

local function repl()
	io.write(console.reset, "> ")
	local input = io.read()
	if not input then print() os.exit() end
	
	local fn, err = load("return "..input, "=stdin", "t")
	if not fn then
		fn, err = multiline(input)
	end
	return fn, err
end

while true do
	local fn, err = repl()
	if not fn then
		print(prettyprint.error(err))
	else
		result(xpcall(fn, onerror))
	end
end
