local thread = require "thread"

local a = 1

local function threadfunction(n)
	print("in thread "..n..", a = "..a.."!")
	a = a+1
	return n
end

local threads = {}

for i = 1, 4 do
	table.insert( threads, thread.new(threadfunction, i) )
end

for i = 1, #threads do
	print(threads[i]:wait())
end

print("in main, a = "..a.."!")