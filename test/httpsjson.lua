local https = require "ssl.https"
local json = require "json"
local inspect = require "inspect"

local response = https.request("https://dantevg.nl/mods-plugins/AFKScoreboard/versions.json")
print(inspect(json.decode(response), {indent = "\t"}))