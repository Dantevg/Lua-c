--- The generic `thread` module wrapper.
-- Returns any specific thread module, which implements the `thread` interface
-- 
-- Default implementations: @{thread.posix}, @{thread.win}
-- @module thread

local prequire = require "prequire"
return prequire "thread.posix" or prequire "thread.win"
	or error "Could not find implementation for 'thread'"

--- Create a new thread.
-- @function new
-- @tparam function fn the function to be called in the new thread
-- @param[opt] ... any arguments which will be passed to `fn`
-- @treturn Thread a wrapper around the newly created Lua thread

--- @type Thread

--- Wait for a thread to complete.
-- @function wait
-- @tparam Thread t the thread as returned from @{new}
-- @return the values returned from the thread function

--- Immediately stop a thread.
-- @function kill
-- @tparam Thread t the thread as returned from @{new}