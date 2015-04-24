
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

print("\n Testing + ")
local b = func.CreateFix(1, 100)
for i=1, 100 do
	local c = func.CreateFix(i, 100)
	b = b + c
end
print(b)
print()

print("\n Testing - ")
for i=1, 100 do
	local c = func.CreateFix(i, 100)
	b = b - c 
end
print(b)
print()

local F = func.CreateFix
local F0 = func.CreateFix0
print("\n Test * ")
do
	local c = F(1, 2) * F(2, 3)
	print(c)
end

print("\n Test / ")
do
	print(F0(1.2) / F0(0.6))
end

print("Test is done")
