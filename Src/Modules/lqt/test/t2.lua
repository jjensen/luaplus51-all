#!/usr/bin/lua

local arg = {n = select('#', ...), [0] = arg[0], ...}

require'qtcore'
require'qtgui'

app = QApplication(1 + arg.n, arg)

quit = QPushButton("Quit")
quit:resize(75, 30)
quit:setFont(QFont("Times", 18, 75))

-- won't work, the signals and slots are checked if they exist
-- print(quit:connect('madeup()', app, 'quit()'))

quit:show()

app.exec()


