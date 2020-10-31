local image = require "SDLImage"

local img = image.new(16, 16)
img:colour(255)
img:pixel(1,1)
img:save("test/SDLImage.create.bmp")