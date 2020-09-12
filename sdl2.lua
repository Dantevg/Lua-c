scale = 4
frame = 0
t = 0

function randompixels()
	for i = 1, 10 do
		colour(math.random(255), math.random(255), math.random(255), 255)
		pixel(math.random(width/scale)-1, math.random(height/scale)-1)
	end
end

function randomfill()
	for x = 1, math.floor(width/scale) do
		for y = 1, math.floor(height/scale) do
			colour(math.random(255), math.random(255), math.random(255), 255)
			pixel(x-1, y-1)
		end
	end
end

-- Adapted from https://github.com/klassmann/sdl2-lua53-example/blob/master/src/script.lua
function spiral()
	local n = 100
	local speed = 5000
	for i = 1, n do
		local x = math.floor(i * math.sin(i*t/speed))
		local y = math.floor(i * math.cos(i*t/speed))
		local l = 255 - i*255//n
		colour(l, l, l, 255)
		pixel(width/scale//2 + x, height/scale//2 + y)
	end
end

function draw(dt)
	t = t+dt
	frame = frame+1
	if frame % 50 == 0 then -- Print average FPS every 50 frames
		print( math.floor(1 / dt * 1000) )
	end
	
	spiral()
end