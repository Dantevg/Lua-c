local function test(file)
	print("[TEST] "..file)
	local success, err = pcall(dofile, "test/"..file)
	if not success then print(err) end
end

test("hello.lua")
test("require.lua")
test("file.write.lua")
test("file.read.lua")
test("SDLImage.create.lua")
test("SDLImage.load.write.lua")
test("SDLImage.load.read.lua")
test("luasocket.lua")
test("thread.lua")

return false