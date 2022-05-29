local prequire = require "prequire"
local terminal = prequire "terminal" -- Advanced terminal features may not be available
local tc = prequire "terminalcolours" -- Can do without colours
local autocomplete = prequire "luacomplete" -- Don't *need* autocompletion
local pretty = prequire "pretty"

local historyPath = _MB_RES_DIR..".luahistory.txt"

local trace = {}
local dotrace = false
local unicode = (...) ~= "--no-unicode"

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

if not pretty then
	pretty = setmetatable(
		{reset = {}},
		{
			__call = function(_, _, s) return s end,
			__index = function() return function(_, s) return s end end,
		}
	)
end

-- Add trace prettyprint
function pretty:trace(x)
	local args = {}
	for _, arg in ipairs(x.args) do
		table.insert(args, self:colour("reset")..tostring(arg[1])..": "..pretty(self, arg[2]))
	end
	return string.format("%s(%s%s) %s",
		x.name or x.source, table.concat(args, ", "), self:colour("reset"),
		pretty["function"](self, x.func))
end



local commands = {}

function commands.table(arg)
	local tbl = _G[arg]
	if not tbl then return end
	print(pretty:table(tbl))
end

function commands.metatable(arg)
	local mt = getmetatable(_G[arg])
	if not mt then return end
	print(pretty:table(mt))
end

commands["function"] = function(arg)
	local fn = _G[arg]
	if not fn then return end
	print(pretty["function"](fn))
end

function commands.require(arg)
	local has = package.loaded[arg]
	if has then package.loaded[arg] = nil end
	local success, result = pcall(require, arg)
	if success then
		_G[arg] = result
	else
		print(pretty:error(result))
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
		print(tc(tc.fg.grey)..arg..pretty:colour("reset")..completion)
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
	print(debug.traceback(pretty:error(err), 2))
end

local function result(success, ...)
	debug.sethook() -- Reset debug hook
	local t = {...}
	
	-- Error case has already been handled by onerror via xpcall
	if not success then return end
	
	-- Print results
	env.it = t[1]
	for i = 1, select("#", ...) do
		print(pretty(t[i]))
	end
	
	-- Print debug trace
	local level = 0
	for i = 4, #trace-3 do
		if trace[i].type == "return" then
			level = level-1
		elseif trace[i].type == "call" then
			print(pretty:colour("reset")
				..string.rep(unicode and "\u{2502} " or "| ", level)
				..(unicode and "\u{251c}\u{2574}" or "|-")
				..pretty:trace(trace[i]))
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
			print(pretty:error(err))
		else
			trace = {}
			if dotrace then debug.sethook(hook, "cr") end
			result(xpcall(fn, onerror))
		end
	end
end
