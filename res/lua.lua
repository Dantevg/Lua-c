local terminal = require "prequire" "terminal" -- Advanced terminal features may not be available
local tc = require "prequire" "terminalcolours" -- Can do without colours
local autocomplete = require "prequire" "luacomplete" -- Don't *need* autocompletion

local historyPath = "res/.luahistory.txt"

local trace = {}
local dotrace = false

local env = {}



-- Module replacements

if not tc then
	-- So that the program will still run without terminalcolours
	tc = setmetatable(
		{fg = {}, bg = {}, cursor = {}},
		{__call = function() return "" end}
	)
end

local read
if terminal then
	read = terminal.read
else
	read = function(prompt)
		io.write(prompt)
		return io.read()
	end
end



local prettyprint = {}

local resultstyle = {tc.reset, tc.bright}
local function nop(x) return tc(resultstyle)..tostring(x) end
local function special(x)
	return string.format("%s[%s]",
		tc(tc.bright, tc.fg.magenta), tostring(x))
end
local function pretty(x, long)
	local t = type(x)
	
	if prettyprint[t] and not rawget(getmetatable(x) or {}, "__tostring") then
		return prettyprint[t](x, long)..tc(resultstyle)
	else
		return nop(x)..tc(resultstyle)
	end
end
local function prettykey(x)
	if type(x) == "string" and x:match("^[%a_][%w_]*$") then
		return tostring(x)
	else
		return "["..pretty(x).."]"
	end
end

prettyprint["nil"] = function(x) return tc(tc.reset, tc.fg.grey)..tostring(x) end
prettyprint["number"] = function(x) return tc(tc.reset, tc.fg.cyan)..tostring(x) end
prettyprint["string"] = function(x) return tc(tc.reset, tc.fg.green)..'"'..x..'"' end
prettyprint["boolean"] = function(x) return tc(tc.reset, tc.fg.yellow)..tostring(x) end
prettyprint["table"] = function(x, long)
	if type(x) ~= "table" and not rawget(getmetatable(x) or {}, "__pairs") then
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
	
	return tc(resultstyle).."{ "..table.concat(contents, ", ").." }"
		..(getmetatable(x) and tc(tc.fg.grey).." + mt" or "")
end
prettyprint["function"] = function(x, long)
	if type(x) ~= "function" then return prettyprint.error("not a function") end
	local d = debug.getinfo(x, "S")
	local filename = d.short_src:match("([^/]+)$")
	local str = string.format("%s[%s @ %s:%d]",
		tc(tc.bright, tc.fg.magenta), tostring(x), filename, d.linedefined)
		
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
	return str..tc(resultstyle).."\n"..table.concat(contents, "\n")
end
prettyprint["thread"] = special
prettyprint["userdata"] = special

prettyprint["error"] = function(x)
	return tc(tc.bright, tc.fg.red)..tostring(x)
end

prettyprint["trace"] = function(x)
	local args = {}
	for _, arg in ipairs(x.args) do
		table.insert(args, tc(tc.reset)..tostring(arg[1])..": "..pretty(arg[2]))
	end
	return string.format("%s(%s%s) %s",
		x.name or x.source, table.concat(args, ", "), tc(resultstyle),
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
	-- Now we do really need luacomplete module
	if not autocomplete then autocomplete = require "luacomplete" end
	local completions = autocomplete(arg or "")
	for _, completion in ipairs(completions) do
		print(tc(tc.fg.grey)..arg..tc(resultstyle)..completion)
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
			print(tc(resultstyle)
				..string.rep("\u{2502} ", level).."\u{251c}\u{2574}"
				..prettyprint.trace(trace[i]))
			level = level+1
		end
	end
end

local function multiline(input)
	local fn, err = load(input, "=stdin", "t")
	while not fn and string.match(err, "<eof>$") do
		local newinput = read(tc(tc.reset).."... ")
		if not newinput then break end
		input = input.."\n"..newinput
		fn, err = load(input, "=stdin", "t")
	end
	return fn, err
end

local function hook(type)
	local d = debug.getinfo(2)
	d.type = type
	d.args = {}
	for i = 1, d.nparams do
		table.insert(d.args, {debug.getlocal(2, i)})
	end
	table.insert(trace, d)
end

if terminal then
	if autocomplete then
		terminal.autocomplete = autocomplete.complete
		terminal.hints = autocomplete.hint
	end
	terminal.history.setLength(100)
	terminal.history.load(historyPath)
end
setmetatable(_G, {__index = env})
print(tc(tc.fg.yellow).._MB_VERSION..tc(tc.reset).." for "..tc(tc.fg.yellow).._VERSION..tc(tc.reset))

while true do
	local input = read(tc(tc.reset).."> ")
	if not input then
		if terminal then terminal.history.save(historyPath) end
		io.write(tc(tc.reset))
		os.exit()
	end
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
