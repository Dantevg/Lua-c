local image = require "SDLImage"

local function checkPixel(img, x, y, r, g, b)
	local r1, g1, b1 = img:getPixel(x,y)
	if r1 ~= r or g1 ~= g or b1 ~= b then
		error("Pixel colour mismatch: expected "..r.." "..g.." "..b
			..", got "..r1.." "..g1.." "..b1, 2)
	end
end

local img = image.new("test/SDLImage.create.bmp")

assert(img:getWidth() == 16)
assert(img:getHeight() == 16)
checkPixel(img, 0, 0, 0, 0, 0)
checkPixel(img, 0, 1, 0, 0, 0)
checkPixel(img, 1, 0, 0, 0, 0)
checkPixel(img, 1, 1, 255, 255, 255)

checkPixel(img, 0, 10, 0, 0, 0)
checkPixel(img, 0, 11, 255, 255, 255)
checkPixel(img, 1, 10, 0, 0, 0)
checkPixel(img, 1, 11, 0, 0, 0)

os.remove("test/SDLImage.create.bmp")