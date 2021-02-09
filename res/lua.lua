local console = require "console"

local trace = {}
local dotrace = false

local env = {}



-- Autocompletion modified from https://github.com/Dantevg/MoonBox/blob/master/rom/lib/luasyntax.lua#L67
local keywords = {
	"and", "break", "do", "else", "elseif",
	"end", "false", "for", "function", "if",
	"in", "local", "nil", "not", "or",
	"repeat", "return", "then", "true", "until", "while",
}

local function keywordcomplete(name, completions)
	for _, keyword in ipairs(keywords) do
		if keyword:sub( 1, #name )  == name then
			table.insert(completions, keyword:sub(#name + 1))
		end
	end
end

local function containercomplete(t, name, before, after, completions)
	while t do
		for k, v in pairs(t) do
			if type(k) == "string" and k:sub( 1, #name ) == name and (after ~= "" or k:match("[%a_][%w_]*")) then
				local after = after..(type(v) == "table" and "." or (type(v) == "function" and "(" or ""))
				table.insert(completions, before..k:sub(#name + 1)..after)
			end
		end
		t = getmetatable(t) and getmetatable(t).__index or nil
		if type(t) ~= "table" then break end
	end
end

local function autocomplete(input)
	local path = {}
	
	local from, to, match = input:find("^([%a_][%w_]*)")
	while input and to do
		table.insert(path, match)
		input = input:sub(to+1)
		from, to, match = input:find("^%.([%a_][%w_]*)")
		if not from then
			from, to, match = input:find("%['([^']*)'%]")
		end
		if not from then
			from, to, match = input:find('%["([^"]*)"%]')
		end
	end
	
	-- Last part
	from, to, match = input:find("^%.")
	local before, after = "", ""
	if not from then
		from, to, match = input:find("%['([^']*)$")
		before, after = "", "']"
	end
	if not from then
		from, to, match = input:find('%["([^"]*)$')
		before, after = "", '"]'
	end
	if not from then
		from, to, match = input:find('%[')
		before, after = '"', '"]'
	end
	if from then
		table.insert(path, match or "")
	else
		before, after = "", ""
	end
	
	-- Get container
	local t = _G
	for i = 1, #path-1 do
		if type(t) == "table" and t[ path[i] ] then
			t = t[ path[i] ]
		else
			return {}
		end
	end
	
	-- Match
	local completions = {}
	containercomplete(t, path[#path], before, after, completions)
	
	if #path == 1 then
		keywordcomplete(path[#path], completions)
	end
	
	return completions
end



local prettyprint = {}

local resultstyle = console.reset..console.bright
local function nop(x) return resultstyle..tostring(x) end
local function special(x)
	return string.format("%s[%s]",
		console.bright..console.fg.magenta, tostring(x))
end
local function pretty(x, long)
	local t = type(x)
	
	if prettyprint[t] and not (getmetatable(x) or {}).__tostring then
		return prettyprint[t](x, long)..resultstyle
	else
		return nop(x)..resultstyle
	end
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
prettyprint["table"] = function(x, long)
	if type(x) ~= "table" and not (getmetatable(x) or {}).__pairs then
		return prettyprint.error("not a table")
	end
	if not long then return special(x) end
	
	local contents = {}
	
	if type(x) == "table" then
		for _, v in ipairs(x) do
			table.insert(contents, pretty(v))
		end
	end
	
	for k, v in pairs(x) do
		if not contents[k] then
			table.insert(contents, prettykey(k).." = "..pretty(v))
		end
	end
	
	return resultstyle.."{ "..table.concat(contents, ", ").." }"
		..(getmetatable(x) and console.fg.grey.." + mt" or "")
end
prettyprint["function"] = function(x, long)
	if type(x) ~= "function" then return prettyprint.error("not a function") end
	local d = debug.getinfo(x, "S")
	local filename = d.short_src:match("([^/]+)$")
	local str = string.format("%s[%s @ %s:%d]",
		console.bright..console.fg.magenta, tostring(x), filename, d.linedefined)
		
	if not long or d.source:sub(1,1) ~= "@" or d.source == "@stdin" then
		return str
	end
	
	local file = io.open(d.source:sub(2))
	local contents = {}
	local i = 1
	for line in file:lines() do
		if i >= d.linedefined then
			if i > d.linedefined then break end
			table.insert(contents, line)
		end
		i = i+1
	end
	file:close()
	return str..resultstyle.."\n"..table.concat(contents, "\n")
end
prettyprint["thread"] = special
prettyprint["userdata"] = special

prettyprint["error"] = function(x)
	return console.bright..console.fg.red..tostring(x)
end

prettyprint["trace"] = function(x)
	return string.format("%s (%s%s%s) %s",
		x.name or x.source, console.fg.cyan, x.namewhat, resultstyle,
		prettyprint["function"](x.func))
end



local commands = {}

function commands.table(arg)
	local tbl = _G[arg]
	if not tbl then return end
	print(prettyprint.table(tbl, true))
end

function commands.metatable(arg)
	local mt = getmetatable(_G[arg])
	if not mt then return end
	print(prettyprint.table(mt, true))
end

commands["function"] = function(arg)
	local fn = _G[arg]
	if not fn then return end
	print(prettyprint["function"](fn, true))
end

function commands.require(arg)
	local has = package.loaded[arg]
	if has then package.loaded[arg] = nil end
	local success, result = pcall(require, arg)
	if success then
		_G[arg] = result
	else
		print(prettyprint.error(result))
	end
end

function commands.trace(arg)
	if arg == "start" or arg == "on" then
		dotrace = true
	elseif arg == "stop"  or arg == "off" then
		dotrace = false
	end
end

function commands.use(arg)
	local tbl = _G[arg]
	if tbl then
		setmetatable(env, {__index = tbl})
	else
		setmetatable(env, nil)
	end
end

function commands.complete(arg)
	local completions = autocomplete(arg or "")
	for _, completion in ipairs(completions) do
		print(console.fg.grey..arg..resultstyle..completion)
	end
end

function commands.help()
	print("Available commands:")
	print("  :table, :t <table>")
	print("  :metatable, :mt <object>")
	print("  :function, :fn, :f <function>")
	print("  :require <modulename>")
	print("  :trace <on|start|off|stop>")
	print("  :use [table]")
	print("  :complete <part>")
	print("  :help, :?")
	print("  :exit, :quit, :q")
end

function commands.exit()
	os.exit()
end

commands.t = commands.table
commands.mt = commands.metatable
commands.fn = commands["function"]
commands.f = commands["function"]
commands.m = commands.require
commands["?"] = commands.help
commands.quit = commands.exit
commands.q = commands.exit



local function onerror(err)
	print(debug.traceback(prettyprint.error(err), 2))
end

local function result(success, ...)
	debug.sethook() -- Reset debug hook
	local t = {...}
	
	-- Error case has already been handled by onerror via xpcall
	if not success then return end
	
	-- Print results
	env.it = t[1]
	for i = 1, select("#", ...) do
		print(pretty(t[i], true))
	end
	
	-- Print debug trace
	local level = 0
	for i = 4, #trace-3 do
		if trace[i].type == "return" then
			level = level-1
		elseif trace[i].type == "call" then
			print(resultstyle
				..string.rep("\u{2502} ", level).."\u{251c}\u{2574}"
				..prettyprint.trace(trace[i]))
			level = level+1
		end
	end
end

local function multiline(input)
	local fn, err = load(input, "=stdin", "t")
	while not fn and string.match(err, "<eof>$") do
		io.write(console.reset, "... ")
		local newinput = io.read()
		if not newinput then break end
		input = input.."\n"..newinput
		fn, err = load(input, "=stdin", "t")
	end
	return fn, err
end

local function hook(type)
	local d = debug.getinfo(2)
	d.type = type
	table.insert(trace, d)
end

setmetatable(_G, {__index = env})
print(console.fg.yellow.._MB_VERSION..console.reset.." for "..console.fg.yellow.._VERSION)

while true do
	io.write(console.reset, "> ")
	local input = io.read()
	if not input then print() os.exit() end
	local command, args = input:match("^:(%S+)%s*(.*)$")
	if command and commands[command] then
		commands[command](args)
	else
		local fn, err = load("return "..input, "=stdin", "t")
		if not fn then fn, err = multiline(input) end
		if not fn then
			print(prettyprint.error(err))
		else
			trace = {}
			if dotrace then debug.sethook(hook, "cr") end
			result(xpcall(fn, onerror))
		end
	end
end
