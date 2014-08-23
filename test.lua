local monitor = require "monitor"

local function test(n)
	if n == 0 then
		return
	end
	print ("hello word" , n)
	return test(n-1)
end

print(monitor.depth(test, 10))


print(monitor.report(test, 3))