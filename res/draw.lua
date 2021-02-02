local window = require "SDLWindow"
local event = require "event"

local screen = window.new()

frame = 0
t = 0

function randompixels()
	for i = 1, 10 do
		screen:colour(math.random(255), math.random(255), math.random(255), 255)
		screen:pixel(math.random(screen:getWidth())-1, math.random(screen:getHeight())-1)
	end
end

function randomfill()
	for x = 1, math.floor(screen:getWidth()) do
		for y = 1, math.floor(screen:getHeight()) do
			screen:colour(math.random(255), math.random(255), math.random(255), 255)
			screen:pixel(x-1, y-1)
		end
	end
end

-- Adapted from https://github.com/klassmann/sdl2-lua53-example/blob/master/src/script.lua
function spiral()
	local n = 100
	local speed = 5000
	for i = 1, n do
		local x = math.floor(i * math.sin(i*t/speed))
		local y = math.floor(i * math.cos(i*t/speed))
		local l = 255 - i*255//n
		screen:colour(l, l, l, 255)
		screen:pixel(screen:getWidth()//2 + x, screen:getHeight()//2 + y)
	end
end

function draw(dt)
	screen:colour(0)
	screen:clear()
	t = t+dt
	frame = frame+1
	if frame % 50 == 0 then -- Print average FPS every 50 frames
		print( math.floor(1 / dt * 1000) )
	end
	
	spiral()
	screen:colour(255)
	screen:write(("The quick brown fox jumps over the lazy dog."):sub(1, frame), 0, 0)
	screen:present()
end

event.addTimer(20, draw, true)
-- event.addTimer(1500, print, true)
-- event.on("kb.input", print)
-- event.on("kb.down", print)
-- event.on("kb.up", print)
-- event.on("mouse.move", print)
-- event.on("mouse.down", print)
-- event.on("mouse.scroll", print)
event.on("screen", "resize", function(...) return screen:resize(...) end)

screen:loadFont("res/poly4x3-r_meta.lua")