local lfs = require "lfs"
local event = require "event"

local filewatch = {}

filewatch.delay = 200

filewatch.watchers = {n = 0}

function filewatch.timer()
	for i = 1, filewatch.watchers.n do
		local watcher = filewatch.watchers[i]
		local newtime = lfs.attributes(watcher.path, "modification")
		if newtime and newtime ~= watcher.timestamp then
			watcher.timestamp = newtime
			event.push("filewatch", i, watcher.path, watcher.timestamp)
		end
	end
end

function filewatch.start()
	filewatch.timer = event.addTimer(filewatch.delay, filewatch.timer, true)
	filewatch.started = true
end

function filewatch.stop()
	event.removeTimer(filewatch.timer)
	filewatch.started = false
end

function filewatch.watch(path)
	if not filewatch.started then filewatch.start() end
	filewatch.watchers.n = filewatch.watchers.n+1
	filewatch.watchers[filewatch.watchers.n] = {
		path = path,
		timestamp = 0,
	}
	return filewatch.watchers.n
end

function filewatch.unwatch(id)
	filewatch.watchers[id] = nil
end

return filewatch