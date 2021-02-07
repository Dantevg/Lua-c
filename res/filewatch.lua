local event = require "event"
local filewatch = require "filewatch"

local args = {...}
if not args[1] then error("No file input given") end

event.on("filewatch", filewatch.watch(args[1]), print)
