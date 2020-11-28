local socket = require "socket"
local thread = require "thread"

local port = select(1, ...) or 64242
local toport = select(2, ...) or 64243

local udp = socket.udp()
udp:setsockname("*", port)
udp:settimeout(1)
print(udp:getsockname())

local function socketIO()
	while true do
		local data, ip, fromport = udp:receivefrom()
		if data then
			print("\r"..ip..":"..fromport..": "..data)
			io.write("> ")
			io.flush()
		end
	end
end

local ioThread = thread.new(socketIO)

while true do
	io.write("> ")
	local input = io.read("l")
	if not input then -- EOF
		print()
		break
	end
	udp:sendto(input, "127.0.0.1", toport)
end