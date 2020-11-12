local image = require "image"

local img = image.new("test/image.create.bmp")
img:loadFont("res/poly4x3-r_meta.lua")
img:colour(255)
img:write("Hello", 0, 10)
img:save("test/image.create.bmp")