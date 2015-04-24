
require('fp')

local a = func.CreateFix(0,1)
xpcall(function()
	local b = func.CreateFix(10,12)
	--print("Site 100")
	local c = b / a
end, function(err)
	print("Error occured as expected")
	print(err)
end)

local b = func.CreateFix(1, 100)
for i=1, 100 do
	local c = func.CreateFix(i, 100)
	b = b + c
end

print(b)
print("Test is done")
