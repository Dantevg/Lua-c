--- The generic `thread` module wrapper.
-- Returns any specific thread module, which implements the `thread` interface
-- 
-- Default implementations: @{thread.unsafe}, @{thread.posix}
-- @module thread

return require("thread.posix")