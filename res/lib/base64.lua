--[[--
	
	Base64 encoding/decoding for streams.
	
	@module base64
	@author RedPolygon
	@see stream
	
]]--

local stream = require "stream"

local base64 = {}



-- CONFIGURATION VARIABLES

--- The character at index 62.
-- Default: `'+'`
base64.c62 = "+"
--- The character at index 63.
-- Default: `'/'`
base64.c63 = "/"
--- The character used for padding.
-- When this is `nil`, no padding will be added.
-- Default: `'='`
base64.padding = "="
--- Whether to error on missing padding characters.
-- Default: `false`
base64.strictPadding = false
--- The maximum line length.
-- Any value larger than 0 will enable line wrapping.
-- Default: `0`
base64.maxLength = 0
--- The newline character.
-- Default: `"\r\n"`
base64.newline = "\r\n"
--- Whether to error on characters outside the encoding alphabet
-- Default: `true`
base64.strictNonencoding = true



-- STANDARD CONFIGURATIONS

base64.standards = {}

function base64.standards.default()
	base64.c62 = "+"
	base64.c63 = "/"
	base64.padding = "="
	base64.strictPadding = false
	base64.maxLength = 0
	base64.strictNonencoding = true
end

function base64.standards.mime()
	base64.c62 = "+"
	base64.c63 = "/"
	base64.padding = "="
	base64.strictPadding = true
	base64.maxLength = 76
	base64.newline = "\r\n"
	base64.strictNonencoding = false -- Accept out-of-alphabet characters
end

function base64.standards.url()
	base64.c62 = "-"
	base64.c63 = "_"
	base64.padding = "="
	base64.strictPadding = false -- accept missing padding
	base64.maxLength = 0
	base64.strictNonencoding = true
end



-- ENCODING STREAM

stream.base64encode = {}

stream.base64encode.__index = stream.base64encode
stream.base64encode.__tostring = function(self)
	return string.format("%s -> Base64 encode", self.source)
end
stream.base64encode.__call = stream.get

stream.base64encode.charmap =
	stream("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"):table()

--- Encode the stream to base64 format
-- @function base64encode
-- @tparam[optchain=false] boolean isString whether the source should be interpreted
-- as a string, and converted with `string.byte` instead of `tonumber`
-- @treturn Stream
function stream.base64encode.new(source, isString)
	local self = {}
	self.source = source
	self.col = 0
	
	self.get = coroutine.wrap(function()
		self.source:map(isString and string.byte or tonumber)
			:groupBySize(3):map(stream.table) -- Divide into groups of 3
			:forAll(function(buf)
				-- Convert
				for _, v in ipairs(self:chunk(buf)) do
					self.col = self.col+1
					if base64.maxLength > 0 and self.col > base64.maxLength then
						coroutine.yield(base64.newline)
						self.col = 1
					end
					coroutine.yield(v)
				end
			end)
	end)
	
	return setmetatable(self, stream.base64encode)
end

function stream.base64encode:enc(x)
	if x == 62 then
		return base64.c62
	elseif x == 63 then
		return base64.c63
	elseif self.charmap[x+1] then
		return self.charmap[x+1]
	else
		error("Base64 character out of range")
	end
end

function stream.base64encode:chunk(buf)
	local all = buf[1] << 0x10 | (buf[2] or 0) << 0x8 | (buf[3] or 0)
	return {
		self:enc((all >> 3*6) & 0x3F),
		self:enc((all >> 2*6) & 0x3F),
		(#buf >= 2 and self:enc((all >> 1*6) & 0x3F) or base64.padding),
		(#buf >= 3 and self:enc((all >> 0*6) & 0x3F) or base64.padding),
	}
end

setmetatable(stream.base64encode, stream)



-- DECODING STREAM

stream.base64decode = {}

stream.base64decode.__index = stream.base64decode
stream.base64decode.__tostring = function(self)
	return string.format("%s -> Base64 decode", self.source)
end
stream.base64decode.__call = stream.get

stream.base64decode.charmap = {}
for i = 1, #stream.base64encode.charmap do
	stream.base64decode.charmap[stream.base64encode.charmap[i]] = i
end

--- Decode the stream from base64 format
-- @function base64decode
-- @tparam[opt=false] boolean toString whether the converted data should be converted
-- to a string with `string.char`
-- @treturn Stream
function stream.base64decode.new(source, toString)
	local self = {}
	self.source = source
	
	self.get = coroutine.wrap(function()
		self.source:map(tostring):filter(stream.op.neq "\n"):filter(stream.op.neq "\r")
			:filter(function(x)
				if not base64.strictNonencoding then
					return string.find(table.concat(stream.base64encode.charmap)..base64.c62..base64.c63, x, 1, true)
				else return true end
			end)
			:groupBySize(4):map(stream.table)
			:forAll(function(buf)
				if #buf ~= 4 and base64.strict then
					error("Invalid base64 string: length is not a multiple of 4 (check padding options)")
				end
				
				-- Convert
				for _, v in ipairs(self:chunk(buf)) do
					coroutine.yield(toString and string.char(v) or v)
				end
			end)
	end)
	
	return setmetatable(self, stream.base64decode)
end

function stream.base64decode:dec(x)
	if x == base64.c62 then
		return 62
	elseif x == base64.c63 then
		return 63
	elseif self.charmap[x] then
		return self.charmap[x] - 1
	else
		error("Invalid base64 character (are c62 and c63 correct?)")
	end
end

function stream.base64decode:chunk(buf)
	local all =
		  ((buf[1] and buf[1] ~= base64.padding) and self:dec(buf[1]) or 0) << 3*6
		| ((buf[2] and buf[2] ~= base64.padding) and self:dec(buf[2]) or 0) << 2*6
		| ((buf[3] and buf[3] ~= base64.padding) and self:dec(buf[3]) or 0) << 1*6
		| ((buf[4] and buf[4] ~= base64.padding) and self:dec(buf[4]) or 0) << 0*6
	
	return {
		((all >> 2*8) & 0xFF ~= 0 and (all >> 2*8) & 0xFF or nil),
		((all >> 1*8) & 0xFF ~= 0 and (all >> 1*8) & 0xFF or nil),
		((all >> 0*8) & 0xFF ~= 0 and (all >> 0*8) & 0xFF or nil),
	}
end

setmetatable(stream.base64decode, stream)



-- RETURN

return base64
