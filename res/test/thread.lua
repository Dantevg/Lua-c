local thread = require "thread"

local a = 1

local function threadfunction(n)
	print("in thread "..n..", a = "..a.."!")
	a = a+1
	
	return 10, 20
end

local threads = {}

for i = 1, 5 do
	table.insert( threads, thread.new("Threadfunction"..i, threadfunction, i) )
end

for i = 1, 5 do
	print(thread.wait(threads[i]))
end

print("in main, a = "..a.."!")

return false