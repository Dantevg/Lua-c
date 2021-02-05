local screen = require("screen.terminal").new()
local event = require "event"

local lastKey = 0

function draw()
	screen:clear()
	screen:write(lastKey, 5, 5)
	screen:write("Hello, World!", 10, 5)
	screen:pixel(9, 4)
	screen:rect(10, 7, 20, 10, true)
	screen:present()
end

function kb(key)
	lastKey = key
end

event.addTimer(100, draw, true)
event.on("kb", "down", kb)