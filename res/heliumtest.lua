local window = require "SDLWindow"
local event = require "event"
local mouse = require "mouse"

local helium = require "helium"
helium.Box = require "helium.Box"
helium.Text = require "helium.Text"

local screen = window.new()

local he = helium.new(screen)
he:append(helium.Box(10, 10, 20, 10, {255}))
he:append(helium.Text("Hello!", 0, 0, {255}))

function draw()
	screen:colour(0)
	screen:clear()
	he:draw()
	screen:present()
end

function update(x, y)
	if not mouse.down(1) then return end
	local b = he:findByType("box")
	b.w, b.h = (x-b.x)//2, (y-b.y)//2
end

event.addTimer(20, draw, true)
event.on("mouse.move", function(_, x, y) update(x, y) end)
event.on("mouse.down", function(_, _, x, y) update(x, y) end)
event.on("mouse.up", update)
screen:loadFont("res/poly4x3-r_meta.lua")