local thread = require "thread"

local function threadfunction()
	print("in thread!")
	
	return 10, 20, 30
end

local t1 = thread.new("Threadfunction1", threadfunction)
local t2 = thread.new("Threadfunction2", threadfunction)

print(thread.wait(t1))
print(thread.wait(t2))

-- local threads = {}

-- for i = 1, 10 do
-- 	table.insert( threads, thread.new("Threadfunction"..i, threadfunction) )
-- 	-- print(getmetatable(threads[i]).thread)
-- end

-- for i = 1, 10 do
-- 	print(thread.wait(threads[i]))
-- end

return false