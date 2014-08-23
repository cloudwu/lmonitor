local monitor = require "monitor"

local function foo()
	return
end

local function test(n)
	if n == 0 then
		return
	end
	for i=1,n do
	end
	foo()
	for i=1,n do
	end
	return test(n-1)
end

print(monitor.depth(test, 10))
print(monitor.depth(8, test, 10))
print(monitor.report(test, 10))
print(monitor.detailreport(test, 10))
