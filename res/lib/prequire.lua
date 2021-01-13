-- Tries to require given module
-- Returns the module if found, else returns nothing
return function(module)
	local status, result = pcall(require, module)
	if status == true then return result end
end