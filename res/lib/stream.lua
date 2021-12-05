--[[--
	
	Arbitrary data streams. Most of these functions are inspired by Java's
[streams](https://docs.oracle.com/javase/8/docs/api/java/util/stream/Stream.html)
and Javascript's stream-like
[array functions](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array#instance_methods).
Some functions might not do what you think they do based on other languages, though.
	
	@module stream
	@author RedPolygon
	
]]--

local op = require "operator"

local stream = {}



-- DEFAULTS (for internal use)

stream.__index = stream
stream.__call = function(x, ...) return x.new(...) end

function stream:get()
	local x = self.get()
	-- Replace dead coroutine with nop function,
	-- to prevent "cannot resume dead coroutine" errors
	if x == nil then self.get = op.nop end
	return x
end



-- SOURCES

--- Auto-detect stream source type.
-- You can also just call `stream(data)` immediately.
-- @param source
-- @treturn Stream
function stream.new(source)
	local t = type(source)
	if t == "string" then
		return stream.string(source)
	elseif t == "table" then
		return stream.table(source)
	elseif t == "userdata" and getmetatable(source) == getmetatable(io.stdin) then
		return stream.file(source)
	else -- for generator functions or arbitrary values (numbers, booleans, userdata)
		return stream.generate(source)
	end
end

--- Check whether `source` is a stream
-- @param source
-- @treturn boolean
function stream.is(source)
	local mt = getmetatable(source)
	return mt and mt.__index
		and (mt.__index == stream or getmetatable(mt.__index) == stream)
end



stream.null = {}

stream.null.__index = stream.null
stream.null.__tostring = function(self)
	return string.format("%s Null", stream)
end
stream.null.__call = stream.get

--- Empty stream source, or pump all stream values.
-- When `source` is a @{Stream}, pumps all values and ignores the result.  
-- Otherwise, returns a new empty stream (somewhat useless).
-- @function null
-- @tparam[opt] Stream source
-- @treturn nil|Stream
function stream.null.new(source)
	if stream.is(source) then
		return source:forAll(op.nop)
	end
	
	local self = {}
	self.get = op.nop
	
	return setmetatable(self, stream.null)
end

setmetatable(stream.null, stream)

--- Alias for @{null}
-- @function empty
-- @see null
stream.empty = stream.null



stream.string = {}

stream.string.__index = stream.string
stream.string.__tostring = function(self)
	return string.format("%s String", stream)
end
stream.string.__call = stream.get

--- Stream string source or sink.
-- 
-- When `source` is a @{Stream}, reduces this stream to a string,
-- alias for `table.concat(source:table())`.
-- 
-- Otherwise, creates a @{Stream} from this string (or an empty string)
-- @function string
-- @tparam[opt] Stream|string source
-- @treturn string|Stream
-- @usage stream.string("hello"):table() --> {'h','e','l','l','o'}
-- @usage stream.table({1,2,3}):string() --> "123"
function stream.string.new(source)
	if stream.is(source) then
		return table.concat(source:map(tostring):table())
	end
	
	local self = {}
	self.source = source or ""
	
	self.get = coroutine.wrap(function()
		local i = 1
		local sub = self.source:sub(i,i)
		while sub ~= "" do
			coroutine.yield(sub)
			i = i+1
			sub = self.source:sub(i,i)
		end
	end)
	
	return setmetatable(self, stream.string)
end

setmetatable(stream.string, stream)



stream.file = {}

stream.file.__index = stream.file
stream.file.__tostring = function(self)
	if self.file == io.stdin then
		return string.format("%s File (stdin)", stream)
	elseif self.path then
		return string.format("%s File %q", stream, self.path)
	else
		return string.format("%s File", stream)
	end
end
stream.file.__call = stream.get

--- Stream file source.
-- 
-- When `source` is a @{Stream}, reads from this stream to `file`, which is a
-- path or opened file (or `stdout`), and returns `nil`: (default for `mode` is `"ab"`)  
-- `source:file([file] [, mode])`
-- 
-- Otherwise, reads from `source`, which is a path or opened file (or `stdin`),
-- and returns a @{Stream}: (default for `mode` is `"rb"`)  
-- `file([source] [, mode])`
-- 
-- When `mode` is specified, writes/reads to/from the file using this mode.
-- @function file
-- @tparam[opt=io.stdin] Stream|string|file source
-- @tparam[opt=io.stdout] string|file file
-- @tparam[opt] string mode
-- @treturn nil|Stream
-- @usage stream.file("input.txt"):string() --> "Hello" (contents of input.txt)
-- @usage stream.string(""):file("emptyme.txt", "w") -- clear file contents
-- @usage stream.file():file() -- echo user input back
function stream.file.new(source, file, mode)
	if stream.is(source) then
		local f = (type(file) == "string") and io.open(file, mode or "ab") or file or io.stdout
		for x in source do f:write(x) end
		f:close()
		return
	end
	
	local self = {}
	if type(source) == "string" then
		self.source = io.open(source, file or "rb")
	else
		self.source = source or io.stdin
	end
	if not self.source then error "Could not open file" end
	
	self.get = function()
		local x = self.source:read(1)
		if x == nil then self.source:close() end
		return x
	end
	
	return setmetatable(self, stream.file)
end

setmetatable(stream.file, stream)



stream.table = {}

stream.table.__index = stream.table
stream.table.__tostring = function(self)
	return string.format("%s Table", stream)
end
stream.table.__call = stream.get

--- Stream table source.
-- 
-- When `source` is a @{Stream}, reduces the stream to a table.
-- Alias for
-- 	source:collect({}, table.insert)
-- 
-- When `source` is a table, creates a @{Stream} from this table (or an empty table)
-- @function table
-- @tparam[opt] table|Stream source
-- @treturn Stream|table
-- @usage stream.table({1,2,3}):string() --> "123"
-- @usage stream.string("hello"):table() --> {'h','e','l','l','o'}
function stream.table.new(source)
	if stream.is(source) then
		return source:collect({}, table.insert)
	end
	
	local self = {}
	self.source = source or {}
	
	self.get = coroutine.wrap(function()
		local i = 1
		while self.source[i] do
			coroutine.yield(self.source[i])
			i = i+1
		end
	end)
	
	return setmetatable(self, stream.table)
end

setmetatable(stream.table, stream)



stream.generate = {}

stream.generate.__index = stream.generate
stream.generate.__tostring = function(self)
	if self.source then
		return string.format("%s Generate %q", stream, self.source)
	else
		return string.format("%s Generate", stream)
	end
end
stream.generate.__call = stream.get

--- Stream generator function or value source.
-- When `data` is a function, repeatedly calls this function for values.
-- Otherwise, yields `data` repeatedly.
-- @function generate
-- @tparam function|any data
-- @tparam[opt] boolean coro when `fn` is a function, wrap it in a coroutine
-- @treturn Stream
-- @usage stream.generate(math.random):take(2):table() --> {0.84018771676347, 0.39438292663544}
function stream.generate.new(fn, coro)
	local self = {}
	if type(fn) == "function" then
		self.get = function() return fn() end
		if coro then self.get = coroutine.wrap(self.get) end
	else
		self.source = fn
		self.get = function() return self.source end
	end
	
	return setmetatable(self, stream.generate)
end

setmetatable(stream.generate, stream)



stream.iterate = {}

stream.iterate.__index = stream.iterate
stream.iterate.__tostring = function(self)
	return string.format("%s Iterate", stream)
end
stream.iterate.__call = stream.get

--- Generate an iterative stream.
-- The first result will be `seed`. Next, `fn` will be called with the seed,
-- and the result will be stored as the seed.
-- @function iterate
-- @param seed
-- @param[opt] ... the rest of the seed values
-- @tparam function fn
-- @treturn Stream
function stream.iterate.new(...)
	local self = {}
	self.seeds = {...}
	self.fn = table.remove(self.seeds) or op.id
	
	self.get = function()
		local x = self.seeds[1]
		if x == nil then return end
		self.seeds = {self.fn(table.unpack(self.seeds))}
		return x
	end
	
	return setmetatable(self, stream.iterate)
end

setmetatable(stream.iterate, stream)



stream.loop = {}

stream.loop.__index = stream.loop
stream.loop.__tostring = function(self)
	return string.format("%s Loop", stream)
end
stream.loop.__call = stream.get

--- Generate stream as if it were a generic for-loop.
-- The resulting stream will be of tables of the results of `fn`.
-- @function loop
-- @tparam function fn
-- @tparam table t
-- @param var
-- @treturn Stream
function stream.loop.new(fn, t, var)
	local self = {}
	self.fn = fn
	self.t = t
	self.var = var
	
	self.get = function()
		local vars = {self.fn(self.t, self.var)}
		if vars[1] == nil then return end
		self.var = vars[1]
		return stream.table(vars)
	end
	
	return setmetatable(self, stream.loop)
end

setmetatable(stream.loop, stream)



--- Pairwise loop over the table.
-- Alias for `loop(pairs(t))`
-- @function pairs
-- @tparam table t
-- @treturn Stream
-- @see loop
function stream.pairs(t)
	return setmetatable(stream.loop(pairs(t)), {
		__index = stream.loop,
		__tostring = function()
			return string.format("%s Pairs", stream)
		end,
		__call = stream.get,
	})
end



--- Pairwise loop over the numerical indices of the table.
-- Alias for `loop(ipairs(t))`
-- @function ipairs
-- @tparam table t
-- @treturn Stream
-- @see loop
function stream.ipairs(t)
	return setmetatable(stream.loop(ipairs(t)), {
		__index = stream.loop,
		__tostring = function()
			return string.format("%s Ipairs", stream)
		end,
		__call = stream.get,
	})
end



--- Generate a range of numbers.
-- @function range
-- @tparam number from
-- @tparam[opt] number to
-- @treturn Stream
-- @see generate
function stream.range(from, to)
	return setmetatable(stream.generate(function()
			from = from+1
			return (not to or from-1 <= to) and from-1 or nil
		end), {
		__index = stream.generate,
		__tostring = function()
			if to then
				return string.format("%s Range (%d,%d)", stream, from, to)
			else
				return string.format("%s From %d", stream, from)
			end
		end,
		__call = stream.get,
	})
end

--- Alias for @{range}.
-- @function from
-- @see range
stream.from = stream.range



-- FILTERS

--- @type Stream

--- Get a single value from the stream.
-- You can also just call the stream immediately. This makes it possible
-- for the streams to be used as iterators (see example)
-- @function get
-- @return the value from the stream, or `nil` if nothing is present
-- @usage
-- for x in stream("Hello, world! From Lua"):group(stream.util.match("%a")) do
-- 	print(x:string())
-- end
-- --> Hello
-- --> world
-- --> From
-- --> Lua

stream.map = {}

stream.map.__index = stream.map
stream.map.__tostring = function(self)
	return string.format("%s -> Map", self.source)
end
stream.map.__call = stream.get

--- Perform a function on every value.
-- The function will be called with every value from the source,
-- the result of this function will be passed on.
-- @function map
-- @tparam function fn
-- @treturn Stream
-- @usage stream.string("12345"):map(tonumber):table() --> {1,2,3,4,5}
function stream.map.new(source, fn)
	local self = {}
	self.source = source
	self.fn = fn or op.id
	
	self.get = coroutine.wrap(function()
		for x in self.source do
			coroutine.yield(self.fn(x))
		end
	end)
	
	return setmetatable(self, stream.map)
end

setmetatable(stream.map, stream)



stream.flatMap = {}

stream.flatMap.__index = stream.flatMap
stream.flatMap.__tostring = function(self)
	return string.format("%s -> FlatMap", self.source)
end
stream.flatMap.__call = stream.get

--- Perform a function on every value, and flatten the result.
-- The function will be called with every value from the source,
-- the results of the stream returned by this function will be passed on.
-- @function flatMap
-- @tparam function fn
-- @treturn Stream
-- @usage stream({{1,2},{},{3,4},{5}}):flatMap(stream.new):table() --> {1,2,3,4,5}
function stream.flatMap.new(source, fn)
	local self = {}
	self.source = source
	self.fn = fn or function(x) return stream.generate(x) end
	
	self.get = coroutine.wrap(function()
		for x in self.source do
			local s = self.fn(x)
			if not stream.is(s) then error("Not a stream of streams") end
			for y in s do
				coroutine.yield(y)
			end
		end
	end)
	
	return setmetatable(self, stream.flatMap)
end

setmetatable(stream.flatMap, stream)



stream.flat = {}

stream.flat.__index = stream.flat
stream.flat.__tostring = function(self)
	return string.format("%s -> Flat", self.source)
end
stream.flat.__call = stream.get

--- Flatten the stream one level.
-- Receives a stream of streams,
-- returns a stream of the values in the inner streams
-- @function flat
-- @treturn Stream
function stream.flat.new(source)
	local self = {}
	self.source = source
	
	self.get = coroutine.wrap(function()
		for x in self.source do
			if not stream.is(x) then error("Not a stream of streams") end
			for y in x do
				coroutine.yield(y)
			end
		end
	end)
	
	return setmetatable(self, stream.flat)
end

setmetatable(stream.flat, stream)



stream.forEach = {}

stream.forEach.__index = stream.forEach
stream.forEach.__tostring = function(self)
	return string.format("%s -> ForEach", self.source)
end
stream.forEach.__call = stream.get

--- Perform a function on every value, but ignore the result.
-- @function forEach
-- @tparam function fn
-- @treturn Stream
-- @usage stream.string("123"):forEach(print):null()
-- --> 1
-- --> 2
-- --> 3
function stream.forEach.new(source, fn)
	local self = {}
	self.source = source
	self.fn = fn or op.nop
	
	self.get = coroutine.wrap(function()
		for x in self.source do
			self.fn(x)
			coroutine.yield(x)
		end
	end)
	
	return setmetatable(self, stream.forEach)
end

setmetatable(stream.forEach, stream)



stream.filter = {}

stream.filter.__index = stream.filter
stream.filter.__tostring = function(self)
	return string.format("%s -> Filter", self.source)
end
stream.filter.__call = stream.get

--- Filter the stream.
-- The function will be called with every value from its source,
-- and the value will only be passed on when it returns true.
-- @function filter
-- @tparam function fn
-- @treturn Stream
-- @usage
-- stream.string("AbCdEf"):filter(stream.util.match("%l")):string() --> "bdf"
function stream.filter.new(source, fn)
	local self = {}
	self.source = source
	self.fn = fn or function() return true end
	
	self.get = coroutine.wrap(function()
		for x in self.source do
			if self.fn(x) then coroutine.yield(x) end
		end
	end)
	
	return setmetatable(self, stream.filter)
end

setmetatable(stream.filter, stream)



stream.reverse = {}

stream.reverse.__index = stream.reverse
stream.reverse.__tostring = function(self)
	return string.format("%s -> Reverse", self.source)
end
stream.reverse.__call = stream.get

--- Reverse the stream.
-- **Warning**: this function needs to read the entire source stream
-- before it can return any value. Will not work for infinite streams.
-- @function reverse
-- @treturn Stream
-- @usage stream.string("hello"):reverse():string() --> "olleh"
function stream.reverse.new(source)
	local self = {}
	self.source = source
	self.data = {}
	
	self.get = coroutine.wrap(function()
		for x in self.source do
			table.insert(self.data, x)
		end
		for i = #self.data, 1, -1 do
			coroutine.yield(self.data[i])
		end
	end)
	
	return setmetatable(self, stream.reverse)
end

setmetatable(stream.reverse, stream)



stream.take = {}

stream.take.__index = stream.take
stream.take.__tostring = function(self)
	return string.format("%s -> Take %s", self.source, self.n)
end
stream.take.__call = stream.get

--- Take the first `n` values.
-- @function take
-- @tparam[opt=1] number n
-- @treturn Stream
function stream.take.new(source, n)
	local self = {}
	self.source = source
	self.n = n or 1
	
	self.get = coroutine.wrap(function()
		for i = 1, self.n do
			coroutine.yield(self.source())
		end
	end)
	
	return setmetatable(self, stream.take)
end

setmetatable(stream.take, stream)

--- Alias for @{take}.
-- @function head
-- @see take
stream.head = stream.take

--- Alias for @{take}.
-- @function limit
-- @see take
stream.limit = stream.take



stream.drop = {}

stream.drop.__index = stream.drop
stream.drop.__tostring = function(self)
	return string.format("%s -> Drop %s", self.source, self.n)
end
stream.drop.__call = stream.get

--- Drop the first `n` values.
-- @function drop
-- @tparam[opt=1] number n
-- @treturn Stream
function stream.drop.new(source, n)
	local self = {}
	self.source = source
	self.n = n or 1
	
	self.get = coroutine.wrap(function()
		for i = 1, self.n do
			if not self.source() then return end
		end
		for x in self.source do
			coroutine.yield(x)
		end
	end)
	
	return setmetatable(self, stream.drop)
end

setmetatable(stream.drop, stream)

--- Alias for @{drop}.
-- @function skip
-- @see drop
stream.skip = stream.drop



--- Take the last `n` values.
-- This is an alias for `reverse():take(n):reverse()`,
-- and as such also doesn't work for infinite streams.
-- @function tail
-- @tparam[opt=1] number n
-- @treturn Stream
-- @see reverse, take
function stream.tail(source, n)
	return setmetatable(source:reverse():take(n):reverse(), {
		__index = stream.reverse,
		__tostring = function(self)
			return string.format("%s -> Tail %d", source, n)
		end,
		__call = stream.get,
	})
end



stream.takeWhile = {}

stream.takeWhile.__index = stream.takeWhile
stream.takeWhile.__tostring = function(self)
	return string.format("%s -> TakeWhile", self.source)
end
stream.takeWhile.__call = stream.get

--- Pass the input values along until the condition isn't met.
-- Uses the function `fn` in the same way as @{filter}.
-- @function takeWhile
-- @tparam function fn
-- @treturn Stream
-- @see op
-- @usage stream.table({1,2,3,4,3,2,1})
-- 	:takeWhile(stream.op.lt(4))
-- 	:table() --> {1,2,3}
function stream.takeWhile.new(source, fn)
	local self = {}
	self.source = source
	self.fn = fn
	
	self.get = coroutine.wrap(function()
		for x in self.source do
			if not self.fn(x) then break end
			coroutine.yield(x)
		end
	end)
	
	return setmetatable(self, stream.takeWhile)
end

setmetatable(stream.takeWhile, stream)



stream.group = {}

stream.group.__index = stream.group
stream.group.__tostring = function(self)
	return string.format("%s -> Group", self.source)
end
stream.group.__call = stream.get

--- Group values into streams.
-- Uses the function `fn` in the same way as @{filter},
-- but passes the previous value as a second parameter,
-- and the size of the current group as a third parameter.
-- Returns a `Stream` of `Streams`.
-- @function group
-- @tparam function fn
-- @tparam boolean keepEmpty
-- @treturn Stream
-- @usage stream.table({1,2,1,3,2,2})
-- 	:group(function(x, prev) return not prev or x >= prev end)
-- 	:map(function(s) return s:string() end) -- s is a Stream here
-- 	:table() --> {"12","13","22"}
function stream.group.new(source, fn, keepEmpty)
	local self = {}
	self.source = source
	self.fn = fn or function() return true end
	self.keepEmpty = keepEmpty
	self.buffer = nil
	
	self.get = coroutine.wrap(function()
		self.buffer = {}
		for x in self.source do
			if self.fn(x, self.buffer[#self.buffer], #self.buffer) then -- add to group
				table.insert(self.buffer, x)
			elseif #self.buffer > 0 or self.keepEmpty then -- prevent empty groups
				coroutine.yield(stream.table(self.buffer))
				if self.fn(x, nil, 0) then -- start new buffer
					self.buffer = {x}
				else
					self.buffer = {}
				end
			end
		end
		if #self.buffer > 0 then -- source is empty, buffer still has items
			coroutine.yield(stream.table(self.buffer))
		end
	end)
	
	return setmetatable(self, stream.group)
end

setmetatable(stream.group, stream)



stream.concat = {}

stream.concat.__index = stream.concat
stream.concat.__tostring = function(self)
	local names = {}
	for i = 1, #self.sources do names[i] = tostring(self.sources[i]) end
	return string.format("%s (%s) -> Concat", stream, table.concat(names, ", "))
end
stream.concat.__call = stream.get

--- Concatenate another stream to the end of the current one.
-- @function concat
-- @tparam Stream stream
-- @tparam[opt] Stream ...
-- @treturn Stream
function stream.concat.new(...)
	local self = {}
	self.sources = {...}
	
	self.get = coroutine.wrap(function()
		local i = 1
		while self.sources[i] do
			for x in self.sources[i] do
				coroutine.yield(x)
			end
			i = i+1
		end
	end)
	
	return setmetatable(self, stream.concat)
end

setmetatable(stream.concat, stream)



stream.zip = {}

stream.zip.__index = stream.zip
stream.zip.__tostring = function(self)
	local names = {}
	for i = 1, #self.sources do names[i] = tostring(self.sources[i]) end
	return string.format("%s (%s) -> Zip", stream, table.concat(names, ", "))
end
stream.zip.__call = stream.get

--- Zip multiple streams together.
-- Zipping `n` streams results in a stream of `n`-length streams.
-- @function zip
-- @tparam Stream stream
-- @tparam[opt] Stream ...
-- @treturn Stream
-- @usage
-- a = stream.string("abc")
-- b = stream.string("def")
-- stream.zip(a, b):map(stream.string):table()
-- --> {"ad", "be", "cf"}
function stream.zip.new(...)
	local self = {}
	self.sources = {...}
	
	self.get = function()
		local buffer = {}
		for i = 1, #self.sources do
			local x = self.sources[i]()
			if x == nil then return end
			buffer[i] = x
		end
		return stream.table(buffer)
	end
	
	return setmetatable(self, stream.zip)
end

setmetatable(stream.zip, stream)



--- Pass all values until the value `at` is reached.
-- Alias for `takeWhile(function(x) return x ~= at end)`
-- @function stopAt
-- @param at
-- @treturn Stream
-- @see takeWhile
-- @usage stream.string("hello world"):stopAt("r"):string() --> "hello wo"
function stream.stopAt(source, at)
	return setmetatable(source:takeWhile(stream.op.neq(at)), {
		__index = stream.takeWhile,
		__tostring = function(self)
			return string.format("%s -> StopAt %q", source, at)
		end,
		__call = stream.get,
	})
end



--- Group the stream at the value `at`.
-- Alias for `group(function(x) return x ~= at end)`
-- @function splitAt
-- @param at
-- @tparam boolean keepEmpty
-- @treturn Stream
-- @see group
-- @usage
-- stream.string("hello world")
-- 	:splitAt(" ")
-- 	:map(stream.string)
-- 	:table() --> {"hello", "world"}
function stream.splitAt(source, at, keepEmpty)
	return setmetatable(source:group(stream.op.neq(at), keepEmpty), {
		__index = stream.group,
		__tostring = function(self)
			return string.format("%s -> SplitAt %q", source, at)
		end,
		__call = stream.get,
	})
end

--- Alias for @{splitAt}.
-- @function groupBy
-- @see splitAt
stream.groupBy = stream.splitAt



--- Group the stream in groups with length `size`.
-- Alias for `group(function(_, _, s) return s < size end)`
-- @function groupBySize
-- @tparam number size
-- @treturn Stream
-- @see group
-- @usage
-- stream.string("hello world")
-- 	:groupBySize(4)
-- 	:map(stream.string)
-- 	:table() --> {"hell", "o wo", "rld"}
function stream.groupBySize(source, size)
	return setmetatable(source:group(function(_, _, s) return s < size end), {
		__index = stream.group,
		__tostring = function(self)
			return string.format("%s -> GroupBySize %q", source, size)
		end,
		__call = stream.get,
	})
end



--- Map numbers from (`min1`,`max1`) to (`min2`,`max2`).
-- Alias for
-- 	map(function(x)
-- 		return min2 + (x-min1) / (max1-min1) * (max2-min2)
-- 	end)
-- @function mapRange
-- @tparam number min1
-- @tparam number max1
-- @tparam number min2
-- @tparam number max2
-- @treturn Stream
-- @see map
-- @usage stream.generate(math.random)
-- 	:mapRange(0,1,0,10):map(math.floor)
-- 	:take(2):table() --> {8,3}
function stream.mapRange(source, min1, max1, min2, max2)
	return setmetatable(source:map(function(x) return min2 + (x-min1)/(max1-min1)*(max2-min2) end), {
		__index = stream.map,
		__tostring = function()
			return string.format("%s -> MapRange (%d,%d) to (%d,%d)", source, min1, max1, min2, max2)
		end,
		__call = stream.get,
	})
end



--- Map table or string indices to values.
-- @function mapIndex
-- @tparam table|string t
-- @treturn Stream
-- @see map
-- @usage stream.generate(math.random)
-- 	:mapRange(0,1,1,16):mapIndex("0123456789abcdef")
-- 	:take(5):string() --> "d16c2"
function stream.mapIndex(source, t)
	local f
	if type(t) == "string" then
		f = function(x) return string.sub(t, math.floor(x), math.floor(x)) end
	else
		f = function(x) return t[math.floor(x)] end
	end
	return setmetatable(source:map(f), {
		__index = stream.map,
		__tostring = function()
			return string.format("%s -> MapIndex", source)
		end,
		__call = stream.get,
	})
end


--- Filter out repeated elements.
-- @function distinct
-- @treturn Stream
-- @see filter
-- @usage stream("hello world"):distinct():string() --> "helo wrd"
function stream.distinct(source)
	local t = {}
	return setmetatable(source:filter(function(x)
		local has = (t[x] == true)
		t[x] = true
		return not has
	end), {
		__index = stream.filter,
		__tostring = function()
			return string.format("%s -> Distinct", source)
		end,
		__call = stream.get,
	})
end



-- SINKS

--- Reduce the stream values into a single value.
-- @function reduce
-- @tparam function fn
-- @param[opt] acc
-- @return acc
-- @usage stream.table({1,2,3,4,5}):reduce(op.add) --> 15
function stream.reduce(source, fn, acc)
	acc = acc or source()
	for x in source do
		acc = fn(acc, x)
	end
	return acc
end

--- Collect the stream values into a container.
-- @function collect
-- @param container
-- @tparam function accumulator
-- @return container
-- @usage stream.string("hey"):collect({}, table.insert) --> {'h','e','y'}
function stream.collect(source, container, accumulator)
	for x in source do
		accumulator(container, x)
	end
	return container
end

--- Get the stream length.
-- Alias for `reduce(function(a) return a+1 end, 0)`
-- (which can be written as `reduce(stream.op.add(1), 0)`)
-- @function length
-- @treturn number
-- @see reduce
-- @usage stream.string("hello world"):stopAt("r"):length() --> 8
function stream.length(source)
	return source:reduce(function(a) return a+1 end, 0)
end

--- Get the n-th item in the stream.
-- Alias for `drop(n-1)()`
-- @function idx
-- @tparam number n
-- @return x
-- @see drop
-- @usage stream.string("hello world"):idx(2) --> "e"
function stream.idx(source, n)
	return source:drop(n-1)()
end

--- Get the first item in the stream.
-- Alias for just calling the stream
-- @function fst
-- @return x
-- @usage stream.string("hello world"):groupBy(" "):map(stream.fst):string() --> "hw"
function stream.fst(source)
	return source()
end

--- Get the second item in the stream.
-- Alias for `drop(1)()`
-- @function snd
-- @return x
-- @see drop
-- @usage stream.string("hello world"):snd() --> "e"
function stream.snd(source)
	return source:drop(1)()
end

--- Perform `fn` on all values, but ignore the stream result.
-- @function forAll
-- @tparam function fn
-- @usage stream.string("abc"):forAll(print)
-- --> a
-- --> b
-- --> c
function stream.forAll(source, fn)
	for x in source do
		fn(x)
	end
end

--- Returns whether **any** of the values fit the constraint `fn`.
-- @function any
-- @tparam function fn
-- @treturn boolean
-- @see all
-- @see op
-- @usage stream.string("hello world"):any(stream.op.eq(" ")) --> true
function stream.any(source, fn)
	fn = fn or function() return true end
	for x in source do
		if fn(x) then return true end
	end
	return false
end

--- Returns whether **all** of the values fit the constraint `fn`.
-- @function all
-- @tparam function fn
-- @treturn boolean
-- @see any
-- @see op
-- @usage stream.generate(math.random):take(5):all(stream.op.gt(0.2))
-- --> true (or false, I don't know)
function stream.all(source, fn)
	fn = fn or function() return true end
	for x in source do
		if not fn(x) then return false end
	end
	return true
end



-- UTILITY FUNCTIONS

--- Utility functions.
-- @section stream.util

stream.util = {}

--- Wrapper for `string.match`.
-- @usage
-- filter(stream.util.match("%l"))
-- -- is equivalent to (but half as long as)
-- filter(function(x) return string.match(x, "%l") end)
function stream.util.match(match)
	return function(x) return string.match(x, match) end
end

--- Wrap a regular 2-parameter function into a curried variant.
-- @usage
-- filter(stream.util.curry(op.index){'a','b','c'})
-- -- is equivalent to
-- filter(function(x) return ({'a','b','c'})[x] end)
function stream.util.curry(f)
	return function(a) return function(b) return f(a,b) end end
end

--- Wrap a regular 2-parameter function into a flipped curried variant.
-- Note how the function's arguments are flipped in the example;
-- the second argument is given first.
-- @usage
-- filter(stream.util.curry(op.lt)(3))
-- -- is equivalent to
-- filter(function(x) return x < 3 end)
function stream.util.curry_(f)
	return function(b) return function(a) return f(a,b) end end
end



-- CURRIED OPERATORS

--- Operators
-- @section op

--- Curried `operator` library proxy.
-- The curried comparison functions (`lt`, `gt`, `leq`/`le`, `geq`/`ge` and their symbols)
-- have a logical order (use @{curry_}), so the first function specifies
-- the second argument
-- @table stream.op
-- @usage
-- stream.op.index({'a','b','c'})(2) --> 'b'  -- normal order, t[2]
-- stream.op.lt(3)(1)                --> true -- reversed order, 1 < 3
stream.op = setmetatable({}, {__index = function(t,k)
	return op.comparison[k] and stream.util.curry_(op[k]) or stream.util.curry(op[k])
end})



-- RETURN

return setmetatable(stream, {
	__name = "Stream",
	__tostring = function() return "Stream:" end,
	__call = function(_, ...) return stream.new(...) end,
})
