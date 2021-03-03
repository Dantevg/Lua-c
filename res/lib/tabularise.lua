local pattern = "([^\t]+)\t"

return function(t, distance, separator)
	distance = distance or 2
	separator = separator or " "
	
	local columnLengths = {}
	
	-- Get column lengths
	for i = 1, #t do
		local j = 0
		for part in string.gmatch(t[i], pattern) do
			j = j+1; columnLengths[j] = math.max(columnLengths[j] or 0, #part)
		end
	end
	
	-- Replace column separators
	for i = 1, #t do
		local j = 0
		t[i] = string.gsub(t[i], pattern, function(x)
			j = j+1; return x..string.rep(separator, columnLengths[j] - #x + distance)
		end)
	end
	
	return t
end