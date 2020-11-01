local thread = require "thread"

local function threadfunction(n)
	print("in thread "..n.."!")
	
	return 10, 20, 30
end

local threads = {}

for i = 1, 10 do
	table.insert( threads, thread.new("Threadfunction"..i, threadfunction, i) )
end

for i = 1, 10 do
	print(thread.wait(threads[i]))
end

return false