for i = 1, 5 do
	print("Waiting for customevent")
	coroutine.yield("customevent", i)
end