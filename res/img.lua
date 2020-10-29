local image = require "SDLImage"

local img = image.new("img.bmp")
-- local img = image.new(32, 32)

img:loadFont("poly4x3-r_meta.lua")
-- img:colour(0)
-- img:clear()
img:colour(255)
img:write("Hello", 0, 0)
img:save("img.bmp")

return false