local screen = require("screen.terminal").new()
require "event"

screen:clear()
screen:write("Hello, World!", 10, 5)
screen:pixel(9, 4)
screen:rect(10, 7, 20, 10, true)
screen:present()