local window = require "SDLWindow"
local event = require "event"

local screen = window.new()
screen:loadFont("poly4x3-r_meta.lua")

local input = ""
local x, y = 0, 0

function onkeydown(_, key)
	if key == "backspace" and x > 0 then
		input = input:sub(1, -2)
		x = x-4
		screen:colour(0)
		screen:rect(x, y, 4, 5, true)
		screen:colour(255)
	elseif key == "return" then
		input = ""
		y = y + 6
		x = 0
	end
end

function oninput(_, text)
	input = input .. text
	screen:write(text, x, y)
	x = x+4
end

function draw()
	-- screen:clear()
	
	-- screen:write(input, 0, y)
	
	screen:present()
end

event.on("kb.down", onkeydown)
event.on("kb.input", oninput)
event.on("screen.resize", function(...) return screen:resize(...) end)
event.addTimer(20, draw, true)