local amplitude = 50
local frequency = 1/100
local zoom = 1
local timeshift = 1/2
local thickness = 1

amplitude, frequency = amplitude*zoom, frequency/zoom

local t = 0
local lastFrame = 0

local function drawLines(canvas)
	canvas:colour(50)
	for x = 0, canvas:getWidth(), 50 do
		canvas:rect(x, 0, 1, canvas:getHeight())
	end
	for y = canvas:getHeight()/2 - math.floor(canvas:getHeight()/2/50)*50, canvas:getHeight(), 50 do
		canvas:rect(0, math.floor(y), canvas:getWidth(), 1)
	end
end

local function drawStats(canvas)
	canvas:colour(255)
	canvas:write("amplitude "..amplitude, 0, 0)
	canvas:write("frequency "..frequency, 0, 7)
	canvas:write("timeshift "..timeshift, 0, 14)
	canvas:write("thickness "..thickness, 0, 21)
	canvas:write("t "..t, 0, 27)
	canvas:write("fps "..math.floor(1/(t-lastFrame)), 0, 35)
end

local function colourByValue(value)
	return math.floor(value/amplitude*128) + 128, 255 - math.floor(value/amplitude*128) + 128, 255
end

local function colourByAbsValue(value)
	return math.floor(math.abs(value/amplitude*255)), 255-math.floor(math.abs(value/amplitude*255)), 255
end

return function(canvas, dt)
	lastFrame = t
	t = t + dt/1000
	canvas:colour(0)
	canvas:clear()
	
	-- drawLines(canvas)
	drawStats(canvas)
	
	canvas:colour(255)
	for x = 0, canvas:getWidth(), 1/thickness do
		local value = math.sin(2*math.pi * (x*frequency + t*timeshift)) * amplitude
		canvas:colour(colourByAbsValue(value))
		canvas:pixel(math.floor(x), math.floor(canvas:getHeight()/2 - value))
	end
end