local event = require "event"
local window = require "SDLWindow"
local filewatch = require "filewatch"

local path = ...
local chunk, noError = nil, false

if type(path) ~= "string" then
	error "Expected path"
end

local screen = window.new()

function update()
	noError, chunk = pcall(dofile, path)
	if not noError then print(chunk) end
end

function draw(dt)
	if type(chunk) == "function" and noError then
		local err
		noError, err = pcall(chunk, screen, dt)
		if not noError then print(err) end
	end
	screen:present()
end

event.addTimer(0, draw, true)
event.on("screen", "resize", function(...) return screen:resize(...) end)
event.on("filewatch", filewatch.watch(path), update)
screen:loadFont("res/poly4x3-r_meta.lua")
update()
