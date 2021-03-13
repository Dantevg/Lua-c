local function test(file)
	print("[TEST] "..file)
	local success, err = pcall(dofile, "test/"..file)
	if not success then print(err) end
end

print("Automatic tests")
test("auto/buffer.lua")
test("auto/require.lua")

print("Manual tests")
test("hello.lua")
test("file.write.lua")
test("file.read.lua")
test("image.create.lua")
test("image.load.write.lua")
test("image.load.read.lua")
test("luasocket.lua")
test("thread.lua")
test("sys.lua")
test("mouse.lua")
test("stream.lua")
test("event.lua")
