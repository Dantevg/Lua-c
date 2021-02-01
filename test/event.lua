local event = require "event"

event.on("myCustomEvent", function(...) print(...) os.exit() end)
event.push("myCustomEvent", "hello", "world")