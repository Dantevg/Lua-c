local image = require "SDLImage"

local img = image.new("test/SDLImage.create.bmp")
img:loadFont("poly4x3-r_meta.lua")
img:colour(255)
img:write("Hello", 0, 10)
img:save("test/SDLImage.create.bmp")