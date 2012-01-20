#!/usr/bin/lua

require'qtcore'
require'qtgui'

app = QApplication(1 + select('#', ...), {arg[0], ...})

-- the conversion from Lua string to QString is automatic
hello = QPushButton.new("Hello World!")
-- but not the other way round
print(hello:text():toUtf8())

hello:resize(100, 30)
hello:show()

app.exec()


