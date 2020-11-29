--- The generic `image` module wrapper.
-- Returns any specific image module, which implements the `image` interface
-- 
-- Default implementations: @{image.SDLImage}
-- @module image

return require("image.SDLImage")