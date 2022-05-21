local tc = require "prequire" "terminalcolours" -- Can do without colours
if not tc then
	-- So that the program will still run without terminalcolours
	tc = setmetatable(
		{fg = {}, bg = {}, cursor = {}},
		{__call = function() return "" end}
	)
end

local pretty = {}

pretty.reset = {tc.reset, tc.bright}

local reset = pretty.reset
local function nop(x) return tc(reset)..tostring(x) end
function pretty.special(x)
	return string.format("%s[%s]",
		tc(tc.bright, tc.fg.magenta), tostring(x))
end
local function prettyprint(x, long, ...)
	local t = type(x)
	
	if pretty[t] and not rawget(getmetatable(x) or {}, "__tostring") then
		return pretty[t](x, long, ...)..tc(reset)
	else
		return nop(x)..tc(reset)
	end
end
local function prettykey(x)
	if type(x) == "string" and x:match("^[%a_][%w_]*$") then
		return tostring(x)
	else
		return "["..prettyprint(x).."]"
	end
end

pretty["nil"] = function(x) return tc(tc.reset, tc.fg.grey)..tostring(x) end
pretty["number"] = function(x) return tc(tc.reset, tc.fg.cyan)..tostring(x) end
pretty["string"] = function(x) return tc(tc.reset, tc.fg.green)..'"'..x..'"' end
pretty["boolean"] = function(x) return tc(tc.reset, tc.fg.yellow)..tostring(x) end
pretty["table"] = function(x, maxdepth, multiline, depth)
	if type(x) ~= "table" and not rawget(getmetatable(x) or {}, "__pairs") then
		return pretty.error("not a table")
	end
	if maxdepth == true then maxdepth = 128 end
	if not maxdepth then maxdepth = 0 end
	depth = depth or 0
	if depth >= maxdepth then return pretty.special(x) end
	
	local contents = {}
	
	if type(x) == "table" then
		for _, v in ipairs(x) do
			table.insert(contents, prettyprint(v, maxdepth, multiline, depth + 1))
		end
	end
	
	for k, v in pairs(x) do
		if not contents[k] then
			table.insert(contents, prettykey(k).." = "..prettyprint(v, maxdepth, multiline, depth + 1))
		end
	end
	
	if multiline then
		local indent = string.rep("  ", depth)
		local newindent = indent.."  "
		return tc(reset).."{\n"
			..newindent..table.concat(contents, ",\n"..newindent).."\n"
			..indent.."}"..(getmetatable(x) and tc(tc.fg.grey).." + mt" or "")
	else
		return tc(reset).."{ "..table.concat(contents, ", ").." }"
			..(getmetatable(x) and tc(tc.fg.grey).." + mt" or "")
	end
end
pretty["function"] = function(x, long)
	if type(x) ~= "function" then return pretty.error("not a function") end
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
	return str..tc(reset).."\n"..table.concat(contents, "\n")
end
pretty["thread"] = pretty.special
pretty["userdata"] = pretty.special

pretty["error"] = function(x)
	return tc(tc.bright, tc.fg.red)..tostring(x)
end

return setmetatable(pretty, {
	__call = function(_, ...) return prettyprint(...) end,
})
