--[[--
	
	Lua syntax autocompletions. Most important functions are @{complete} and @{hint}.
	
	@module luacomplete
	@author RedPolygon
	@license MIT
	
]]--

local luacomplete = {}



-- HELPER FUNCTIONS

local function concat(a, ...)
	if type(a) ~= "table" then a = {} end
	local args = {...}
	for _, t in ipairs(args) do
		if type(t) == "table" then
			for _, v in ipairs(t) do
				table.insert(a, v)
			end
		end
	end
	return a
end



-- FINDING

--- Find a global.
-- @tparam string input
-- @tparam[opt] table env
-- @return result
function luacomplete.findGlobal(input, env)
	local match = input:match("([%a_][%w_]*)$")
	return (env or _G)[match]
end

--- Find a key in bracket-style.
-- @tparam string input
-- @return value
function luacomplete.findKeyBracket(input)
	local from, _, match = input:find("%['([^']*)'%]$")
	if not from then
		from, _, match = input:find('%["([^"]*)"%]$')
	end
	if not match then return end
	return (luacomplete.find(input:sub(1, from-1)) or {})[match]
end

--- Find a key in dot-style.
-- @tparam string input
-- @return value
function luacomplete.findKeyDot(input)
	local from, _, match = input:find("%.([%a_][%w_]*)$")
	if not match then return end
	return (luacomplete.find(input:sub(1, from-1)) or {})[match]
end

--- Get a value.
-- @tparam string input
-- @return value
function luacomplete.find(input)
	return luacomplete.findKeyBracket(input)
		or luacomplete.findKeyDot(input)
		or luacomplete.findGlobal(input)
end



-- AUTOCOMPLETION

--- Find a completion in the keys of a table.
-- When the table has a metatable with a table field __index,
-- that will also be traversed, recursively.
-- @tparam string input
-- @tparam table list
-- @tparam[opt] string after will be placed after every completion
-- @treturn table completions
function luacomplete.completeListK(input, list, after)
	local completions = {}
	if type(list) == "table" then
		for name, _ in pairs(list) do
			if name:sub(1, #input) == input then
				table.insert(completions, name:sub(#input + 1)..(after or ""))
			end
		end
	end
	
	-- Traverse __index
	local mt = getmetatable(list)
	if type(mt) == "table" and type(rawget(mt, "__index")) == "table" and mt.__index ~= list then
		concat(completions, luacomplete.completeListK(input, mt.__index, after))
	end
	
	return completions
end

--- Find a completion in a table of values.
-- @tparam string input
-- @tparam table list
-- @tparam[opt] string after will be placed after every completion
-- @treturn table completions
function luacomplete.completeListV(input, list, after)
	local completions = {}
	for _, name in ipairs(list or {}) do
		if name:sub(1, #input) == input then
			table.insert(completions, name:sub(#input + 1)..(after or ""))
		end
	end
	return completions
end

--- List of Lua 5.3 keywords.
luacomplete.keywords = {
	"and", "break", "do", "else", "elseif",
	"end", "false", "for", "function", "if",
	"in", "local", "nil", "not", "or",
	"repeat", "return", "then", "true", "until", "while",
}

--- Try to complete a keyword.
-- @tparam string input
-- @treturn table completions
function luacomplete.completeKeyword(input)
	local match = input:match("[^%.:%w](%w+)$") or input:match("^(%w+)$")
	if not match then return {} end
	return luacomplete.completeListV(match, luacomplete.keywords)
end

--- Try to complete a global.
-- @tparam string input
-- @tparam[opt=_G] table env
-- @treturn table completions
function luacomplete.completeGlobal(input, env)
	local match = input:match("[^%.:%a_]([%a_][%w_]*)$") or input:match("^([%a_][%w_]*)$")
	if not match then return {} end
	return luacomplete.completeListK(match, env or _G)
end

--- Try to complete a table key in bracket-style.
-- @tparam string input
-- @treturn table completions
function luacomplete.completeKeyBracket(input)
	local after, from, _, match = "']", input:find("%['([^']*)$")
	if not match then
		after, from, _, match = '"]', input:find('%["([^"]*)$')
	end
	if not match then return end
	return luacomplete.completeListK(match,
		luacomplete.find(input:sub(1, from-1)), after)
end

--- Try to complete a table key in dot-style.
-- @tparam string input
-- @treturn table completions
function luacomplete.completeKeyDot(input)
	local from, _, match = input:find("%.([%a_][%w_]*)$")
	if not match then
		from, _, match = input:find("%.$")
		if from then match = "" end
	end
	if not match then return end
	return luacomplete.completeListK(match, luacomplete.find(input:sub(1, from-1)))
end

--- Try to complete a table key in both bracket and dot style.
-- @tparam string input
-- @treturn table completions
function luacomplete.completeKey(input)
	return luacomplete.completeKeyBracket(input)
		or luacomplete.completeKeyDot(input)
		or {}
end

--- Try to complete any input (table key, global and keyword).
-- @tparam string input
-- @treturn table completions (sorted)
function luacomplete.complete(input)
	local key = luacomplete.completeKey(input)
	local global = luacomplete.completeGlobal(input)
	local keyword = luacomplete.completeKeyword(input)
	local completions = concat(key, global, keyword)
	table.sort(completions)
	return completions
end



-- HINTING

--- Get hint for a function.
-- @tparam string completion
-- @tparam function f
-- @treturn string hint
function luacomplete.hintFunction(completion, f)
	local d = debug.getinfo(f)
	local args = {}
	for i = 1, d.nparams do
		table.insert(args, (debug.getlocal(f, i)))
	end
	if d.isvararg then table.insert(args, "...") end
	return completion.."("..table.concat(args, ", ")..")"
end

--- Get hints for an input.
-- @tparam string input
-- @treturn table hints
function luacomplete.hints(input)
	local hints = luacomplete.complete(input)
	for i, completion in ipairs(hints) do
		local value = luacomplete.find(input..completion)
		if type(value) == "function" then
			hints[i] = luacomplete.hintFunction(completion, value)
		elseif type(value) == "table" then
			hints[i] = completion.."."
		end
	end
	return hints
end

--- Get hint for an input.
-- @tparam string input
-- @treturn string hint
function luacomplete.hint(input)
	local hint = luacomplete.complete(input)[1]
	if not hint then return end
	
	local value = luacomplete.find(input..hint)
	if type(value) == "function" then
		return luacomplete.hintFunction(hint, value)
	elseif type(value) == "table" then
		return hint.."."
	else
		return hint
	end
end



-- RETURN

function luacomplete.__call(_, ...)
	return luacomplete.complete(...)
end

return setmetatable(luacomplete, luacomplete)
