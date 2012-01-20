#!/usr/bin/lua

require'qtcore'
require'qtgui'

app = QApplication(1 + select('#', ...), {arg[0], ...})

window = QWidget()
window:resize(200, 120)

quit = QPushButton("Quit", window)
quit:setFont(QFont('Times', 18, 75))
quit:setGeometry(10, 40, 180, 40)

QObject.connect(quit, '2clicked()', app, '1quit()')

window:show()

app.exec()


