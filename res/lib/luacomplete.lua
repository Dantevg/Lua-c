local autocomplete = {}



-- HELPER FUNCTIONS

local function concat(a, ...)
	local args = {...}
	for _, t in ipairs(args) do
		for _, v in ipairs(t) do
			table.insert(a, v)
		end
	end
	return a
end



-- FINDING

function autocomplete.findGlobal(input, env)
	local match = input:match("([%a_][%w_]*)$")
	return (env or _G)[match]
end

function autocomplete.findKeyBracket(input)
	local from, _, match = input:find("%['([^']*)'%]$")
	if not from then
		from, _, match = input:find('%["([^"]*)"%]$')
	end
	if not match then return end
	return (autocomplete.find(input:sub(1, from-1)) or {})[match]
end

function autocomplete.findKeyDot(input)
	local from, _, match = input:find("%.([%a_][%w_]*)$")
	if not match then return end
	return (autocomplete.find(input:sub(1, from-1)) or {})[match]
end

function autocomplete.find(input)
	return autocomplete.findKeyBracket(input)
		or autocomplete.findKeyDot(input)
		or autocomplete.findGlobal(input)
end



-- AUTOCOMPLETION

function autocomplete.completeListK(input, list, after)
	local completions = {}
	for name, _ in pairs(list or {}) do
		if name:sub(1, #input) == input then
			table.insert(completions, name:sub(#input + 1)..(after or ""))
		end
	end
	return completions
end

function autocomplete.completeListV(input, list, after)
	local completions = {}
	for _, name in ipairs(list or {}) do
		if name:sub(1, #input) == input then
			table.insert(completions, name:sub(#input + 1)..(after or ""))
		end
	end
	return completions
end

autocomplete.keywords = {
	"and", "break", "do", "else", "elseif",
	"end", "false", "for", "function", "if",
	"in", "local", "nil", "not", "or",
	"repeat", "return", "then", "true", "until", "while",
}

function autocomplete.completeKeyword(input)
	local match = input:match("[^%.:%w](%w+)$") or input:match("^(%w+)$")
	if not match then return {} end
	return autocomplete.completeListV(match, autocomplete.keywords)
end

function autocomplete.completeGlobal(input, env)
	local match = input:match("[^%.:%a_]([%a_][%w_]*)$") or input:match("^([%a_][%w_]*)$")
	if not match then return {} end
	return autocomplete.completeListK(match, env or _G)
end

function autocomplete.completeKeyBracket(input)
	local after, from, _, match = "']", input:find("%['([^']*)$")
	if not match then
		after, from, _, match = '"]', input:find('%["([^"]*)$')
	end
	if not match then return end
	return autocomplete.completeListK(match,
		autocomplete.find(input:sub(1, from-1)), after)
end

function autocomplete.completeKeyDot(input)
	local from, _, match = input:find("%.([%a_][%w_]*)$")
	if not match then
		from, _, match = input:find("%.$")
		if from then match = "" end
	end
	if not match then return end
	return autocomplete.completeListK(match, autocomplete.find(input:sub(1, from-1)))
end

function autocomplete.completeKey(input)
	return autocomplete.completeKeyBracket(input)
		or autocomplete.completeKeyDot(input)
end

function autocomplete.complete(input)
	local keyword = autocomplete.completeKeyword(input)
	local global = autocomplete.completeGlobal(input)
	local key = autocomplete.completeKey(input)
	return concat(keyword, global, key)
end



-- HINTING

function autocomplete.hintFunction(completion, f)
	local d = debug.getinfo(f)
	local args = {}
	for i = 1, d.nparams do
		table.insert(args, (debug.getlocal(f, i)))
	end
	if d.isvararg then table.insert(args, "...") end
	return completion.."("..table.concat(args, ", ")..")"
end

function autocomplete.hints(input)
	local completions = autocomplete.complete(input)
	for i, completion in ipairs(completions) do
		local value = autocomplete.find(input..completion)
		if type(value) == "function" then
			completions[i] = autocomplete.hintFunction(completion, value)
		elseif type(value) == "table" then
			completions[i] = completion.."."
		end
	end
	return completions
end

function autocomplete.hint(input)
	local completion = autocomplete.complete(input)[1]
	if not completion then return end
	
	local value = autocomplete.find(input..completion)
	if type(value) == "function" then
		return autocomplete.hintFunction(completion, value)
	elseif type(value) == "table" then
		return completion.."."
	else
		return completion
	end
end



-- RETURN

function autocomplete.__call(_, ...)
	return autocomplete.complete(...)
end

return setmetatable(autocomplete, autocomplete)
