local image = require "image"

local img = image.new(16, 16)
img:colour(255)
img:pixel(1,1)
img:save("test/image.create.bmp")