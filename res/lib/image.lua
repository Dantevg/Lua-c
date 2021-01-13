--- The generic `image` module wrapper.
-- Returns any specific image module, which implements the `image` interface
-- 
-- Default implementations: @{image.SDLImage}
-- @module image

local prequire = require "prequire"
return prequire "image.SDLImage"
	or error "Could not find implementation for 'image'"