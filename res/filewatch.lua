local event = require "event"
local lfs = require "lfs"

local screen = require "screen.terminal"
local s = screen.new()

local args = {...}
local file = args[1]
if not file then error("No file input given") end

local delay = args[2] and tonumber(args[2]) or 200

local timestamp

function change(newtime)
	timestamp = newtime
	s:clear()
	local f = io.open(file)
	if not f then error("Could not read file") end
	local y = 0
	for line in f:lines() do
		s:write(line, 0, y)
		y = y+1
	end
	f:close()
	s:present()
end

function watch()
	local newtime = lfs.attributes(file, "modification")
	if not newtime then error("Could not get file information") end
	if timestamp ~= newtime then
		change(newtime)
	end
end

event.addTimer(delay, watch, true)
change(lfs.attributes(file, "modification"))