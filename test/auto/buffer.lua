local Buffer = require "Buffer"
local Value = require "Value"

local hello = "Hello, World!"
local b = Buffer.of(hello)
local v = b:view(4, 5)
assert(tostring(b) == hello)
assert(#b == #hello)
assert(b[0] == Value.of(hello))
assert(v[0] == Value.of('o'))
assert(v[5] == nil)
b[4] = 'a'
assert(v[0] == Value.of('a'))

local fortytwo = Value()
fortytwo:set(42)
assert(fortytwo:get() == 42)
assert(fortytwo + 1 == Value.of(43))
assert(fortytwo == Value.of('*'))
assert(tostring(fortytwo) == '*')
