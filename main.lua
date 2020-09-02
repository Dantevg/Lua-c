function main(str, str2)
	local s, len = bar(str)
	local s2, len2 = bar(str2)
	
	local t = {}
	t[s] = len
	t[s2] = len2
	
	return t
end

player1 = {
	name = "RedPolygon",
	level = 20,
}