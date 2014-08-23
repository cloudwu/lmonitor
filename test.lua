local monitor = require "monitor"
local print_r = require "print_r"

local function test(n)
	if n == 0 then
		return
	end
	print ("hello word" , n)
	return test(n-1)
end

print(monitor.depth(test, 10))


print(monitor.report(print_r, _G))