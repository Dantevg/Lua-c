local console = {}

console.start = "\x1b["
local s = console.start

-- Cursor
console.cursor = {}
console.cursor._up            = s.."@1A"
console.cursor._down          = s.."@1B"
console.cursor._right         = s.."@1C"
console.cursor._left          = s.."@1D"
console.cursor._nextline      = s.."@1E"
console.cursor._prevline      = s.."@1F"
console.cursor._hor           = s.."@1G"
console.cursor._pos           = s.."@2;@1H"
console.cursor.clearend       = s.."0J"
console.cursor.clearbegin     = s.."1J"
console.cursor.clear          = s.."2J"
console.cursor.cleardel       = s.."2J"
console.cursor.clearlineend   = s.."0K"
console.cursor.clearlinebegin = s.."1K"
console.cursor.clearline      = s.."2K"
console.cursor._scrollup      = s.."@1S"
console.cursor._scrolldown    = s.."@1T"

console.cursor.__index = function(t, k)
	if rawget(t, "_"..k) then
		return function(...)
			local arg = {...}
			return t["_"..k]:gsub("@(%d)", function(x) return arg[tonumber(x)] or "" end)
		end
	end
end

setmetatable(console.cursor, console.cursor)

-- Effects
console.reset     = s.."0m"
console.bright    = s.."1m"
console.dim       = s.."2m"
console.underline = s.."4m"
console.blink     = s.."5m"
console.invert    = s.."7m"
console.strike    = s.."9m"

-- Foreground / text colour
console.fg = {}
console.fg.black   = s.."30m"
console.fg.red     = s.."31m"
console.fg.green   = s.."32m"
console.fg.yellow  = s.."33m"
console.fg.blue    = s.."34m"
console.fg.magenta = s.."35m"
console.fg.cyan    = s.."36m"
console.fg.white   = s.."37m"
console.fg.grey    = s.."90m"

console.fg.colour = function(r, g, b)
	return s..string.format("38;2;%d;%d;%dm", r, g, b)
end

-- Background colour
console.bg = {}
console.bg.black   = s.."40m"
console.bg.red     = s.."41m"
console.bg.green   = s.."42m"
console.bg.yellow  = s.."43m"
console.bg.blue    = s.."44m"
console.bg.magenta = s.."45m"
console.bg.cyan    = s.."46m"
console.bg.white   = s.."47m"
console.bg.grey    = s.."100m"

console.bg.colour = function(r, g, b)
	return s..string.format("48;2;%d;%d;%dm", r, g, b)
end

-- Aliases
console.bold = console.bright
console.fg.gray = console.fg.grey
console.bg.gray = console.bg.grey
console.fg.color = console.fg.colour
console.bg.color = console.bg.colour

return console