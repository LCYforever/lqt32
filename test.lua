--package.cpath = package.cpath .. ';../build/lib/RelWithDebInfo/?.dll'
local QtCore = require 'qtcore'
local QtGui = require 'qtgui'
local QtWidgets = require 'qtwidgets'

--mt = getmetatable(QtWidgets.QPushButton)
--for k, v in pairs(mt) do 
--	print(k, v)
--end
--print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")

--for k, v in pairs(QtWidgets.QPushButton) do 
--	print(k, v)
--end
--print(QtWidgets.QPushButton['QAbstractButton*'])
--print(QtWidgets.QPushButton['QPushButton*'])
--print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
--print(QtWidgets.QPushButton.__type)
--print(type(QtWidgets.QPushButton))
--print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")

mt = getmetatable(QtWidgets.QPushButton)
for k, v in pairs(mt) do 
	print(k, v)
end
print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
for k, v in pairs(QtWidgets.QPushButton) do 
	print(k, v)
end
print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")

local app = QtWidgets.QApplication.new(select('#',...) + 1, {'lua', ...})

QPushButton = qt.inherit('QPushButton', QtWidgets.QPushButton)
for k,v in pairs(getmetatable(QPushButton)) do
	print(k, n)
end
print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")


-- function QPushButton.new(...)
-- 	local button = QtWidgets.QPushButton.new(...)
-- 	local o = qt.cast(button, 'QPushButton')
-- 	o.__ClassType = 'QPushButton'
-- 	o.__SuperClass = QtWidgets.QPushButton
-- 	o.__SuperClassType = {}
-- 	-- if o.__init__ then
-- 	-- 	o:__init__(...)
-- 	-- end
-- 	return o
-- end

local btn = QPushButton.new("Hello World!")

btn:connect('2pressed()', function(self)
    print("I'm about to close...")
    self:close()
end)

btn:setWindowTitle("A great example!")
btn:resize(300,100)
btn:show()

app.exec()