local screen = require "screen"
local event = require "event"

frame = 0
t = 0

function randompixels()
	for i = 1, 10 do
		screen.colour(math.random(255), math.random(255), math.random(255), 255)
		screen.pixel(math.random(screen.getWidth())-1, math.random(screen.getHeight())-1)
	end
end

function randomfill()
	for x = 1, math.floor(screen.getWidth()) do
		for y = 1, math.floor(screen.getHeight()) do
			screen.colour(math.random(255), math.random(255), math.random(255), 255)
			screen.pixel(x-1, y-1)
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
		screen.colour(l, l, l, 255)
		screen.pixel(screen.getWidth()//2 + x, screen.getHeight()//2 + y)
	end
end

screen.init()
screen.setScale(2)
function draw(dt)
	screen.clear()
	t = t+dt
	frame = frame+1
	if frame % 50 == 0 then -- Print average FPS every 50 frames
		print( math.floor(1 / dt * 1000) )
	end
	
	spiral()
	screen.present()
end

function textinput(data)
	print("textinput: "..data)
end

event.addTimer(20, draw)
event.on("textinput", textinput)