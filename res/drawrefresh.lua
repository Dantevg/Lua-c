local event = require "event"
local window = require "SDLWindow"
local filewatch = require "filewatch"

local path = ...
local chunk

if type(path) ~= "string" then
	error "Expected path"
end

local screen = window.new()

function update()
	print("update", path)
	chunk = dofile(path)
end

function draw(dt)
	if type(chunk) == "function" then chunk(screen, dt) end
	screen:present()
end

event.addTimer(100, draw, true)
event.on("screen", "resize", function(...) return screen:resize(...) end)
event.on("filewatch", filewatch.watch(path), update)
update()
