local file = io.open("test/file.write.txt", "r")

local data = file:read("a")
file:close()

assert(data == "hello")

local success, err = os.remove("test/file.write.txt")
if not success then error(err) end