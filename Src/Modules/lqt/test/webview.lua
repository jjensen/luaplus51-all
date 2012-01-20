#!/usr/bin/lua

require'qtcore'
require'qtgui'
require'qtwebkit'

local app = QApplication(1 + select('#', ...), {arg[0], ...})

local address = tostring(arg[1])

if address == 'nil' then
	address = 'www.lua.org'
end

print('Loading site  '..address..' ...')

local webView = QWebView()
webView:connect('2loadFinished(bool)', function()
	print('Loaded', webView:url():toEncoded())
end)
webView:setUrl(QUrl("http://" .. address))
webView:show()

app.exec()



