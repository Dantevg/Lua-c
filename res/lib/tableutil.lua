local tableutil = {}

tableutil.mt = {
	__index = tableutil,
}

function tableutil.contains(t, item)
	for _, v in pairs(t) do
		if v == item then return true end
	end
	return false
end

function tableutil.concat(t1, t2)
	if t2 then
		for _, v in ipairs(t2) do
			table.insert(t1, v)
		end
		return t1
	else
		local new = setmetatable({}, tableutil.mt)
		for _, v in ipairs(t1) do
			tableutil.concat(new, v)
		end
		return new
	end
end

function tableutil.tostringi(t)
	local str = "{"
	for i, v in ipairs(t) do
		if type(v) == "string" then
			str = str .. string.format("%q, ", v)
		else
			str = str .. tostring(v)..", "
		end
	end
	return str:sub(1, -3).."}"
end

function tableutil.tostringk(t)
	local str = "{"
	for k, v in pairs(t) do
		if type(k) == "string" then
			str = str .. string.format("[%q] = ", k)
		else
			str = str .. "["..k.."] = "
		end
		if type(v) == "string" then
			str = str .. string.format("%q, ", v)
		else
			str = str .. tostring(v)..", "
		end
	end
	return str:sub(1, -3).."}"
end



-- FP-style higher-order functions
-- functions that modify (map, filter) return a new table

function tableutil.map(t, fn)
	local new = setmetatable({}, tableutil.mt)
	for i, v in ipairs(t) do
		table.insert(new, fn(v, i, t))
	end
	return new
end

function tableutil.forEach(t, fn)
	for i, v in ipairs(t) do
		fn(v, i, t)
	end
	return t
end

function tableutil.reduce(t, acc, fn)
	for i, v in ipairs(t) do
		acc = fn(acc, v, i, t)
	end
	return acc
end

function tableutil.filter(t, fn)
	local new = setmetatable({}, tableutil.mt)
	for i, v in ipairs(t) do
		if fn(v, i, t) then table.insert(new, v) end
	end
	return new
end

function tableutil.generate(source, fn)
	local new = setmetatable({}, tableutil.mt)
	return tableutil.reduce(source, new,
		function(t, v, i, _) table.insert(t, (fn(v, i, new, source))) return t end
	)
end

return tableutil
