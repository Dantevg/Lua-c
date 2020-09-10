scale = 4
frame = 0

function randomfill()
	for x = 1, math.floor(width/scale) do
		for y = 1, math.floor(height/scale) do
			colour(math.random(255), math.random(255), math.random(255), 255)
			pixel(x-1, y-1)
		end
	end
end

-- Adapted from https://github.com/klassmann/sdl2-lua53-example/blob/master/src/script.lua
function spiral(t)
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

function draw(t)
	frame = frame+1
	if frame % 120 == 0 then -- Print average FPS every 120 frames
		print( math.floor(frame / t * 1000) )
	end
	
	spiral(t)
end