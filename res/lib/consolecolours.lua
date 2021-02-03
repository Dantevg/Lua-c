local consolecolours = {}

consolecolours.start = "\x1b["
local s = consolecolours.start

-- Effects
consolecolours.reset      = s.."0m"
consolecolours.bright     = s.."1m"
consolecolours.dim        = s.."2m"
consolecolours.underline  = s.."4m"
consolecolours.blink      = s.."5m"
consolecolours.invert     = s.."7m"
consolecolours.strike     = s.."9m"

-- Foreground / text colour
consolecolours.fg = {}
consolecolours.fg.black   = s.."30m"
consolecolours.fg.red     = s.."31m"
consolecolours.fg.green   = s.."32m"
consolecolours.fg.yellow  = s.."33m"
consolecolours.fg.blue    = s.."34m"
consolecolours.fg.magenta = s.."35m"
consolecolours.fg.cyan    = s.."36m"
consolecolours.fg.white   = s.."37m"
consolecolours.fg.grey    = s.."90m"

-- Background colour
consolecolours.bg = {}
consolecolours.bg.black   = s.."40m"
consolecolours.bg.red     = s.."41m"
consolecolours.bg.green   = s.."42m"
consolecolours.bg.yellow  = s.."43m"
consolecolours.bg.blue    = s.."44m"
consolecolours.bg.magenta = s.."45m"
consolecolours.bg.cyan    = s.."46m"
consolecolours.bg.white   = s.."47m"
consolecolours.bg.grey    = s.."100m"

-- Aliases
consolecolours.bold = consolecolours.bright

return consolecolours